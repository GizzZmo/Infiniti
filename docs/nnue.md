# NNUE Evaluation

Infiniti supports **NNUE** (Efficiently Updatable Neural Network) evaluation as an optional replacement for the hand-crafted evaluator.

---

## What is NNUE?

NNUE is a class of neural network architectures designed specifically for board games. The key insight is that board positions change incrementally between moves — only a few pieces move — so the first network layer (the *accumulator*) can be updated incrementally rather than recomputed from scratch. This makes inference fast enough to run at the leaves of a chess search tree.

NNUE was first introduced in Stockfish and has since become the standard evaluation method for top chess engines.

---

## Architecture

Infiniti uses a **HalfKP** (Half King-Piece) input feature set:

| Layer | Type | Size |
|-------|------|------|
| Input features | HalfKP | Up to 40 960 active features (2 perspectives) |
| L1 (accumulator) | ClippedReLU | 256 neurons × 2 perspectives = 512 inputs to L2 |
| L2 | Linear + ClippedReLU | 32 neurons |
| L3 | Linear + ClippedReLU | 32 neurons |
| Output | Linear | 1 scalar (centipawn score) |

### HalfKP Features

HalfKP encodes the position as a set of (king square, piece type, piece square, piece color) tuples, computed separately for each side. Each of the 2 perspectives contributes `HALFKP_FEATURE_COUNT` features to the L1 accumulator.

The feature count is defined in `nnue/features_halfkp.h`:

```cpp
constexpr int HALFKP_FEATURE_COUNT = ...;  // typically 40 960
```

### Accumulator

The `Accumulator` class maintains a running sum of the active feature weights for each perspective. It supports:

- `reset(bias)` — initialise from the L1 bias vector
- `add_feature(idx, weights)` — activate a feature
- `remove_feature(idx, weights)` — deactivate a feature

The `NNUE::Evaluator` interface wraps the accumulator and exposes `push()` / `pop()` for stack-based incremental updates during search make/unmake.

### Network Forward Pass

`network_evaluate(weights, acc, stm)` takes the two accumulator halves (ordered by `stm` — the side to move goes first), concatenates them, and runs them through L2 → L3 → output.

Integer quantisation is used throughout: weights are `int8_t` / `int16_t`, activations are clipped to `[0, 127]`.

---

## Network File Format

The engine loads weights from a binary file specified via the `EvalFile` UCI option. The file is read by `nnue/nnue_file.cpp`.

> The exact binary layout depends on the network file used. Infiniti's reader expects a layout matching its `NetworkWeights` struct (see `nnue/network.h`).

---

## Enabling NNUE

1. Obtain or train a compatible NNUE network file (`.nnue`).
2. Start the engine and send:

```
setoption name EvalFile value /path/to/your/network.nnue
setoption name UseNNUE value true
```

3. The engine prints confirmation:

```
info string NNUE file loaded successfully: /path/to/your/network.nnue
```

4. Search as normal — the neural network evaluator is now active.

---

## Disabling NNUE

```
setoption name UseNNUE value false
```

The engine falls back to the tapered hand-crafted evaluator described in [Evaluation](evaluation.md).

---

## Source Files

| File | Purpose |
|------|---------|
| `nnue/nnue.h` / `nnue.cpp` | Public `Evaluator` interface and factory function |
| `nnue/nnue_file.h` / `nnue_file.cpp` | Binary network file loader |
| `nnue/features_halfkp.h` / `features_halfkp.cpp` | HalfKP feature index computation |
| `nnue/accumulator.h` / `accumulator.cpp` | Incremental L1 accumulator |
| `nnue/network.h` / `network.cpp` | L2 → L3 → output forward pass |
