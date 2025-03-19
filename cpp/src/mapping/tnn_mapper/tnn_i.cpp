/******************************************************************************
 * Copyright (C) 2025 Rebecca Pelke                                           *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This is work is licensed under the terms described in the LICENSE file     *
 * found in the root directory of this source tree.                           *
 ******************************************************************************/
#include "mapping/tnn_mapper/tnn_i.h"
#include "helper/config.h"

namespace nq {

MapperTnnI::MapperTnnI() :
    vd_p_(CFG.N, 0), vd_m_(CFG.N, 0), tmp_out_(CFG.M, 0.0), Mapper(true) {}

MapperTnnI::~MapperTnnI() {}

void MapperTnnI::d_write(const int32_t *mat, int32_t m_matrix,
                         int32_t n_matrix) {
    d_write_diff_tnn(mat, m_matrix, n_matrix);
}

void MapperTnnI::a_write(int32_t m_matrix, int32_t n_matrix) {
    a_write_p_m_bnn_tnn(m_matrix, n_matrix);
}

void MapperTnnI::d_mvm(int32_t *res, const int32_t *vec, const int32_t *mat,
                       int32_t m_matrix, int32_t n_matrix) {
    for (size_t n = 0; n < n_matrix; ++n) {
        if (vec[n] == +1) {
            vd_p_[n] = 1;
            vd_m_[n] = 0;
        } else if (vec[n] == -1) {
            vd_m_[n] = 1;
            vd_p_[n] = 0;
        } else if (vec[n] == 0) {
            vd_m_[n] = 0;
            vd_p_[n] = 0;
        } else {
            std::cerr << "TNN input is neither 0 nor +1 nor -1.";
            abort();
        }
    }

    for (size_t m = 0; m < m_matrix; ++m) {
        for (size_t n = 0; n < n_matrix; ++n) {
            res[m] += gd_p_[m][n] * vd_p_[n] + gd_m_[m][n] * vd_m_[n] -
                      gd_m_[m][n] * vd_p_[n] - gd_p_[m][n] * vd_m_[n];
        }
    }
}

void MapperTnnI::a_mvm(int32_t *res, const int32_t *vec, const int32_t *mat,
                       int32_t m_matrix, int32_t n_matrix) {
    std::fill(tmp_out_.begin(), tmp_out_.end(), 0.0);

    for (size_t n = 0; n < n_matrix; ++n) {
        if (vec[n] == +1) {
            vd_p_[n] = 1;
            vd_m_[n] = 0;
        } else if (vec[n] == -1) {
            vd_m_[n] = 1;
            vd_p_[n] = 0;
        } else if (vec[n] == 0) {
            vd_m_[n] = 0;
            vd_p_[n] = 0;
        } else {
            std::cerr << "TNN input is neither 0 nor +1 nor -1.";
            abort();
        }
    }

    for (size_t m = 0; m < m_matrix; ++m) {
        for (size_t n = 0; n < n_matrix; ++n) {
            tmp_out_[m] += ia_p_[m][n] * vd_p_[n] + ia_m_[m][n] * vd_m_[n] -
                           ia_m_[m][n] * vd_p_[n] - ia_p_[m][n] * vd_m_[n];
        }
    }

    for (size_t m = 0; m < m_matrix; ++m) {
        res[m] += round(adc_->analog_digital_conversion(tmp_out_[m]) / i_mm_);
    }
}

} // namespace nq