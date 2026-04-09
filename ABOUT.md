# About Infiniti

## What is Infiniti?

Infiniti is an open-source UCI chess engine written in modern C++ (C++20). It is designed to be readable, well-structured, and easy to extend, while still implementing the core techniques that make a chess engine competitive: bitboard-based move generation, alpha-beta search with modern pruning, and a tapered hand-crafted evaluator backed by optional NNUE (Neural Network-based Unified Evaluation) support.

The engine speaks the [Universal Chess Interface (UCI)](docs/uci.md) protocol, which means it works out of the box with any UCI-compatible graphical chess interface such as Arena, Cute Chess, Lucas Chess, BanksiaGUI, or ChessBase.

---

## Goals

- **Correctness first** — a move generator that produces only legal moves, a position state that correctly handles all edge cases (en passant, castling rights, the fifty-move rule, Zobrist hashing).
- **Readable code** — each component is separated into its own translation unit with a clear, minimal public interface.
- **Modern C++** — C++20 features are used where they improve clarity (e.g., `constexpr`, `[[nodiscard]]`, scoped enums).
- **Extensibility** — the NNUE evaluator is accessed through a pure virtual interface (`NNUE::Evaluator`), making it easy to swap or upgrade the network architecture.

---

## History

Infiniti was started in 2026 by Jon Arve Ovesen as a personal project to explore chess programming techniques from first principles. The name reflects the aspiration to push the engine's strength toward the theoretical maximum — an infinite search over a perfectly evaluated tree.

---

## Architecture at a Glance

```
┌─────────────────────────────────────────────┐
│                   main.cpp                  │
│  init_bitboards()  →  UCI::loop()           │
└────────────────┬────────────────────────────┘
                 │  UCI commands
     ┌───────────▼───────────┐
     │      uci.cpp          │   position / go / setoption
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
    │           eval.cpp                  │
    │  Tapered HCE (PSTs, bishop pair,    │
    │  passed pawns)  ←─── or NNUE ───►  nnue/
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

---

## Key Technical Details

| Component | Detail |
|-----------|--------|
| **Language** | C++20 |
| **Build system** | CMake 3.16+ |
| **Board representation** | Bitboards (`uint64_t`) |
| **Move encoding** | 32-bit packed integer (from, to, type, promo) |
| **Hashing** | Zobrist (random keys for piece/square, side, castling, en passant) |
| **Search** | Iterative deepening · Negamax · PVS · NMP · LMR · Futility · QSearch |
| **Evaluation** | Tapered HCE with piece-square tables + NNUE (HalfKP, L1=256) |
| **TT** | Single-bucket; stores score, static eval, depth, bound, PV flag, best move |
| **Move ordering** | TT move → MVV-LVA → promotions → killers → history heuristic |
| **UCI options** | `Hash`, `UseNNUE`, `EvalFile` |

---

## License

Infiniti is released under the [MIT License](LICENSE).  
© 2026 Jon Arve Ovesen
