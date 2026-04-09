#pragma once
#include "types.h"
#include <bit>
#include <cstdlib>

// File and rank masks
extern Bitboard FileMask[8];
extern Bitboard RankMask[8];
extern Bitboard DiagMask[15];
extern Bitboard AntiDiagMask[15];

// Attack tables
extern Bitboard PawnAttacks[2][64];
extern Bitboard KnightAttacks[64];
extern Bitboard KingAttacks[64];

void init_bitboards();

inline int popcount(Bitboard b) {
    return std::popcount(b);
}

inline int lsb(Bitboard b) {
    return std::countr_zero(b);
}

inline int msb(Bitboard b) {
    return 63 - std::countl_zero(b);
}

inline Square pop_lsb(Bitboard& b) {
    Square s = Square(lsb(b));
    b &= b - 1;
    return s;
}

// Classical sliding attacks
Bitboard rook_attacks(Square sq, Bitboard occ);
Bitboard bishop_attacks(Square sq, Bitboard occ);
inline Bitboard queen_attacks(Square sq, Bitboard occ) {
    return rook_attacks(sq, occ) | bishop_attacks(sq, occ);
}

inline Bitboard file_mask(File f) { return FileMask[f]; }
inline Bitboard rank_mask(Rank r) { return RankMask[r]; }
