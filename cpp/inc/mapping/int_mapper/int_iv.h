/******************************************************************************
 * Copyright (C) 2025 Rebecca Pelke                                           *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This work is licensed under the terms described in the LICENSE file        *
 * found in the root directory of this source tree.                           *
 ******************************************************************************/
#ifndef MAPPERINTIV_H
#define MAPPERINTIV_H

#include <cstdint>

#include "mapping/mapper.h"

namespace nq {

// Mapping: I_UINT_W_DIFF
class MapperIntIV : public Mapper {
  public:
    // How this mapping places one weight on the crossbar and what it costs.
    static constexpr MappingProperties PROPERTIES = {
        /* mode */ MappingMode::I_UINT_W_DIFF,
        /* name */ "I_UINT_W_DIFF",
        /* type */ MappingType::INT,
        /* col_mult */ 2,
        /* row_mult */ 1,
        /* split_in_cols */ true,
        /* out_enc */ OutputEnc::DIFF_COL,
        /* xbars */ 1,
        /* cycles */ 1,
        /* bit_serial */ true};

    MapperIntIV();
    MapperIntIV(const MapperIntIV &) = delete;
    virtual ~MapperIntIV();

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
    std::vector<int32_t> tmp_out_int_;
    std::vector<float> tmp_out_fp_;
    std::vector<int32_t> vd_slice_;
};

} // namespace nq

#endif
