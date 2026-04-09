#pragma once
#include "accumulator.h"
#include <array>
#include <cstdint>

namespace NNUE {

struct NetworkWeights {
    std::array<int16_t, HALFKP_FEATURE_COUNT * L1_SIZE> l1_weights[2]{};
    std::array<int16_t, L1_SIZE> l1_biases[2]{};
    std::array<int8_t, L1_SIZE * 2 * 32> l2_weights{};
    std::array<int32_t, 32> l2_biases{};
    std::array<int8_t, 32 * 32> l3_weights{};
    std::array<int32_t, 32> l3_biases{};
    std::array<int8_t, 32> out_weights{};
    int32_t out_bias = 0;
    bool loaded = false;
};

int network_evaluate(const NetworkWeights& weights, const AccumulatorEntry& acc, Color stm);

} // namespace NNUE
