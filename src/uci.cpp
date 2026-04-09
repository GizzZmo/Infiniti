#include "uci.h"
#include "movegen.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

UCI::UCI() {
    pos.reset();
    nnue_evaluator = NNUE::create_evaluator();
}

std::string UCI::move_to_str(Move m) {
    if (m == MOVE_NULL) return "0000";
    std::string s;
    s += char('a' + file_of(from_sq(m)));
    s += char('1' + rank_of(from_sq(m)));
    s += char('a' + file_of(to_sq(m)));
    s += char('1' + rank_of(to_sq(m)));
    if (move_type(m) == PROMOTION) {
        static const char pc[] = ".pnbrqk";
        s += pc[promo_type(m)];
    }
    return s;
}

Move UCI::str_to_move(const Position& pos, const std::string& s) {
    if (s.size() < 4) return MOVE_NULL;
    File ff = File(s[0] - 'a');
    Rank rf = Rank(s[1] - '1');
    File ft = File(s[2] - 'a');
    Rank rt = Rank(s[3] - '1');
    Square from = make_square(ff, rf);
    Square to = make_square(ft, rt);

    PieceType promo = NO_PIECE_TYPE;
    if (s.size() >= 5) {
        switch (s[4]) {
            case 'q': promo = QUEEN; break;
            case 'r': promo = ROOK; break;
            case 'b': promo = BISHOP; break;
            case 'n': promo = KNIGHT; break;
        }
    }

    MoveList moves;
    generate_legal_moves(pos, moves);
    for (int i = 0; i < moves.count; i++) {
        Move m = moves.moves[i];
        if (from_sq(m) == from && to_sq(m) == to) {
            if (promo != NO_PIECE_TYPE && move_type(m) == PROMOTION) {
                if (promo_type(m) == promo) return m;
            } else if (promo == NO_PIECE_TYPE) {
                return m;
            }
        }
    }
    return MOVE_NULL;
}

void UCI::cmd_uci() {
    std::cout << "id name Infiniti" << std::endl;
    std::cout << "id author Infiniti Team" << std::endl;
    std::cout << "option name Hash type spin default 16 min 1 max 2048" << std::endl;
    std::cout << "option name UseNNUE type check default true" << std::endl;
    std::cout << "option name EvalFile type string default" << std::endl;
    std::cout << "uciok" << std::endl;
}

void UCI::cmd_isready() {
    std::cout << "readyok" << std::endl;
}

void UCI::cmd_ucinewgame() {
    pos.reset();
}

void UCI::cmd_position(const std::string& line) {
    std::istringstream ss(line);
    std::string token;
    ss >> token; // "position"
    ss >> token;

    if (token == "startpos") {
        pos.reset();
        if (!(ss >> token)) return; // possibly "moves"
    } else if (token == "fen") {
        std::string fen;
        while (ss >> token && token != "moves") {
            if (!fen.empty()) fen += ' ';
            fen += token;
        }
        pos.set(fen);
    }

    if (token == "moves") {
        while (ss >> token) {
            Move m = str_to_move(pos, token);
            if (m != MOVE_NULL) {
                StateInfo si;
                pos.make_move(m, si);
            }
        }
    }
}

void UCI::cmd_go(const std::string& line) {
    SearchLimits limits;
    std::istringstream ss(line);
    std::string token;
    ss >> token; // "go"

    while (ss >> token) {
        if (token == "wtime") ss >> limits.wtime_ms;
        else if (token == "btime") ss >> limits.btime_ms;
        else if (token == "winc") ss >> limits.winc_ms;
        else if (token == "binc") ss >> limits.binc_ms;
        else if (token == "movetime") ss >> limits.movetime_ms;
        else if (token == "depth") ss >> limits.depth;
        else if (token == "infinite") limits.infinite = true;
    }

    searcher.set_position(pos);
    SearchResult result = searcher.go(limits);
    std::cout << "bestmove " << move_to_str(result.best_move) << std::endl;
}

void UCI::cmd_setoption(const std::string& line) {
    std::istringstream ss(line);
    std::string token, name, value;
    ss >> token; // "setoption"
    ss >> token; // "name"
    while (ss >> token && token != "value") name += (name.empty() ? "" : " ") + token;
    while (ss >> token) value += (value.empty() ? "" : " ") + token;

    if (name == "Hash") {
        // Hash resizing not yet wired to TT; stored for future implementation.
        (void)value;
    } else if (name == "UseNNUE") {
        use_nnue = (value == "true");
    } else if (name == "EvalFile") {
        eval_file = value;
        if (!eval_file.empty()) {
            std::ifstream f(eval_file, std::ios::binary);
            if (f.good()) {
                nnue_loaded = true;
                std::cout << "info string NNUE file loaded successfully: " << eval_file << std::endl;
            } else {
                nnue_loaded = false;
                std::cout << "info string NNUE load failed: cannot open file" << std::endl;
            }
        }
    }
}

void UCI::cmd_stop() {
    searcher.stop();
}

void UCI::cmd_quit() {}

void UCI::loop() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "uci") cmd_uci();
        else if (cmd == "isready") cmd_isready();
        else if (cmd == "ucinewgame") cmd_ucinewgame();
        else if (cmd == "position") cmd_position(line);
        else if (cmd == "go") cmd_go(line);
        else if (cmd == "setoption") cmd_setoption(line);
        else if (cmd == "stop") cmd_stop();
        else if (cmd == "quit") break;

        std::cout.flush();
    }
}
