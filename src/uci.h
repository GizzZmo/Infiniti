#pragma once
#include "position.h"
#include "search.h"
#include <string>

class UCI {
public:
    UCI();
    void loop();

private:
    Position pos;
    Searcher searcher;
    bool use_nnue = false;
    std::string eval_file;
    bool nnue_loaded = false;

    void cmd_uci();
    void cmd_isready();
    void cmd_ucinewgame();
    void cmd_position(const std::string& line);
    void cmd_go(const std::string& line);
    void cmd_setoption(const std::string& line);
    void cmd_stop();
    void cmd_quit();

    static std::string move_to_str(Move m);
    static Move str_to_move(const Position& pos, const std::string& s);
};
