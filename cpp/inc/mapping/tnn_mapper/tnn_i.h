/******************************************************************************
 * Copyright (C) 2025 Rebecca Pelke                                           *
 * All Rights Reserved                                                        *
 *                                                                            *
 * This work is licensed under the terms described in the LICENSE file        *
 * found in the root directory of this source tree.                           *
 ******************************************************************************/
#ifndef MAPPERTNNI_H
#define MAPPERTNNI_H

#include <cstdint>

#include "mapping/mapper.h"

namespace nq {

// Mapping TNN I: i_NN = v_D+ - v_D-, w_NN = g_D+ - g_D-
class MapperTnnI : public Mapper {
  public:
    // How this mapping places one weight on the crossbar and what it costs.
    static constexpr MappingProperties PROPERTIES = {
        /* mode */ MappingMode::TNN_I,
        /* name */ "TNN_I",
        /* type */ MappingType::TNN,
        /* col_mult */ 2,
        /* row_mult */ 2,
        /* split_in_cols */ false,
        /* out_enc */ OutputEnc::DIFF_COL,
        /* xbars */ 1,
        /* cycles */ 1,
        /* bit_serial */ false};

    MapperTnnI();
    MapperTnnI(const MapperTnnI &) = delete;
    virtual ~MapperTnnI();

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
    std::vector<float> tmp_out_;
};

} // namespace nq

#endif
