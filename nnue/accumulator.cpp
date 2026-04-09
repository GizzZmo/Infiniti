#include "accumulator.h"

namespace NNUE {

void Accumulator::reset(const std::array<int16_t, L1_SIZE>& bias) {
    data = bias;
}

void Accumulator::add_feature(int feature_idx, const std::array<int16_t, HALFKP_FEATURE_COUNT * L1_SIZE>& weights) {
    int base = feature_idx * L1_SIZE;
    for (int i = 0; i < L1_SIZE; i++)
        data[i] += weights[base + i];
}

void Accumulator::remove_feature(int feature_idx, const std::array<int16_t, HALFKP_FEATURE_COUNT * L1_SIZE>& weights) {
    int base = feature_idx * L1_SIZE;
    for (int i = 0; i < L1_SIZE; i++)
        data[i] -= weights[base + i];
}

} // namespace NNUE
