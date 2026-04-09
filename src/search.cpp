#include "search.h"
#include "movegen.h"
#include "eval.h"
#include "bitboard.h"
#include <algorithm>
#include <iostream>
#include <vector>
#include <cstring>

const int Searcher::MVV_LVA[7][7] = {
    {0,0,0,0,0,0,0},
    {0,15,14,13,12,11,10},
    {0,25,24,23,22,21,20},
    {0,35,34,33,32,31,30},
    {0,45,44,43,42,41,40},
    {0,55,54,53,52,51,50},
    {0,65,64,63,62,61,60},
};

Searcher::Searcher() : tt(16) {
    clear_history();
}

void Searcher::clear_history() {
    memset(killers, 0, sizeof(killers));
    memset(history, 0, sizeof(history));
}

bool Searcher::time_up() const {
    if (stop_flag) return true;
    if (time_limit_ms <= 0) return false;
    auto now = std::chrono::steady_clock::now();
    int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - search_start).count();
    return elapsed >= time_limit_ms;
}

int Searcher::score_move(const Position& pos, Move m, Move tt_move, int ply) const {
    if (m == tt_move) return 1000000;
    Square from = from_sq(m);
    Square to = to_sq(m);
    Piece captured = pos.piece_on(to);
    if (move_type(m) == EN_PASSANT) captured = make_piece(~pos.side_to_move(), PAWN);

    if (captured != NO_PIECE) {
        PieceType atk = type_of(pos.piece_on(from));
        PieceType vic = type_of(captured);
        return 100000 + MVV_LVA[atk][vic] * 100;
    }
    if (move_type(m) == PROMOTION) return 90000;

    if (ply < MAX_PLY) {
        if (m == killers[ply][0]) return 80000;
        if (m == killers[ply][1]) return 79000;
    }
    return history[from][to];
}

