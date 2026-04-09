#pragma once
#include "features_halfkp.h"
#include <array>
#include <cstdint>

namespace NNUE {
constexpr int L1_SIZE = 256;

struct AccumulatorEntry {
    std::array<int16_t, L1_SIZE> acc[2];
    bool computed[2] = {false, false};
};

class Accumulator {
public:
    void reset(const std::array<int16_t, L1_SIZE>& bias);
    void add_feature(int feature_idx, const std::array<int16_t, HALFKP_FEATURE_COUNT * L1_SIZE>& weights);
    void remove_feature(int feature_idx, const std::array<int16_t, HALFKP_FEATURE_COUNT * L1_SIZE>& weights);
    const std::array<int16_t, L1_SIZE>& get() const { return data; }
private:
    std::array<int16_t, L1_SIZE> data{};
};
}
