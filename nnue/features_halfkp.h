#pragma once
#include "../src/types.h"

namespace NNUE {
constexpr int HALFKP_FEATURE_COUNT = 40960;
int halfkp_feature_index(Square ksq, Square psq, Piece p, Color perspective);
}
