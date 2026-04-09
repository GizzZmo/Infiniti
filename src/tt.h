#pragma once
#include "types.h"
#include <vector>
#include <cstdint>
#include <algorithm>

enum TTBound { TT_NONE=0, TT_EXACT=1, TT_LOWER=2, TT_UPPER=3 };

struct TTEntry {
    uint16_t key16 = 0;
    int16_t  score = 0;
    int16_t  static_eval = 0;
    uint8_t  depth = 0;
    uint8_t  bound_and_pv = 0;
    Move     move = MOVE_NULL;
};

class TranspositionTable {
public:
    explicit TranspositionTable(size_t mb = 16) { resize(mb); }

    void resize(size_t mb) {
        num_entries = (mb * 1024 * 1024) / sizeof(TTEntry);
        if (num_entries == 0) num_entries = 1;
        table.assign(num_entries, TTEntry{});
    }

    void clear() {
        std::fill(table.begin(), table.end(), TTEntry{});
    }

    TTEntry* probe(uint64_t key, bool& found) const {
        size_t idx = key % num_entries;
        TTEntry* e = const_cast<TTEntry*>(&table[idx]);
        found = (e->key16 == uint16_t(key >> 48)) && (e->bound_and_pv & 3) != TT_NONE;
        return e;
    }

    void store(uint64_t key, Move m, int score, int static_eval, int depth, TTBound bound, bool is_pv) {
        size_t idx = key % num_entries;
        TTEntry& e = table[idx];
        uint16_t k16 = uint16_t(key >> 48);
        if (e.key16 != k16 || depth >= e.depth || bound == TT_EXACT) {
            e.key16 = k16;
            e.score = int16_t(score);
            e.static_eval = int16_t(static_eval);
            e.depth = uint8_t(std::min(depth, 255));
            e.bound_and_pv = uint8_t(bound | (is_pv ? 4 : 0));
            if (m != MOVE_NULL) e.move = m;
        }
    }

private:
    std::vector<TTEntry> table;
    size_t num_entries = 1;
};
