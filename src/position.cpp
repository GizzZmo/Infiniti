#include "position.h"
#include "bitboard.h"
#include <sstream>
#include <random>
#include <cassert>
#include <cmath>

namespace Zobrist {
    uint64_t psq[2][7][64];
    uint64_t side;
    uint64_t castling[16];
    uint64_t ep[8];

    void init() {
        static bool initialized = false;
        if (initialized) return;
        initialized = true;
        std::mt19937_64 rng(0x1234567890ABCDEFULL);
        for (int c = 0; c < 2; c++)
            for (int pt = 0; pt < 7; pt++)
                for (int sq = 0; sq < 64; sq++)
                    psq[c][pt][sq] = rng();
        side = rng();
        for (int i = 0; i < 16; i++) castling[i] = rng();
        for (int i = 0; i < 8; i++) ep[i] = rng();
    }
}

Position::Position() {
    reset();
}

void Position::put_piece(Piece p, Square sq) {
    board[sq] = p;
    Color c = color_of(p);
    PieceType pt = type_of(p);
    bb_pieces[c][pt] |= (1ULL << sq);
    key ^= Zobrist::psq[c][pt][sq];
}

void Position::remove_piece(Square sq) {
    Piece p = board[sq];
    if (p == NO_PIECE) return;
    Color c = color_of(p);
    PieceType pt = type_of(p);
    bb_pieces[c][pt] &= ~(1ULL << sq);
    key ^= Zobrist::psq[c][pt][sq];
    board[sq] = NO_PIECE;
}

void Position::move_piece(Square from, Square to) {
    Piece p = board[from];
    Color c = color_of(p);
    PieceType pt = type_of(p);
    Bitboard mask = (1ULL << from) | (1ULL << to);
    bb_pieces[c][pt] ^= mask;
    key ^= Zobrist::psq[c][pt][from] ^ Zobrist::psq[c][pt][to];
    board[to] = p;
    board[from] = NO_PIECE;
}

Bitboard Position::pieces(Color c) const {
    Bitboard b = 0;
    for (int pt = 1; pt <= 6; pt++) b |= bb_pieces[c][pt];
    return b;
}

Bitboard Position::pieces(PieceType pt) const {
    return bb_pieces[WHITE][pt] | bb_pieces[BLACK][pt];
}

Bitboard Position::all_pieces() const {
    return pieces(WHITE) | pieces(BLACK);
}

Square Position::king_square(Color c) const {
    Bitboard b = bb_pieces[c][KING];
    if (!b) return NO_SQUARE;
    return Square(lsb(b));
}

bool Position::is_attacked(Square sq, Color by) const {
    Bitboard occ = all_pieces();
    if (PawnAttacks[~by][sq] & bb_pieces[by][PAWN]) return true;
    if (KnightAttacks[sq] & bb_pieces[by][KNIGHT]) return true;
    if (KingAttacks[sq] & bb_pieces[by][KING]) return true;
    if (bishop_attacks(sq, occ) & (bb_pieces[by][BISHOP] | bb_pieces[by][QUEEN])) return true;
    if (rook_attacks(sq, occ) & (bb_pieces[by][ROOK] | bb_pieces[by][QUEEN])) return true;
    return false;
}

Bitboard Position::attackers_to(Square sq, Bitboard occ) const {
    Bitboard result = 0;
    result |= PawnAttacks[BLACK][sq] & bb_pieces[WHITE][PAWN];
    result |= PawnAttacks[WHITE][sq] & bb_pieces[BLACK][PAWN];
    result |= KnightAttacks[sq] & (bb_pieces[WHITE][KNIGHT] | bb_pieces[BLACK][KNIGHT]);
    result |= KingAttacks[sq] & (bb_pieces[WHITE][KING] | bb_pieces[BLACK][KING]);
    result |= bishop_attacks(sq, occ) & (bb_pieces[WHITE][BISHOP] | bb_pieces[BLACK][BISHOP] |
                                          bb_pieces[WHITE][QUEEN] | bb_pieces[BLACK][QUEEN]);
    result |= rook_attacks(sq, occ) & (bb_pieces[WHITE][ROOK] | bb_pieces[BLACK][ROOK] |
                                        bb_pieces[WHITE][QUEEN] | bb_pieces[BLACK][QUEEN]);
    return result;
}