int Searcher::quiescence(Position& pos, int alpha, int beta, int ply) {
    if (time_up()) return 0;
    if (ply > seldepth) seldepth = ply;

    int stand_pat = evaluate(pos);
    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;

    MoveList captures;
    generate_captures(pos, captures);

    std::sort(captures.moves, captures.moves + captures.count, [&](Move a, Move b) {
        return score_move(pos, a, MOVE_NULL, ply) > score_move(pos, b, MOVE_NULL, ply);
    });

    for (int i = 0; i < captures.count; i++) {
        node_count++;
        StateInfo si;
        pos.make_move(captures.moves[i], si);
        int score = -quiescence(pos, -beta, -alpha, ply + 1);
        pos.unmake_move(captures.moves[i], si);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

int Searcher::negamax(Position& pos, int depth, int alpha, int beta, int ply, bool is_pv, bool do_null) {
    if (time_up()) return 0;

    if (depth <= 0) return quiescence(pos, alpha, beta, ply);

    node_count++;
    if (ply > seldepth) seldepth = ply;

    if (ply > 0 && pos.halfmove_clock() >= 100) return 0;

    bool tt_found = false;
    TTEntry* tte = tt.probe(pos.hash(), tt_found);
    Move tt_move = MOVE_NULL;
    if (tt_found) {
        tt_move = tte->move;
        if (!is_pv && tte->depth >= depth) {
            TTBound bound = TTBound(tte->bound_and_pv & 3);
            int tt_score = tte->score;
            if (bound == TT_EXACT) return tt_score;
            if (bound == TT_LOWER && tt_score >= beta) return tt_score;
            if (bound == TT_UPPER && tt_score <= alpha) return tt_score;
        }
    }

    bool in_check = pos.in_check();
    int static_eval = evaluate(pos);

    // Null move pruning
    if (do_null && !in_check && !is_pv && depth >= 3 &&
        popcount(pos.pieces(pos.side_to_move())) > 1) {
        int R = (depth >= 6) ? 3 : 2;
        StateInfo si;
        pos.make_null_move(si);
        int null_score = -negamax(pos, depth - 1 - R, -beta, -beta + 1, ply + 1, false, false);
        pos.unmake_null_move(si);
        if (null_score >= beta) return beta;
    }

    // Futility pruning
    bool futility = false;
    if (!in_check && !is_pv && depth == 1 && static_eval + 200 <= alpha) {
        futility = true;
    }

    MoveList moves;
    generate_legal_moves(pos, moves);

    if (moves.count == 0) {
        return in_check ? -(MATE_SCORE - ply) : 0;
    }

    std::sort(moves.moves, moves.moves + moves.count, [&](Move a, Move b) {
        return score_move(pos, a, tt_move, ply) > score_move(pos, b, tt_move, ply);
    });

    int best_score = -INF_SCORE;
    Move best_move = MOVE_NULL;
    TTBound bound = TT_UPPER;

    for (int i = 0; i < moves.count; i++) {
        Move m = moves.moves[i];
        bool is_capture = pos.piece_on(to_sq(m)) != NO_PIECE || move_type(m) == EN_PASSANT;
        bool is_promotion = move_type(m) == PROMOTION;

        if (futility && !is_capture && !is_promotion && !in_check) continue;

        StateInfo si;
        pos.make_move(m, si);

        int new_depth = depth - 1;
        if (pos.in_check()) new_depth++;

        int score;
        if (!is_pv && i >= 3 && depth >= 3 && !is_capture && !is_promotion && !in_check && !pos.in_check()) {
            int R = 1 + (i >= 6 ? 1 : 0);
            score = -negamax(pos, new_depth - R, -alpha - 1, -alpha, ply + 1, false, true);
            if (score > alpha) {
                score = -negamax(pos, new_depth, -alpha - 1, -alpha, ply + 1, false, true);
            }
        } else if (!is_pv || i > 0) {
            score = -negamax(pos, new_depth, -alpha - 1, -alpha, ply + 1, false, true);
        } else {
            score = -negamax(pos, new_depth, -beta, -alpha, ply + 1, true, true);
        }

        if (is_pv && i > 0 && score > alpha && score < beta) {
            score = -negamax(pos, new_depth, -beta, -alpha, ply + 1, true, true);
        }

        pos.unmake_move(m, si);

        if (time_up()) return 0;

        if (score > best_score) {
            best_score = score;
            best_move = m;
            if (score > alpha) {
                alpha = score;
                bound = TT_EXACT;
                if (score >= beta) {
                    bound = TT_LOWER;
                    if (!is_capture && ply < MAX_PLY) {
                        if (m != killers[ply][0]) {
                            killers[ply][1] = killers[ply][0];
                            killers[ply][0] = m;
                        }
                        history[from_sq(m)][to_sq(m)] += depth * depth;
                        if (history[from_sq(m)][to_sq(m)] > 10000)
                            for (int f = 0; f < 64; f++)
                                for (int t = 0; t < 64; t++)
                                    history[f][t] /= 2;
                    }
                    tt.store(pos.hash(), best_move, best_score, static_eval, depth, TT_LOWER, is_pv);
                    return best_score;
                }
            }
        }
    }

    tt.store(pos.hash(), best_move, best_score, static_eval, depth, bound, is_pv);
    return best_score;
}

void Searcher::print_info(int depth, int score, const std::vector<Move>& pv) const {
    // UCI info is printed inline in go(); this stub is reserved for future use.
    (void)depth; (void)score; (void)pv;
}

SearchResult Searcher::go(const SearchLimits& limits) {
    stop_flag = false;
    node_count = 0;
    seldepth = 0;
    search_start = std::chrono::steady_clock::now();
    clear_history();

    time_limit_ms = 0;
    if (!limits.infinite) {
        if (limits.movetime_ms > 0) {
            time_limit_ms = limits.movetime_ms;
        } else {
            Color stm = root_pos.side_to_move();
            int time_left = (stm == WHITE) ? limits.wtime_ms : limits.btime_ms;
            int inc = (stm == WHITE) ? limits.winc_ms : limits.binc_ms;
            if (time_left > 0) {
                time_limit_ms = time_left / 25 + inc / 2;
                if (time_limit_ms > time_left - 50)
                    time_limit_ms = std::max(time_left - 50, 10);
            }
        }
    }

    int max_depth = limits.depth > 0 ? limits.depth : 100;

    SearchResult result;
    result.best_move = MOVE_NULL;
    result.score = 0;
    result.depth = 0;

    MoveList root_moves;
    generate_legal_moves(root_pos, root_moves);
    if (root_moves.count == 0) return result;
    result.best_move = root_moves.moves[0];

    for (int depth = 1; depth <= max_depth; depth++) {
        seldepth = 0;
        Position pos = root_pos;
        int score = negamax(pos, depth, -INF_SCORE, INF_SCORE, 0, true, false);

        if (time_up() && depth > 1) break;

        std::vector<Move> pv;
        Position pv_pos = root_pos;
        for (int i = 0; i < depth; i++) {
            bool found;
            TTEntry* e = tt.probe(pv_pos.hash(), found);
            if (!found || e->move == MOVE_NULL) break;
            pv.push_back(e->move);
            StateInfo si;
            pv_pos.make_move(e->move, si);
        }
        if (!pv.empty()) result.best_move = pv[0];
        result.score = score;
        result.depth = depth;

        auto now = std::chrono::steady_clock::now();
        int64_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - search_start).count();
        int64_t nps = elapsed_ms > 0 ? (node_count * 1000 / elapsed_ms) : 0;

        std::cout << "info depth " << depth
                  << " seldepth " << seldepth
                  << " score cp " << score
                  << " nodes " << node_count
                  << " nps " << nps
                  << " time " << elapsed_ms
                  << " pv";
        for (Move m : pv) {
            Square from = from_sq(m);
            Square to = to_sq(m);
            std::cout << " " << char('a' + file_of(from)) << char('1' + rank_of(from))
                      << char('a' + file_of(to)) << char('1' + rank_of(to));
            if (move_type(m) == PROMOTION) {
                static const char promo_chars[] = ".pnbrqk";
                std::cout << promo_chars[promo_type(m)];
            }
        }
        std::cout << std::endl;

        if (time_up()) break;
    }

    return result;
}
