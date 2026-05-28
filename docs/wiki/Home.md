# Infiniti Wiki

Welcome to the Infiniti wiki — a collection of in-depth guides and background articles that go beyond the reference documentation.

---

## Navigation

| Page | What you will find |
|------|--------------------|
| [Getting Started](Getting-Started.md) | Step-by-step guide from cloning to your first game |
| [Architecture](Architecture.md) | Deep-dive into the engine internals and design decisions |
| [Roadmap](Roadmap.md) | Planned features and future direction |
| [UI/UX Roadmap](UI-UX-Roadmap.md) | Web platform interface priorities and delivery phases |

---

## Quick Reference

| Topic | Link |
|-------|------|
| Build instructions | [docs/building.md](../building.md) |
| UCI command reference | [docs/uci.md](../uci.md) |
| Search internals | [docs/search.md](../search.md) |
| Evaluation internals | [docs/evaluation.md](../evaluation.md) |
| NNUE evaluation | [docs/nnue.md](../nnue.md) |
| Frequently asked questions | [docs/faq.md](../faq.md) |
| Contributing guide | [docs/contributing.md](../contributing.md) |

---

## About Infiniti

Infiniti is an open-source UCI chess engine written in C++20. It combines:

- **Bitboard-based move generation** for fast, bulk bit-manipulation of board state
- **Iterative deepening negamax** with Principal Variation Search (PVS) and modern pruning
- **Tapered hand-crafted evaluation** (HCE) using PeSTO piece-square tables
- **Optional NNUE evaluation** via a HalfKP neural network loaded at runtime
- **A Node.js web platform** for authentication, KYC, and game management

The engine is designed to be readable and extensible, with each component in its own translation unit and a clear public interface.

---

## Contributing

Contributions are welcome. Please read [Contributing](../contributing.md) before opening a pull request.

© 2026 Jon Arve Ovesen — [MIT License](../../LICENSE)
