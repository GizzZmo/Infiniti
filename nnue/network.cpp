#include "network.h"
#include <algorithm>
#include <cstdint>

namespace NNUE {

static int16_t clamp_relu(int16_t x) {
    return std::max(int16_t(0), std::min(int16_t(127), x));
}

int network_evaluate(const NetworkWeights& weights, const AccumulatorEntry& acc, Color stm) {
    if (!weights.loaded) return 0;

    std::array<int16_t, L1_SIZE * 2> input;
    int opp = (stm == WHITE) ? BLACK : WHITE;
    const auto& stm_acc = acc.acc[stm];
    const auto& opp_acc = acc.acc[opp];
    for (int i = 0; i < L1_SIZE; i++) {
        input[i]           = clamp_relu(stm_acc[i]);
        input[L1_SIZE + i] = clamp_relu(opp_acc[i]);
    }

    std::array<int32_t, 32> l2;
    for (int o = 0; o < 32; o++) {
        int32_t sum = weights.l2_biases[o];
        for (int i = 0; i < L1_SIZE * 2; i++)
            sum += int32_t(input[i]) * int32_t(weights.l2_weights[o * L1_SIZE * 2 + i]);
        l2[o] = std::max(0, sum >> 6);
    }

    std::array<int32_t, 32> l3;
    for (int o = 0; o < 32; o++) {
        int32_t sum = weights.l3_biases[o];
        for (int i = 0; i < 32; i++)
            sum += l2[i] * int32_t(weights.l3_weights[o * 32 + i]);
        l3[o] = std::max(0, sum >> 6);
    }

    int32_t out = weights.out_bias;
    for (int i = 0; i < 32; i++)
        out += l3[i] * int32_t(weights.out_weights[i]);

    return out / 600;
}

} // namespace NNUE
