# Architecture

This page provides a deep-dive into how Infiniti is structured and why each design decision was made.

---

## High-Level Overview

```
┌─────────────────────────────────────────────┐
│                   main.cpp                  │
│  init_bitboards()  →  UCI::loop()           │
└────────────────┬────────────────────────────┘
                 │  UCI text commands (stdin)
     ┌───────────▼───────────┐
     │      uci.cpp          │   position / go / setoption / quit
     │  UCI protocol handler │
     └───┬───────────────────┘
         │
    ┌────▼────────────────────────────────┐
    │           search.cpp                │
    │  Iterative deepening                │
    │  Negamax + PVS + pruning            │
    │  Move ordering (TT / MVV-LVA /      │
    │  killers / history)                 │
    └────┬────────────────────────────────┘
         │  evaluate()
    ┌────▼────────────────────────────────┐
    │           eval.cpp  /  nnue/        │
    │  Tapered HCE (PSTs, bishop pair,    │
    │  passed pawns)  ←─── or NNUE ───►  │
    └────┬────────────────────────────────┘
         │
    ┌────▼────────────────────────────────┐
    │         position.cpp                │
    │  Bitboard state, make/unmake,       │
    │  FEN, Zobrist hashing               │
    └────┬────────────────────────────────┘
         │
    ┌────▼────────────────────────────────┐
    │         movegen.cpp                 │
    │  Legal move generation              │
    │  Capture generation (QSearch)       │
    └─────────────────────────────────────┘
```

Data flows top-down: the UCI handler sets the position and calls the searcher; the searcher calls the evaluator at leaf nodes; the evaluator reads position state; the move generator enumerates legal moves at every node.

---

## Board Representation

### Bitboards

The board is represented as a set of `uint64_t` bitboards — one per (piece type, colour) pair. Each bit corresponds to a square (bit 0 = a1, bit 63 = h8). Bulk operations (attacks, fills, shifts) can then be performed with a single CPU instruction rather than looping over squares.

### Position State

`Position` (in `position.h/.cpp`) holds:

- 12 piece bitboards (6 piece types × 2 colours)
- Side to move
- Castling rights (4 bits)
- En passant square (or `NO_SQ`)
- Halfmove clock (fifty-move rule)
- Fullmove number
- Zobrist hash (64-bit)

### Make / Unmake

Rather than copying the full position state for each node, Infiniti uses an **unmake** approach: `make_move` applies a move and saves the minimal undo information in an `UndoInfo` stack; `unmake_move` reverses it. This avoids heap allocation and cache misses from copying 64+ bytes per node.

### Zobrist Hashing

Each (piece, square) combination, plus side-to-move, castling rights, and en passant file, is assigned a random 64-bit key at startup. The position hash is the XOR of all active keys. After a make/unmake pair the hash is restored exactly, with no recomputation.

---

## Move Generation

`movegen.cpp` generates **fully legal moves** directly, never pseudo-legal moves. This avoids the overhead of a legality filter but requires more bookkeeping:

1. Find the set of squares attacked by the opponent (for king safety).
2. Detect check; if double check, only king moves are generated.
3. Compute the pin mask for each pinned piece.
4. Generate moves piece by piece, restricting pinned pieces to their pin ray.

Captures (for quiescence search) are a subset of the full generator, gated by a non-empty target mask.

---

## Search

See [docs/search.md](../search.md) for the full explanation. At a high level:

| Technique | Why |
|-----------|-----|
| Iterative deepening | Allows anytime behaviour (return best result if time runs out) and seeds move ordering |
| PVS (Principal Variation Search) | Searches the first move with a full window, the rest with a null window — cuts tree size significantly when move ordering is good |
| Transposition table | Caches results indexed by Zobrist hash; avoids re-searching transpositions |
| Null move pruning | Lets the opponent move twice; if they still can't beat β, the node is likely a cut-node |
| LMR (Late Move Reductions) | Reduces depth for late, quiet moves — saves time on moves unlikely to be best |
| Futility pruning | Skips quiet moves at depth 1 when the static eval is far below α |
| Quiescence search | Extends the search through capture sequences to avoid the horizon effect |

### Move Ordering

Good move ordering is essential for alpha-beta efficiency. Moves are scored and sorted at each node:

1. TT best move (1 000 000)
2. Winning / neutral captures ordered by MVV-LVA (100 000 + bonus)
3. Promotions (90 000)
4. Killer moves (80 000 / 79 000)
5. Quiet moves by history heuristic score

---

## Evaluation

See [docs/evaluation.md](../evaluation.md) for numeric details.

### Tapered HCE

The hand-crafted evaluator interpolates between a middlegame and endgame score based on the remaining non-pawn material (the *game phase*). At full material the score is pure middlegame; as pieces come off the board the endgame score gains weight.

Each piece contributes:
- A **material value** (different for MG and EG)
- A **piece-square table bonus** (PeSTO tables, separate MG / EG tables)

Additional terms: bishop pair bonus, passed pawn bonus (scales quadratically with advancement in EG).

### NNUE

When a network file is loaded and `UseNNUE` is `true`, the `NNUE::Evaluator` interface replaces the HCE. The evaluator is accessed through a pure virtual interface, making it easy to swap network architectures.

The HalfKP architecture encodes the position as (king square, piece type, piece square, colour) tuples for each of the two sides. The first layer (the accumulator) is updated incrementally — only the changed features are added or removed — rather than recomputed from scratch after each move.

---

## UCI Protocol Handler

`uci.cpp` implements a simple loop: read a line, tokenise, dispatch to a handler function. It owns the `Position` and `Searcher` objects and passes them by reference into sub-handlers.

Supported commands: `uci`, `isready`, `ucinewgame`, `position`, `go`, `stop`, `setoption`, `ponderhit`, `debug`, `register`, `d`, `perft`, `quit`.

The `go` command runs the search on a dedicated thread and prints `bestmove` when done. `stop` is implemented via an atomic stop flag checked throughout search.

---

## Web Platform

The Node.js web platform in `web/` is a completely separate application that shares no code with the engine. It provides REST APIs for:

- User registration, email verification, password reset
- TOTP two-factor authentication with recovery codes
- KYC document upload and admin review
- Transaction filtering, search, and CSV export
- Favourite games management

The platform uses SQLite (`web/data/app.db`) for persistence and optionally AWS S3 for KYC document storage.

---

## Source File Map

| File | Responsibility |
|------|---------------|
| `src/types.h` | Core types: `Bitboard`, `Move`, `Square`, `Color`, `PieceType` |
| `src/bitboard.h/.cpp` | Bitboard utilities, attack tables, initialisation |
| `src/position.h/.cpp` | Board state, FEN parsing, make/unmake move, Zobrist |
| `src/movegen.h/.cpp` | Legal move and capture generation |
| `src/eval.h/.cpp` | Tapered HCE with PSTs, bishop pair, passed pawns |
| `src/tt.h` | Transposition table |
| `src/search.h/.cpp` | Iterative deepening, negamax, PVS, pruning, ordering |
| `src/uci.h/.cpp` | UCI protocol loop and option handling |
| `src/main.cpp` | Entry point |
| `nnue/nnue.h/.cpp` | Public `Evaluator` interface and factory |
| `nnue/nnue_file.h/.cpp` | Binary network file loader |
| `nnue/features_halfkp.h/.cpp` | HalfKP feature index computation |
| `nnue/accumulator.h/.cpp` | Incremental L1 accumulator |
| `nnue/network.h/.cpp` | L2 → L3 → output forward pass |
