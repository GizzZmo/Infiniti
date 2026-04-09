#include "nnue.h"
#include "nnue_file.h"
#include "network.h"
#include "accumulator.h"
#include "features_halfkp.h"
#include "../src/eval.h"
#include "../src/bitboard.h"
#include <stack>

namespace NNUE {

class NNUEEvaluator : public Evaluator {
public:
    NNUEEvaluator() : loaded_(false) {}

    bool load(const std::string& path) override {
        std::string err = load_nnue_file(path);
        loaded_ = err.empty();
        return loaded_;
    }

    bool is_loaded() const override { return loaded_; }

    void reset_from_board(const Position& pos) override {
        (void)pos;
        current_acc = AccumulatorEntry{};
        std::array<int16_t, L1_SIZE> zero_bias{};
        acc_white.reset(zero_bias);
        acc_black.reset(zero_bias);
    }

    void push() override {
        acc_stack.push(current_acc);
    }

    void pop() override {
        if (!acc_stack.empty()) {
            current_acc = acc_stack.top();
            acc_stack.pop();
        }
    }

    void on_move_make(const Position& pos, Move m) override {
        (void)pos; (void)m;
    }

    int evaluate(const Position& pos) override {
        return ::evaluate(pos);
    }

private:
    bool loaded_;
    NetworkWeights weights_;
    AccumulatorEntry current_acc;
    Accumulator acc_white, acc_black;
    std::stack<AccumulatorEntry> acc_stack;
};

std::unique_ptr<Evaluator> create_evaluator() {
    return std::make_unique<NNUEEvaluator>();
}

} // namespace NNUE
