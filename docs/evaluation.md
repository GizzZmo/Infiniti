# Evaluation

When NNUE is disabled (or no network file is loaded), Infiniti uses a hand-crafted evaluator (`src/eval.cpp`) that returns a score in **centipawns** from the perspective of the side to move.

---

## Tapered Evaluation

The evaluator blends a **middlegame (MG)** score and an **endgame (EG)** score based on the remaining material on the board, a technique known as *tapered evaluation*.

```
score = (mg_score × phase + eg_score × (256 − phase)) / 256
```

### Game Phase

Phase is computed from the non-pawn, non-king material:

| Piece | Phase weight |
|-------|-------------|
| Knight | 1 |
| Bishop | 1 |
| Rook | 2 |
| Queen | 4 |

The maximum phase value is 24 (full piece set for both sides). It is scaled to the range `[0, 256]`. A phase of 256 means pure middlegame; 0 means pure endgame.

---

## Material Values

| Piece | Middlegame (cp) | Endgame (cp) |
|-------|----------------|-------------|
| Pawn | 82 | 94 |
| Knight | 337 | 281 |
| Bishop | 365 | 297 |
| Rook | 477 | 512 |
| Queen | 1025 | 936 |
| King | 20000 | 20000 |

The large king value prevents the engine from ever trading the king; it is never actually used in a material-balance calculation.

---

## Piece-Square Tables (PSTs)

Each piece has separate 8×8 MG and EG tables that add a bonus or penalty based on which square the piece occupies. The tables are stored from White's perspective (a1 = index 0, h8 = index 63). For Black pieces the square is mirrored vertically (`sq ^ 56`).

The PST values are sourced from the well-known PeSTO tables, which have been tuned on large game databases and are a strong baseline for hand-crafted evaluation.

### Example: Pawn MG table (rank 2 = bottom)

```
 0   0   0   0   0   0   0   0   ← rank 8
98 134  61  95  68 126  34 -11   ← rank 7
-6   7  26  31  65  56  25 -20   ← rank 6
...
 0   0   0   0   0   0   0   0   ← rank 1
```

Pawns on the 7th rank (almost promoted) receive a large bonus; pawns on their starting squares receive 0.

---

## Bonus Terms

### Bishop Pair

A side owning two or more bishops receives:

| Phase | Bonus |
|-------|-------|
| Middlegame | +30 cp |
| Endgame | +50 cp |

The larger endgame bonus reflects the increased power of bishops in open endgames.

### Passed Pawns

A pawn is *passed* if no opposing pawn can ever block or capture it — i.e., there are no enemy pawns on the same file or adjacent files ahead of it.

The bonus scales with the pawn's advancement rank `r` (0 = starting rank, 7 = promotion rank):

| Phase | Bonus formula |
|-------|---------------|
| Middlegame | `5 + r × 10` cp |
| Endgame | `10 + r² × 5` cp |

The quadratic endgame bonus strongly rewards very advanced passed pawns that are close to promoting.

---

## Score Sign Convention

The evaluator always returns a score from the **side to move's** perspective:
- Positive → good for the side to move
- Negative → bad for the side to move (opponent is better)

This matches the negamax convention used throughout the search.

---

## Switching to NNUE

Set the UCI option `EvalFile` to a valid NNUE network path and ensure `UseNNUE` is `true`. The `evaluate()` function in `eval.cpp` is the fallback; when NNUE is active the `NNUE::Evaluator` interface is used instead. See [NNUE](nnue.md) for details.