bool Position::in_check() const {
    return is_attacked(king_square(stm), ~stm);
}

void Position::reset() {
    set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Position::set(const std::string& fen) {
    for (int i = 0; i < 64; i++) board[i] = NO_PIECE;
    for (int c = 0; c < 2; c++)
        for (int pt = 0; pt < 7; pt++)
            bb_pieces[c][pt] = 0;
    key = 0;
    ep_sq = NO_SQUARE;
    castling = 0;
    hmclock = 0;
    fmnum = 1;

    std::istringstream ss(fen);
    std::string token;

    ss >> token;
    int sq = 56;
    for (char c : token) {
        if (c == '/') {
            sq -= 16;
        } else if (c >= '1' && c <= '8') {
            sq += c - '0';
        } else {
            static const std::string piece_chars = "PNBRQKpnbrqk";
            static const PieceType pts[] = {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
                                             PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};
            int idx = (int)piece_chars.find(c);
            if (idx != (int)std::string::npos) {
                Color pc = (idx >= 6) ? BLACK : WHITE;
                put_piece(make_piece(pc, pts[idx]), Square(sq));
                sq++;
            }
        }
    }

    ss >> token;
    stm = (token == "b") ? BLACK : WHITE;
    if (stm == BLACK) key ^= Zobrist::side;

    ss >> token;
    if (token.find('K') != std::string::npos) castling |= WHITE_OO;
    if (token.find('Q') != std::string::npos) castling |= WHITE_OOO;
    if (token.find('k') != std::string::npos) castling |= BLACK_OO;
    if (token.find('q') != std::string::npos) castling |= BLACK_OOO;
    key ^= Zobrist::castling[castling];

    ss >> token;
    if (token != "-") {
        File f = File(token[0] - 'a');
        Rank r = Rank(token[1] - '1');
        ep_sq = make_square(f, r);
        key ^= Zobrist::ep[f];
    }

    if (ss >> token) hmclock = std::stoi(token);
    if (ss >> token) fmnum = std::stoi(token);
}

std::string Position::fen() const {
    std::string result;
    for (int r = 7; r >= 0; r--) {
        int empty = 0;
        for (int f = 0; f < 8; f++) {
            Square sq = Square(r * 8 + f);
            Piece p = board[sq];
            if (p == NO_PIECE) {
                empty++;
            } else {
                if (empty) { result += char('0' + empty); empty = 0; }
                static const char pchars[] = ".PNBRQKxpnbrqk";
                result += pchars[p];
            }
        }
        if (empty) result += char('0' + empty);
        if (r > 0) result += '/';
    }
    result += ' ';
    result += (stm == WHITE ? 'w' : 'b');
    result += ' ';
    if (castling == 0) result += '-';
    else {
        if (castling & WHITE_OO) result += 'K';
        if (castling & WHITE_OOO) result += 'Q';
        if (castling & BLACK_OO) result += 'k';
        if (castling & BLACK_OOO) result += 'q';
    }
    result += ' ';
    if (ep_sq == NO_SQUARE) result += '-';
    else {
        result += char('a' + file_of(ep_sq));
        result += char('1' + rank_of(ep_sq));
    }
    result += ' ';
    result += std::to_string(hmclock);
    result += ' ';
    result += std::to_string(fmnum);
    return result;
}

void Position::make_move(Move m, StateInfo& si) {
    si.ep_sq = ep_sq;
    si.castling = castling;
    si.hmclock = hmclock;
    si.key = key;

    Square from = from_sq(m);
    Square to = to_sq(m);
    MoveType mt = move_type(m);
    Piece moving = board[from];
    Piece captured = (mt == EN_PASSANT) ? make_piece(~stm, PAWN) : board[to];
    si.captured = captured;

    if (ep_sq != NO_SQUARE) {
        key ^= Zobrist::ep[file_of(ep_sq)];
        ep_sq = NO_SQUARE;
    }

    key ^= Zobrist::castling[castling];

    hmclock++;

    if (mt == NORMAL) {
        if (captured != NO_PIECE) {
            remove_piece(to);
            hmclock = 0;
        }
        move_piece(from, to);
        if (type_of(moving) == PAWN) {
            hmclock = 0;
            if (std::abs(int(rank_of(to)) - int(rank_of(from))) == 2) {
                ep_sq = Square((int(from) + int(to)) / 2);
                key ^= Zobrist::ep[file_of(ep_sq)];
            }
        }
    } else if (mt == CASTLING) {
        bool kingside = (to > from);
        Square rook_from, rook_to;
        if (stm == WHITE) {
            rook_from = kingside ? H1 : A1;
            rook_to   = kingside ? F1 : D1;
        } else {
            rook_from = kingside ? H8 : A8;
            rook_to   = kingside ? F8 : D8;
        }
        move_piece(from, to);
        move_piece(rook_from, rook_to);
    } else if (mt == EN_PASSANT) {
        Square cap_sq = make_square(file_of(to), rank_of(from));
        remove_piece(cap_sq);
        move_piece(from, to);
        hmclock = 0;
    } else if (mt == PROMOTION) {
        if (captured != NO_PIECE) remove_piece(to);
        remove_piece(from);
        put_piece(make_piece(stm, promo_type(m)), to);
        hmclock = 0;
    }

    static const int castling_mask[64] = {
        ~WHITE_OOO, 15, 15, 15, ~(WHITE_OO|WHITE_OOO), 15, 15, ~WHITE_OO,
        15,15,15,15,15,15,15,15,
        15,15,15,15,15,15,15,15,
        15,15,15,15,15,15,15,15,
        15,15,15,15,15,15,15,15,
        15,15,15,15,15,15,15,15,
        15,15,15,15,15,15,15,15,
        ~BLACK_OOO, 15, 15, 15, ~(BLACK_OO|BLACK_OOO), 15, 15, ~BLACK_OO
    };
    castling &= castling_mask[from];
    castling &= castling_mask[to];
    key ^= Zobrist::castling[castling];

    if (stm == BLACK) fmnum++;

    stm = ~stm;
    key ^= Zobrist::side;
}

void Position::unmake_move(Move m, const StateInfo& si) {
    stm = ~stm;
    if (stm == BLACK) fmnum--;

    Square from = from_sq(m);
    Square to = to_sq(m);
    MoveType mt = move_type(m);

    if (mt == NORMAL || mt == EN_PASSANT) {
        move_piece(to, from);
        if (mt == EN_PASSANT) {
            Square cap_sq = make_square(file_of(to), rank_of(from));
            put_piece(make_piece(~stm, PAWN), cap_sq);
        } else if (si.captured != NO_PIECE) {
            put_piece(si.captured, to);
        }
    } else if (mt == CASTLING) {
        bool kingside = (to > from);
        Square rook_from, rook_to;
        if (stm == WHITE) {
            rook_from = kingside ? H1 : A1;
            rook_to   = kingside ? F1 : D1;
        } else {
            rook_from = kingside ? H8 : A8;
            rook_to   = kingside ? F8 : D8;
        }
        move_piece(to, from);
        move_piece(rook_to, rook_from);
    } else if (mt == PROMOTION) {
        remove_piece(to);
        put_piece(make_piece(stm, PAWN), from);
        if (si.captured != NO_PIECE) put_piece(si.captured, to);
    }

    ep_sq = si.ep_sq;
    castling = si.castling;
    hmclock = si.hmclock;
    key = si.key;
}

void Position::make_null_move(StateInfo& si) {
    si.ep_sq = ep_sq;
    si.castling = castling;
    si.hmclock = hmclock;
    si.key = key;
    si.captured = NO_PIECE;

    if (ep_sq != NO_SQUARE) {
        key ^= Zobrist::ep[file_of(ep_sq)];
        ep_sq = NO_SQUARE;
    }
    stm = ~stm;
    key ^= Zobrist::side;
    hmclock++;
}

void Position::unmake_null_move(const StateInfo& si) {
    stm = ~stm;
    ep_sq = si.ep_sq;
    castling = si.castling;
    hmclock = si.hmclock;
    key = si.key;
}

bool Position::is_legal(Move m) const {
    Position copy = *this;
    StateInfo si;
    copy.make_move(m, si);
    return !copy.is_attacked(copy.king_square(~copy.stm), copy.stm);
}

bool Position::gives_check(Move m) const {
    Position copy = *this;
    StateInfo si;
    copy.make_move(m, si);
    return copy.in_check();
}
