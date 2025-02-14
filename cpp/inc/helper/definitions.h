#ifndef DEFINITIONS_H
#define DEFINITIONS_H

namespace nq {

enum class INT8MappingMode { // Mapping modes for inputs/weights as signed integers
    I_DIFF_W_DIFF_1XB,  // Inputs and weights in differential mode, use 1 XBar / 2 Cycles
    I_DIFF_W_DIFF_2XB,  // Inputs and weights in differential mode, use 2 XBars / 1 Cycle
    I_OFFS_W_DIFF,      // Inputs shifted to positive range
    I_TC_W_DIFF,        // Inputs interpreted as two's complement
    I_UINT_W_DIFF,      // Inputs are already positive-only. Weights in differential mode
    I_UINT_W_OFFS       // Inputs are already positive-only. Weights use an offset
};

static std::string m_mode_to_string(INT8MappingMode mode) {
    switch (mode) {
        case INT8MappingMode::I_DIFF_W_DIFF_1XB:
            return "I_DIFF_W_DIFF_1XB";
        case INT8MappingMode::I_DIFF_W_DIFF_2XB:
            return "I_DIFF_W_DIFF_2XB";
        case INT8MappingMode::I_OFFS_W_DIFF:
            return "I_OFFS_W_DIFF";
        case INT8MappingMode::I_TC_W_DIFF:
            return "I_TC_W_DIFF";
        case INT8MappingMode::I_UINT_W_DIFF:
            return "I_UINT_W_DIFF";
        case INT8MappingMode::I_UINT_W_OFFS:
            return "I_UINT_W_OFFS";
        default:
            return "Unknown mode";
    }
}

} // end namespace

#endif
