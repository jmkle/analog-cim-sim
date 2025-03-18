#include <cstdlib>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "inc/test_helper.h"

TEST(ADCTests, SymADCTest) {
    const int32_t m_matrix = 3;
    const int32_t n_matrix = 5;
    int32_t mat[m_matrix * n_matrix] = {-128, -128, -128, -128, -128,
                                        127,  127,  127,  127,  127,
                                        -12,  88,   65,   0,    -99};
    int32_t vec[n_matrix] = {-1, -1, -1, -1, -1};
    int32_t res[m_matrix] = {0, 0, 0};

    std::string cfg = get_cfg_file("analog/SYM_ADC_1.json");
    set_config(cfg.c_str());

    int32_t status = cpy_mtrx(mat, m_matrix, n_matrix);
    ASSERT_EQ(status, 0) << "Matrix write operation failed.";

    status = exe_mvm(res, vec, mat, m_matrix, n_matrix);
    ASSERT_EQ(status, 0) << "Matrix-vector multiplication failed.";
    ASSERT_THAT(res, ::testing::ElementsAre(643, -637, -40));
}

TEST(ADCTests, PosADCTest) {
    const int32_t m_matrix = 3;
    const int32_t n_matrix = 5;
    int32_t mat[m_matrix * n_matrix] = {
        127, 127, 127, 127, 127, -128, -128, -128, -128, -128, 0, 0, 0, 0, 0};
    int32_t vec[n_matrix] = {1, 1, 1, 1, 1};
    int32_t res[m_matrix] = {0, 0, 0};

    std::string cfg = get_cfg_file("analog/POS_ADC_1.json");
    set_config(cfg.c_str());

    int32_t status = cpy_mtrx(mat, m_matrix, n_matrix);
    ASSERT_EQ(status, 0) << "Matrix write operation failed.";

    status = exe_mvm(res, vec, mat, m_matrix, n_matrix);
    ASSERT_EQ(status, 0) << "Matrix-vector multiplication failed.";
    ASSERT_THAT(res, ::testing::ElementsAre(635, -637, -1));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
