/******************************************************************************
 * Copyright (C) 2025 Rebecca Pelke                                           *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This work is licensed under the terms described in the LICENSE file        *
 * found in the root directory of this source tree.                           *
 ******************************************************************************/
#ifndef MAPPERINTI_H
#define MAPPERINTI_H

#include <cstdint>

#include "mapping/mapper.h"

namespace nq {

// Mapping: I_DIFF_W_DIFF_1XB and I_DIFF_W_DIFF_2XB
class MapperIntI : public Mapper {
  public:
    // How each mapping places one weight on the crossbar and what it costs.
    static constexpr MappingProperties PROPERTIES_1XB = {
        /* mode */ MappingMode::I_DIFF_W_DIFF_1XB,
        /* name */ "I_DIFF_W_DIFF_1XB",
        /* type */ MappingType::INT,
        /* col_mult */ 2,
        /* row_mult */ 1,
        /* split_in_cols */ true,
        /* out_enc */ OutputEnc::DIFF_COL,
        /* xbars */ 1,
        /* cycles */ 2,
        /* bit_serial */ true};

    static constexpr MappingProperties PROPERTIES_2XB = {
        /* mode */ MappingMode::I_DIFF_W_DIFF_2XB,
        /* name */ "I_DIFF_W_DIFF_2XB",
        /* type */ MappingType::INT,
        /* col_mult */ 2,
        /* row_mult */ 1,
        /* split_in_cols */ true,
        /* out_enc */ OutputEnc::DIFF_COL,
        /* xbars */ 2,
        /* cycles */ 1,
        /* bit_serial */ true};

    // The two modes differ only in how many crossbars they occupy, so one
    // mapper serves both.
    static constexpr const MappingProperties &properties_of(MappingMode mode) {
        return (mode == MappingMode::I_DIFF_W_DIFF_2XB) ? PROPERTIES_2XB
                                                        : PROPERTIES_1XB;
    }

    MapperIntI();
    MapperIntI(const MapperIntI &) = delete;
    virtual ~MapperIntI();

    void d_write(const int32_t *mat, int32_t m_matrix,
                 int32_t n_matrix) override;
    void a_write(int32_t m_matrix, int32_t n_matrix) override;
    void d_mvm(int32_t *res, const int32_t *vec, const int32_t *mat,
               int32_t m_matrix, int32_t n_matrix) override;
    void a_mvm(int32_t *res, const int32_t *vec, const int32_t *mat,
               int32_t m_matrix, int32_t n_matrix,
               const char *l_name = "Unknown") override;

  private:
    // Temporary data for MVM
    std::vector<int32_t> vd_p_;
    std::vector<int32_t> vd_m_;
    std::vector<int32_t> tmp_out_int_;
    std::vector<float> tmp_out_fp_;

    std::vector<int32_t> vd_slice_;
};

} // namespace nq

#endif
