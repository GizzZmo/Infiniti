#pragma once
#include <string>
#include <memory>
#include "../src/types.h"
#include "../src/position.h"

namespace NNUE {

class Evaluator {
public:
    virtual ~Evaluator() = default;
    virtual bool load(const std::string& path) = 0;
    virtual bool is_loaded() const = 0;
    virtual void reset_from_board(const Position& pos) = 0;
    virtual void push() = 0;
    virtual void pop() = 0;
    virtual void on_move_make(const Position& pos, Move m) = 0;
    virtual int evaluate(const Position& pos) = 0;
};

std::unique_ptr<Evaluator> create_evaluator();

} // namespace NNUE
