# Roadmap

This page tracks planned features, known limitations, and the long-term direction of Infiniti.

> Items are roughly ordered by priority within each section. Nothing here is guaranteed; the roadmap reflects current intent and will change.

---

## In Progress

- **Improved time management** — replace the simple `time / 25 + increment / 2` formula with an adaptive scheme that accounts for position complexity and move history.

---

## Planned: Search

| Feature | Description |
|---------|-------------|
| Aspiration windows | Narrow the root search window around the previous score; re-search with a wider window on failure |
| Internal Iterative Deepening (IID) | At PV nodes with no TT move, do a shallow search first to seed move ordering |
| Singular extensions | Extend the search depth when one move is clearly better than all alternatives |
| History-based pruning | Use history scores to prune or reduce quiet moves more aggressively |
| Opening book support | Optional opening-book integration so common theory does not need to be rediscovered every game |
| Multi-threaded search (Lazy SMP) | Run multiple threads on the same tree with shared hash table |

---

## Planned: Evaluation

| Feature | Description |
|---------|-------------|
| Pawn structure cache | Cache pawn evaluation results (doubled, isolated, connected pawns) keyed by pawn hash |
| King safety | Penalise an exposed king with a scaled attack weight |
| Mobility evaluation | Bonus for the number of legal moves available to each side |
| Rook on open file | Bonus for rooks with no pawns on their file |
| Endgame tablebases | Syzygy tablebase probing for perfect endgame play |

---

## Planned: NNUE

| Feature | Description |
|---------|-------------|
| Self-play data generation | A mode to generate positions + outcomes for network training |
| Bundled default network | Ship a small pre-trained network so NNUE works out of the box |
| HalfKAv2 feature set | Upgrade from HalfKP to the larger HalfKAv2 input features |
| Quantisation improvements | Move to 8-bit weights for faster inference |

---

## Planned: Infrastructure

| Feature | Description |
|---------|-------------|
| Windows CI | Add a Windows runner to the GitHub Actions pipeline |
| Benchmarking suite | A deterministic perft + search benchmark to track performance regressions |
| Fuzzing | Fuzz the FEN parser and UCI input loop with libFuzzer |
| GitHub wiki sync | Automatically mirror `docs/wiki/` to the GitHub wiki on every merge to `main` |

---

## Planned: Web Platform

| Feature | Description |
|---------|-------------|
| UI/UX roadmap execution | Deliver the staged usability, clarity, authentication, performance, and security plan in [UI/UX Roadmap](UI-UX-Roadmap.md) |
| OAuth login | Sign in with GitHub / Google |
| Rate limiting | Per-IP and per-user rate limits on all API endpoints |
| Admin dashboard | UI for reviewing KYC submissions and transaction anomalies |
| Audit log | Immutable log of all sensitive actions (login, KYC approval, password reset) |

---

## Known Limitations

These limitations are known gaps in the current engine, and each one is either tied to an explicit roadmap item or already has a practical workaround.

| Limitation | Current impact | Planned response / workaround |
|------------|----------------|-------------------------------|
| Single-threaded search | The engine uses only one CPU core, so it leaves modern multi-core hardware underused. | Planned: **Multi-threaded search (Lazy SMP)** in the search roadmap. |
| No tablebase support | Endgames with only a few pieces are still approximated by search and evaluation instead of being played perfectly. | Planned: **Endgame tablebases** in the evaluation roadmap. |
| No opening book | The engine spends time searching well-known opening theory from scratch. | Planned: **Opening book support** in the search roadmap. |
| NNUE requires a compatible file | NNUE does not work out of the box unless the user supplies a matching network file, so the engine falls back to HCE by default. | Planned: **Bundled default network** in the NNUE roadmap. Today, users can set `EvalFile` to a compatible `.nnue` file. |

---

## Completed

- ✅ Bitboard-based move generation with full legality checking
- ✅ Negamax with alpha-beta, PVS, NMP, LMR, futility pruning
- ✅ Quiescence search
- ✅ Transposition table with four bound types
- ✅ Tapered HCE (PeSTO PSTs, bishop pair, passed pawns)
- ✅ NNUE (HalfKP, L1=256) evaluator with incremental accumulator
- ✅ Full UCI protocol support
- ✅ Thread-safe `stop` handling with background search and atomic interruption
- ✅ Multi-compiler CI (GCC, Clang, MSVC, AppleClang)
- ✅ CodeQL security scanning and dependency review
- ✅ Node.js web platform (auth, KYC, transactions, favourites)

---

## Feedback

Have a feature idea or found a bug? Open a [GitHub issue](https://github.com/GizzZmo/Infiniti/issues).
