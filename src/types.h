#pragma once
#include <cstdint>

using Bitboard = uint64_t;
using Move = uint32_t;

enum Color { WHITE = 0, BLACK = 1, NO_COLOR = 2 };
enum PieceType { NO_PIECE_TYPE = 0, PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6 };
enum Piece {
    NO_PIECE = 0,
    W_PAWN = 1, W_KNIGHT = 2, W_BISHOP = 3, W_ROOK = 4, W_QUEEN = 5, W_KING = 6,
    B_PAWN = 9, B_KNIGHT = 10, B_BISHOP = 11, B_ROOK = 12, B_QUEEN = 13, B_KING = 14
};
enum Square {
    A1=0,B1,C1,D1,E1,F1,G1,H1,
    A2,B2,C2,D2,E2,F2,G2,H2,
    A3,B3,C3,D3,E3,F3,G3,H3,
    A4,B4,C4,D4,E4,F4,G4,H4,
    A5,B5,C5,D5,E5,F5,G5,H5,
    A6,B6,C6,D6,E6,F6,G6,H6,
    A7,B7,C7,D7,E7,F7,G7,H7,
    A8,B8,C8,D8,E8,F8,G8,H8,
    NO_SQUARE = 64
};
enum File { FILE_A=0,FILE_B,FILE_C,FILE_D,FILE_E,FILE_F,FILE_G,FILE_H };
enum Rank { RANK_1=0,RANK_2,RANK_3,RANK_4,RANK_5,RANK_6,RANK_7,RANK_8 };
enum MoveType { NORMAL=0, CASTLING=1, EN_PASSANT=2, PROMOTION=3 };
enum CastlingRights { NO_CASTLING=0, WHITE_OO=1, WHITE_OOO=2, BLACK_OO=4, BLACK_OOO=8, ANY_CASTLING=15 };

constexpr Move MOVE_NULL = 0;

inline Color color_of(Piece p) { return (p >= B_PAWN) ? BLACK : (p == NO_PIECE ? NO_COLOR : WHITE); }
inline PieceType type_of(Piece p) { return PieceType(p & 7); }
inline Piece make_piece(Color c, PieceType pt) { return Piece(pt + (c == BLACK ? 8 : 0)); }
inline File file_of(Square s) { return File(s & 7); }
inline Rank rank_of(Square s) { return Rank(s >> 3); }
inline Square make_square(File f, Rank r) { return Square(r * 8 + f); }

inline Move make_move(Square from, Square to) { return Move(from) | (Move(to) << 6); }
inline Move make_special(Square from, Square to, MoveType type, PieceType promo = NO_PIECE_TYPE) {
    return Move(from) | (Move(to) << 6) | (Move(type) << 12) | (Move(promo) << 14);
}
inline Square from_sq(Move m) { return Square(m & 0x3F); }
inline Square to_sq(Move m) { return Square((m >> 6) & 0x3F); }
inline MoveType move_type(Move m) { return MoveType((m >> 12) & 3); }
inline PieceType promo_type(Move m) { return PieceType((m >> 14) & 7); }

inline Color operator~(Color c) { return Color(c ^ 1); }
inline Square operator+(Square s, int d) { return Square(int(s) + d); }
inline Square& operator++(Square& s) { return s = Square(int(s) + 1); }

// Maps a Piece value to its standard ASCII character (uppercase = white, lowercase = black).
// Index layout: [0]='.', [1-6]=PNBRQK, [7-8]=unused, [9-14]=pnbrqk
inline char piece_to_char(Piece p) {
    constexpr char kChars[] = ".PNBRQKxxpnbrqk";
    return kChars[p];
}
