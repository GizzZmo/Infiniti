#pragma once
#include "position.h"
#include "search.h"
#include "../nnue/nnue.h"
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <cstdint>

class UCI {
public:
    UCI();
    ~UCI();
    void loop();

private:
    Position pos;
    Searcher searcher;
    std::unique_ptr<NNUE::Evaluator> nnue_evaluator;
    bool use_nnue = true;
    std::string eval_file;
    bool nnue_loaded = false;

    std::thread search_thread;
    std::vector<uint64_t> game_history;

    void cmd_uci();
    void cmd_isready();
    void cmd_ucinewgame();
    void cmd_position(const std::string& line);
    void cmd_go(const std::string& line);
    void cmd_setoption(const std::string& line);
    void cmd_stop();
    void cmd_quit();
    void cmd_d();
    void cmd_perft(const std::string& line);

    void stop_search();

    static std::string move_to_str(Move m);
    static Move str_to_move(const Position& pos, const std::string& s);
};
