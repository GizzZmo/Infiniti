#pragma once
#include "types.h"
#include <string>
#include <array>

struct StateInfo {
    Square ep_sq;
    int castling;
    int hmclock;
    Piece captured;
    uint64_t key;
};

class Position {
public:
    Position();

    Bitboard pieces(Color c, PieceType pt) const { return bb_pieces[c][pt]; }
    Bitboard pieces(Color c) const;
    Bitboard pieces(PieceType pt) const;
    Bitboard all_pieces() const;

    Piece piece_on(Square sq) const { return board[sq]; }
    Color side_to_move() const { return stm; }
    Square ep_square() const { return ep_sq; }
    int castling_rights() const { return castling; }
    int halfmove_clock() const { return hmclock; }
    int fullmove_number() const { return fmnum; }
    uint64_t hash() const { return key; }

    bool in_check() const;
    bool is_attacked(Square sq, Color by) const;
    Bitboard attackers_to(Square sq, Bitboard occ) const;

    void set(const std::string& fen);
    std::string fen() const;
    void reset();

    void make_move(Move m, StateInfo& si);
    void unmake_move(Move m, const StateInfo& si);
    void make_null_move(StateInfo& si);
    void unmake_null_move(const StateInfo& si);

    bool is_legal(Move m) const;
    bool gives_check(Move m) const;

    Square king_square(Color c) const;

private:
    Bitboard bb_pieces[2][7];
    Piece board[64];
    Color stm;
    Square ep_sq;
    int castling;
    int hmclock;
    int fmnum;
    uint64_t key;

    void put_piece(Piece p, Square sq);
    void remove_piece(Square sq);
    void move_piece(Square from, Square to);
};

namespace Zobrist {
    void init();
    extern uint64_t psq[2][7][64];
    extern uint64_t side;
    extern uint64_t castling[16];
    extern uint64_t ep[8];
}
