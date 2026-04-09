# Search Algorithm

Infiniti uses **iterative deepening negamax** with **alpha-beta pruning** and several modern enhancements. This page explains each component.

---

## Iterative Deepening

The root search (`Searcher::go`) runs a loop from depth 1 up to the maximum allowed depth. After each iteration the best move and score are recorded. If the time limit expires mid-iteration the result from the *previous* completed depth is returned.

Benefits:
- The best move from depth *d* seeds the move ordering for depth *d+1*, improving pruning.
- A result is always available to return even if time expires early.

---

## Negamax

The internal search function (`Searcher::negamax`) uses the negamax formulation of minimax: the score is always from the perspective of the side to move, so the recursive call negates the child score.

```
negamax(pos, depth, α, β, ply, is_pv, do_null)
  → score from side-to-move's perspective
```

---

## Principal Variation Search (PVS)

After the first move is searched with the full `[α, β]` window, subsequent moves are searched with a **zero window** `[α, α+1]`. If the zero-window search unexpectedly raises alpha, a costly full-window re-search is performed.

PVS pays off because in a well-ordered tree the first move is frequently the best; the zero-window searches for the remaining moves are much faster.

---

## Transposition Table (TT)

Before expanding any node, the TT is probed for a previous result at the same position hash.

| Bound type | Meaning | Used when |
|------------|---------|-----------|
| `TT_EXACT` | Exact score | Full-window search completed |
| `TT_LOWER` | Score ≥ stored value (fail-high) | A beta cutoff was stored |
| `TT_UPPER` | Score ≤ stored value (fail-low) | No move exceeded alpha |

If a usable TT hit is found (sufficient depth, non-PV node), the stored score is returned directly without searching.

The TT also provides the **best move** from a previous iteration, which is tried first during move ordering.

---

## Null Move Pruning (NMP)

Applies when:
- Not in check
- Not a PV node
- Depth ≥ 3
- The side to move has at least one non-king piece

The side to move is allowed to "pass" (null move). If the resulting position still causes a beta cutoff, the current node is likely a fail-high regardless of our best move, so `beta` is returned immediately.

**Reduction** `R`:
- `R = 3` when `depth ≥ 6`
- `R = 2` otherwise

---

## Futility Pruning

At depth 1, quiet moves (non-captures, non-promotions) are skipped when:

```
static_eval + 200 ≤ alpha
```

The 200 cp margin represents the maximum realistic gain a single quiet move can contribute.

---

## Late Move Reductions (LMR)

Moves searched after the first three, at depth ≥ 3, that are quiet (non-capture, non-promotion) and do not give check are reduced by `R` plies before the full search:

| Move index | R |
|------------|---|
| 3–5 | 1 |
| 6+ | 2 |

If the reduced search raises alpha, the move is re-searched at full depth.

---

## Check Extension

When making a move puts the opponent in check, the search depth for that branch is increased by 1 ply:

```cpp
if (pos.in_check()) new_depth++;
```

This prevents the engine from missing tactical sequences involving check.

---

## Quiescence Search

After reaching depth 0, the search continues with only captures (and en passant) via `Searcher::quiescence`. This avoids the **horizon effect** — prematurely evaluating a position in the middle of a capture sequence.

The quiescence search uses the **stand-pat score** (static evaluation) as a lower bound: if the static evaluation already beats beta, the position is returned immediately.

---

## Move Ordering

Good move ordering is critical for alpha-beta pruning efficiency. Moves are scored and sorted before each node expansion:

| Priority | Move type | Score |
|----------|-----------|-------|
| 1 | TT best move | 1 000 000 |
| 2 | Winning/neutral captures (MVV-LVA) | 100 000 + MVV-LVA bonus |
| 3 | Promotions | 90 000 |
| 4 | Killer move 1 (same ply) | 80 000 |
| 5 | Killer move 2 (same ply) | 79 000 |
| 6 | Quiet moves | history\[from\]\[to\] |

### MVV-LVA

Most Valuable Victim / Least Valuable Attacker. A capture by a pawn of a queen scores higher than a queen capturing a pawn. The table is indexed `MVV_LVA[attacker_type][victim_type]`.

### Killer Moves

Two quiet moves that caused a beta cutoff at the same ply are remembered and tried before other quiet moves.

### History Heuristic

Quiet moves that cause beta cutoffs accumulate a bonus of `depth²`. The entire table is halved whenever any entry exceeds 10 000 to prevent overflow.

---

## Fifty-Move Rule

If `halfmove_clock ≥ 100` at any non-root node, a draw score (0) is returned immediately.
