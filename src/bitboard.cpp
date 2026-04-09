#include "bitboard.h"
#include "position.h"

Bitboard FileMask[8];
Bitboard RankMask[8];
Bitboard DiagMask[15];
Bitboard AntiDiagMask[15];
Bitboard PawnAttacks[2][64];
Bitboard KnightAttacks[64];
Bitboard KingAttacks[64];

static Bitboard ray_attacks(Square sq, int dx, int dy, Bitboard occ) {
    Bitboard result = 0;
    int f = file_of(sq);
    int r = rank_of(sq);
    int ff = f + dx, rr = r + dy;
    while (ff >= 0 && ff <= 7 && rr >= 0 && rr <= 7) {
        Square s = make_square(File(ff), Rank(rr));
        result |= (1ULL << s);
        if (occ & (1ULL << s)) break;
        ff += dx;
        rr += dy;
    }
    return result;
}

Bitboard rook_attacks(Square sq, Bitboard occ) {
    return ray_attacks(sq, 0, 1, occ) | ray_attacks(sq, 0, -1, occ) |
           ray_attacks(sq, 1, 0, occ) | ray_attacks(sq, -1, 0, occ);
}

Bitboard bishop_attacks(Square sq, Bitboard occ) {
    return ray_attacks(sq, 1, 1, occ) | ray_attacks(sq, 1, -1, occ) |
           ray_attacks(sq, -1, 1, occ) | ray_attacks(sq, -1, -1, occ);
}

void init_bitboards() {
    Zobrist::init();

    for (int f = 0; f < 8; f++) {
        FileMask[f] = 0;
        for (int r = 0; r < 8; r++) FileMask[f] |= (1ULL << make_square(File(f), Rank(r)));
    }
    for (int r = 0; r < 8; r++) {
        RankMask[r] = 0;
        for (int f = 0; f < 8; f++) RankMask[r] |= (1ULL << make_square(File(f), Rank(r)));
    }
    for (int i = 0; i < 15; i++) DiagMask[i] = AntiDiagMask[i] = 0;
    for (int f = 0; f < 8; f++) {
        for (int r = 0; r < 8; r++) {
            Square sq = make_square(File(f), Rank(r));
            DiagMask[r - f + 7] |= (1ULL << sq);
            AntiDiagMask[r + f] |= (1ULL << sq);
        }
    }
    for (int sq = 0; sq < 64; sq++) {
        PawnAttacks[WHITE][sq] = PawnAttacks[BLACK][sq] = 0;
        int f = sq & 7, r = sq >> 3;
        if (r < 7) {
            if (f > 0) PawnAttacks[WHITE][sq] |= (1ULL << (sq + 7));
            if (f < 7) PawnAttacks[WHITE][sq] |= (1ULL << (sq + 9));
        }
        if (r > 0) {
            if (f > 0) PawnAttacks[BLACK][sq] |= (1ULL << (sq - 9));
            if (f < 7) PawnAttacks[BLACK][sq] |= (1ULL << (sq - 7));
        }
    }
    static const int knight_dx[] = {-2,-2,-1,-1, 1, 1, 2, 2};
    static const int knight_dy[] = {-1, 1,-2, 2,-2, 2,-1, 1};
    for (int sq = 0; sq < 64; sq++) {
        KnightAttacks[sq] = 0;
        int f = sq & 7, r = sq >> 3;
        for (int i = 0; i < 8; i++) {
            int nf = f + knight_dx[i], nr = r + knight_dy[i];
            if (nf >= 0 && nf <= 7 && nr >= 0 && nr <= 7)
                KnightAttacks[sq] |= (1ULL << make_square(File(nf), Rank(nr)));
        }
    }
    static const int king_dx[] = {-1,-1,-1, 0, 0, 1, 1, 1};
    static const int king_dy[] = {-1, 0, 1,-1, 1,-1, 0, 1};
    for (int sq = 0; sq < 64; sq++) {
        KingAttacks[sq] = 0;
        int f = sq & 7, r = sq >> 3;
        for (int i = 0; i < 8; i++) {
            int nf = f + king_dx[i], nr = r + king_dy[i];
            if (nf >= 0 && nf <= 7 && nr >= 0 && nr <= 7)
                KingAttacks[sq] |= (1ULL << make_square(File(nf), Rank(nr)));
        }
    }
}
