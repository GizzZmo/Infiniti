#include "movegen.h"
#include "bitboard.h"
#include <cassert>

static void generate_pawn_moves(const Position& pos, MoveList& list, bool captures_only) {
    Color us = pos.side_to_move();
    Color them = ~us;
    Bitboard pawns = pos.pieces(us, PAWN);
    Bitboard occ = pos.all_pieces();
    Bitboard their_pieces = pos.pieces(them);

    int push_dir = (us == WHITE) ? 8 : -8;
    Rank promo_rank = (us == WHITE) ? RANK_8 : RANK_1;
    Rank start_rank = (us == WHITE) ? RANK_2 : RANK_7;

    Bitboard bb = pawns;
    while (bb) {
        Square from = pop_lsb(bb);
        Rank r = rank_of(from);

        // Captures
        Bitboard attacks = PawnAttacks[us][from] & their_pieces;
        while (attacks) {
            Square to = pop_lsb(attacks);
            if (rank_of(to) == promo_rank) {
                list.add(make_special(from, to, PROMOTION, QUEEN));
                list.add(make_special(from, to, PROMOTION, ROOK));
                list.add(make_special(from, to, PROMOTION, BISHOP));
                list.add(make_special(from, to, PROMOTION, KNIGHT));
            } else {
                list.add(make_move(from, to));
            }
        }

        // En passant
        Square ep = pos.ep_square();
        if (ep != NO_SQUARE && (PawnAttacks[us][from] & (1ULL << ep))) {
            list.add(make_special(from, ep, EN_PASSANT));
        }

        if (captures_only) continue;

        // Single push
        Square to1 = Square(int(from) + push_dir);
        if (to1 >= A1 && to1 <= H8 && !(occ & (1ULL << to1))) {
            if (rank_of(to1) == promo_rank) {
                list.add(make_special(from, to1, PROMOTION, QUEEN));
                list.add(make_special(from, to1, PROMOTION, ROOK));
                list.add(make_special(from, to1, PROMOTION, BISHOP));
                list.add(make_special(from, to1, PROMOTION, KNIGHT));
            } else {
                list.add(make_move(from, to1));
                // Double push
                if (r == start_rank) {
                    Square to2 = Square(int(to1) + push_dir);
                    if (!(occ & (1ULL << to2))) {
                        list.add(make_move(from, to2));
                    }
                }
            }
        }
    }
}

static void generate_piece_moves(const Position& pos, MoveList& list, PieceType pt, bool captures_only) {
    Color us = pos.side_to_move();
    Bitboard our_pieces = pos.pieces(us);
    Bitboard their_pieces = pos.pieces(~us);
    Bitboard occ = pos.all_pieces();
    Bitboard bb = pos.pieces(us, pt);

    while (bb) {
        Square from = pop_lsb(bb);
        Bitboard attacks;
        switch (pt) {
            case KNIGHT: attacks = KnightAttacks[from]; break;
            case BISHOP: attacks = bishop_attacks(from, occ); break;
            case ROOK:   attacks = rook_attacks(from, occ); break;
            case QUEEN:  attacks = queen_attacks(from, occ); break;
            case KING:   attacks = KingAttacks[from]; break;
            default: attacks = 0; break;
        }
        attacks &= ~our_pieces;
        if (captures_only) attacks &= their_pieces;

        while (attacks) {
            Square to = pop_lsb(attacks);
            list.add(make_move(from, to));
        }
    }
}

static void generate_castling(const Position& pos, MoveList& list) {
    Color us = pos.side_to_move();
    int rights = pos.castling_rights();
    Bitboard occ = pos.all_pieces();

    if (us == WHITE) {
        if ((rights & WHITE_OO) &&
            !(occ & ((1ULL<<F1)|(1ULL<<G1))) &&
            !pos.is_attacked(E1, BLACK) &&
            !pos.is_attacked(F1, BLACK) &&
            !pos.is_attacked(G1, BLACK)) {
            list.add(make_special(E1, G1, CASTLING));
        }
        if ((rights & WHITE_OOO) &&
            !(occ & ((1ULL<<B1)|(1ULL<<C1)|(1ULL<<D1))) &&
            !pos.is_attacked(E1, BLACK) &&
            !pos.is_attacked(D1, BLACK) &&
            !pos.is_attacked(C1, BLACK)) {
            list.add(make_special(E1, C1, CASTLING));
        }
    } else {
        if ((rights & BLACK_OO) &&
            !(occ & ((1ULL<<F8)|(1ULL<<G8))) &&
            !pos.is_attacked(E8, WHITE) &&
            !pos.is_attacked(F8, WHITE) &&
            !pos.is_attacked(G8, WHITE)) {
            list.add(make_special(E8, G8, CASTLING));
        }
        if ((rights & BLACK_OOO) &&
            !(occ & ((1ULL<<B8)|(1ULL<<C8)|(1ULL<<D8))) &&
            !pos.is_attacked(E8, WHITE) &&
            !pos.is_attacked(D8, WHITE) &&
            !pos.is_attacked(C8, WHITE)) {
            list.add(make_special(E8, C8, CASTLING));
        }
    }
}

static void generate_pseudo_legal(const Position& pos, MoveList& list, bool captures_only) {
    generate_pawn_moves(pos, list, captures_only);
    generate_piece_moves(pos, list, KNIGHT, captures_only);
    generate_piece_moves(pos, list, BISHOP, captures_only);
    generate_piece_moves(pos, list, ROOK, captures_only);
    generate_piece_moves(pos, list, QUEEN, captures_only);
    generate_piece_moves(pos, list, KING, captures_only);
    if (!captures_only) generate_castling(pos, list);
}

void generate_legal_moves(const Position& pos, MoveList& list) {
    MoveList pseudo;
    generate_pseudo_legal(pos, pseudo, false);
    for (int i = 0; i < pseudo.count; i++) {
        if (pos.is_legal(pseudo.moves[i]))
            list.add(pseudo.moves[i]);
    }
}

void generate_captures(const Position& pos, MoveList& list) {
    MoveList pseudo;
    generate_pseudo_legal(pos, pseudo, true);
    for (int i = 0; i < pseudo.count; i++) {
        if (pos.is_legal(pseudo.moves[i]))
            list.add(pseudo.moves[i]);
    }
}
