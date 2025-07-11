/******************************************************************************
 * Copyright (C) 2025 Rebecca Pelke                                           *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 ******************************************************************************/
#include "xbar/crossbar.h"
#include "helper/config.h"

namespace nq {

Crossbar::Crossbar() :
    mapper_(Mapper::create_from_config()), write_xbar_counter_(0),
    mvm_counter_(0), rd_model_(nullptr), consecutive_mvm_counter_(0),
    refresh_xbar_counter_(0), refresh_cell_counter_(0) {
    if (CFG.read_disturb) {
        rd_model_ = std::make_shared<ReadDisturb>(CFG.V_read);
    }
}

void Crossbar::write(const int32_t *mat, int32_t m_matrix, int32_t n_matrix) {
    write_xbar_counter_++;
    consecutive_mvm_counter_ = 0;
    if (CFG.read_disturb) {
        std::vector<std::vector<bool>> update_p(
            CFG.M * CFG.SPLIT.size(), std::vector<bool>(CFG.N, false));
        std::vector<std::vector<bool>> update_m(
            CFG.M * CFG.SPLIT.size(), std::vector<bool>(CFG.N, false));

        // Copy gd_p and gd_m before changing them
        const std::vector<std::vector<int32_t>> prev_gd_p = mapper_->get_gd_p();
        const std::vector<std::vector<int32_t>> prev_gd_m = mapper_->get_gd_m();

        mapper_->d_write(mat, m_matrix, n_matrix);

        // Get the current gd_p and gd_m after writing
        const std::vector<std::vector<int32_t>> &curr_gd_p =
            mapper_->get_gd_p();
        const std::vector<std::vector<int32_t>> &curr_gd_m =
            mapper_->get_gd_m();

        // Compare the previous and current gd_p and gd_m to find updates
        for (size_t i = 0; i < update_p.size(); i++) {
            for (size_t j = 0; j < update_p[i].size(); j++) {
                if (prev_gd_p[i][j] == 1 && curr_gd_p[i][j] == 0) {
                    update_p[i][j] = true;
                }
                if (prev_gd_m[i][j] == 1 && curr_gd_m[i][j] == 0) {
                    update_m[i][j] = true;
                }
            }
        }
        // Update the set-reset cycles for each cell
        rd_model_->update_cycles(update_p, update_m);

        // Reset the consecutive reads when cell_based mitigation is used
        if (CFG.read_disturb_mitigation_strategy ==
            ReadDisturbMitigationStrategy::CELL_BASED) {
            rd_model_->reset_all_consecutive_reads();
        }
    } else {
        mapper_->d_write(mat, m_matrix, n_matrix);
    }
    if (!CFG.digital_only) {
        mapper_->a_write(m_matrix, n_matrix);
    }
}

void Crossbar::mvm(int32_t *res, const int32_t *vec, const int32_t *mat,
                   int32_t m_matrix, int32_t n_matrix) {
    mvm_counter_++;
    consecutive_mvm_counter_++;
    if (CFG.digital_only) {
        mapper_->d_mvm(res, vec, mat, m_matrix, n_matrix);
    } else {
        mapper_->a_mvm(res, vec, mat, m_matrix, n_matrix);

        if (CFG.read_disturb) {
            switch (CFG.read_disturb_mitigation_strategy) {
            case ReadDisturbMitigationStrategy::OFF:
                // No read disturb mitigation -> Simulate effect only
                if (consecutive_mvm_counter_ % CFG.read_disturb_update_freq ==
                    0) {
                    mapper_->rd_update_conductance(rd_model_,
                                                   consecutive_mvm_counter_);
                }
                break;

            case ReadDisturbMitigationStrategy::SOFTWARE:
                // Software refresh, simulate effect first
                if (consecutive_mvm_counter_ % CFG.read_disturb_update_freq ==
                    0) {
                    // Update the conductance values
                    mapper_->rd_update_conductance(rd_model_,
                                                   consecutive_mvm_counter_);
                    // Check if a refresh is needed
                    bool refresh_needed = mapper_->rd_check_software_refresh(
                        rd_model_, consecutive_mvm_counter_,
                        write_xbar_counter_ + refresh_xbar_counter_);

                    if (refresh_needed) {
                        refresh_xbar_counter_++;

                        // Increase the set-reset cycle for every LRS cell since
                        // all LRS cells are reprogrammed by resetting and
                        // setting again
                        std::vector<std::vector<bool>> update_p(
                            CFG.M * CFG.SPLIT.size(),
                            std::vector<bool>(CFG.N, false));
                        std::vector<std::vector<bool>> update_m(
                            CFG.M * CFG.SPLIT.size(),
                            std::vector<bool>(CFG.N, false));

                        // Get the current gd_p and gd_m
                        const std::vector<std::vector<int32_t>> &curr_gd_p =
                            mapper_->get_gd_p();
                        const std::vector<std::vector<int32_t>> &curr_gd_m =
                            mapper_->get_gd_m();

                        for (size_t i = 0; i < update_p.size(); i++) {
                            for (size_t j = 0; j < update_p[i].size(); j++) {
                                if (curr_gd_p[i][j] == 1) {
                                    update_p[i][j] = true;
                                    refresh_cell_counter_++;
                                }
                                if (mapper_->is_diff_weight_mapping()) {
                                    if (curr_gd_m[i][j] == 1) {
                                        update_m[i][j] = true;
                                        refresh_cell_counter_++;
                                    }
                                }
                            }
                        }

                        // Update the set-reset cycles for each cell
                        rd_model_->update_cycles(update_p, update_m);

                        // Reset the consecutive reads
                        consecutive_mvm_counter_ = 0;

                        // Reset conductance values
                        mapper_->a_write(CFG.M, CFG.N);
                    }
                }
                break;

            case ReadDisturbMitigationStrategy::CELL_BASED:
                // Cell-based refresh
                // Update consecutive reads first
                rd_model_->update_consecutive_reads(m_matrix, n_matrix);

                if (consecutive_mvm_counter_ % CFG.read_disturb_update_freq ==
                    0) {
                    // Update the conductance values
                    mapper_->rd_update_conductance(
                        rd_model_, rd_model_->get_consecutive_reads_p(),
                        rd_model_->get_consecutive_reads_m());

                    // Refresh cell individually if needed
                    int refresh_operations =
                        mapper_->rd_cell_based_refresh(rd_model_);
                    refresh_cell_counter_ += refresh_operations;
                }
                break;

            default:
                std::cerr << "Unknown read disturb mitigation strategy!"
                          << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
    }
}

Crossbar::~Crossbar() {
    if (CFG.verbose) {
        std::cout << "MappingMode: " << m_mode_to_string(CFG.m_mode)
                  << std::endl;
        std::cout << "write_xbar_counter_: " << write_xbar_counter_
                  << std::endl;
        std::cout << "mvm_counter_: " << mvm_counter_ << std::endl;

        uint64_t num_write = 0;
        uint64_t num_mvm_total = 0;
        uint64_t num_mvm_sequential = 0;
        uint32_t cells_per_value = CFG.SPLIT.size();

        switch (CFG.m_mode) {
        case MappingMode::I_DIFF_W_DIFF_1XB:
            num_write = write_xbar_counter_;
            num_mvm_total = mvm_counter_ * CFG.I_BIT;
            num_mvm_sequential = mvm_counter_ * 2 * CFG.I_BIT;
            cells_per_value *= 2;
            break;
        case MappingMode::I_DIFF_W_DIFF_2XB:
            num_write = write_xbar_counter_ * 2;
            num_mvm_total = mvm_counter_ * 2 * CFG.I_BIT;
            num_mvm_sequential = mvm_counter_ * CFG.I_BIT;
            cells_per_value *= 4;
            break;
        case MappingMode::I_OFFS_W_DIFF:
            num_write = write_xbar_counter_;
            num_mvm_total = mvm_counter_ * CFG.I_BIT;
            num_mvm_sequential = mvm_counter_ * CFG.I_BIT;
            cells_per_value *= 2;
            break;
        case MappingMode::I_TC_W_DIFF:
            num_write = write_xbar_counter_;
            num_mvm_total = mvm_counter_ * CFG.I_BIT;
            num_mvm_sequential = mvm_counter_ * CFG.I_BIT;
            cells_per_value *= 2;
            break;
        case MappingMode::I_UINT_W_DIFF:
            num_write = write_xbar_counter_;
            num_mvm_total = mvm_counter_ * CFG.I_BIT;
            num_mvm_sequential = mvm_counter_ * CFG.I_BIT;
            cells_per_value *= 2;
            break;
        case MappingMode::I_UINT_W_OFFS:
            num_write = write_xbar_counter_;
            num_mvm_total = mvm_counter_ * CFG.I_BIT;
            num_mvm_sequential = mvm_counter_ * CFG.I_BIT;
            cells_per_value *= 1;
            break;
        default:
            std::cerr << "Unknown mode encountered!" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        std::cout << "num_write: " << num_write << std::endl;
        std::cout << "num_mvm_total: " << num_mvm_total << std::endl;
        std::cout << "num_mvm_sequential: " << num_mvm_sequential << std::endl;
        std::cout << "cells_per_value: " << cells_per_value << std::endl;
    }
}

const std::vector<std::vector<int32_t>> &Crossbar::get_gd_p() const {
    return mapper_->get_gd_p();
}

const std::vector<std::vector<int32_t>> &Crossbar::get_gd_m() const {
    return mapper_->get_gd_m();
}

const std::vector<std::vector<float>> &Crossbar::get_ia_p() const {
    return mapper_->get_ia_p();
}

const std::vector<std::vector<float>> &Crossbar::get_ia_m() const {
    return mapper_->get_ia_m();
}

const std::vector<std::vector<uint64_t>> &Crossbar::get_cycles_p() const {
    return rd_model_->get_cycles_p();
}

const std::vector<std::vector<uint64_t>> &Crossbar::get_cycles_m() const {
    return rd_model_->get_cycles_m();
}

const std::vector<std::vector<uint64_t>> &
Crossbar::get_consecutive_reads_p() const {
    return rd_model_->get_consecutive_reads_p();
}

const std::vector<std::vector<uint64_t>> &
Crossbar::get_consecutive_reads_m() const {
    return rd_model_->get_consecutive_reads_m();
}

const uint64_t Crossbar::get_write_xbar_counter() const {
    return write_xbar_counter_;
}

const uint64_t Crossbar::get_mvm_counter() const { return mvm_counter_; }

const uint64_t Crossbar::get_read_num() const {
    return consecutive_mvm_counter_;
}

const uint64_t Crossbar::get_refresh_xbar_counter() const {
    return refresh_xbar_counter_;
}

const uint64_t Crossbar::get_refresh_cell_counter() const {
    return refresh_cell_counter_;
}

const bool Crossbar::get_rd_run_out_of_bounds() const {
    return rd_model_->get_run_out_of_bounds();
}

} // namespace nq