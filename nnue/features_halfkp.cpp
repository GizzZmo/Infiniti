#include "features_halfkp.h"

namespace NNUE {

int halfkp_feature_index(Square ksq, Square psq, Piece p, Color perspective) {
    if (perspective == BLACK) {
        ksq = Square(ksq ^ 56);
        psq = Square(psq ^ 56);
    }
    PieceType pt = type_of(p);
    Color pc = color_of(p);
    int rel_color = (pc == perspective) ? 0 : 1;
    int piece_idx = (int(pt) - 1) * 2 + rel_color;
    return int(ksq) * 640 + piece_idx * 64 + int(psq);
}

} // namespace NNUE
