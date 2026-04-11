#pragma once
#include "position.h"
#include "tt.h"
#include "../nnue/nnue.h"
#include <atomic>
#include <chrono>
#include <vector>
#include <cstdint>

constexpr int INF_SCORE = 32000;
constexpr int MATE_SCORE = 31000;
constexpr int MAX_PLY = 128;

struct SearchLimits {
    int depth = 0;
    int movetime_ms = 0;
    int wtime_ms = 0, btime_ms = 0;
    int winc_ms = 0, binc_ms = 0;
    int movestogo = 0;
    bool infinite = false;
};

struct SearchResult {
    Move best_move = MOVE_NULL;
    int score = 0;
    int depth = 0;
};

class Searcher {
public:
    Searcher();
    void set_position(const Position& pos) { root_pos = pos; }
    void set_history(const std::vector<uint64_t>& hist) { game_history = hist; }
    void set_nnue(NNUE::Evaluator* eval) { nnue_eval = eval; }
    SearchResult go(const SearchLimits& limits);
    void stop() { stop_flag = true; }
    void new_game() { tt.clear(); clear_history(); }
    void resize_tt(size_t mb) { tt.resize(mb); }

private:
    Position root_pos;
    TranspositionTable tt;
    std::atomic<bool> stop_flag{false};
    std::chrono::time_point<std::chrono::steady_clock> search_start;
    int64_t node_count = 0;
    int seldepth = 0;
    int time_limit_ms = 0;
    NNUE::Evaluator* nnue_eval = nullptr;

    Move killers[MAX_PLY][2];
    int history[64][64];
    std::vector<uint64_t> game_history;
    uint64_t search_path[MAX_PLY];

    bool time_up() const;
    void clear_history();

    int do_evaluate(const Position& pos) const;
    int negamax(Position& pos, int depth, int alpha, int beta, int ply, bool is_pv, bool do_null);
    int quiescence(Position& pos, int alpha, int beta, int ply);
    int score_move(const Position& pos, Move m, Move tt_move, int ply) const;

    void print_info(int depth, int score, const std::vector<Move>& pv) const;

    static const int MVV_LVA[7][7];
};
