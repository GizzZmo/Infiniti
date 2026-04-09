#pragma once
#include "types.h"
#include "position.h"

struct MoveList {
    Move moves[256];
    int count = 0;
    void add(Move m) { moves[count++] = m; }
    const Move* begin() const { return moves; }
    const Move* end() const { return moves + count; }
};

void generate_legal_moves(const Position& pos, MoveList& list);
void generate_captures(const Position& pos, MoveList& list);
