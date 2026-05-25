# Contributing

Bug reports, feature requests, and pull requests are welcome.

---

## Reporting Issues

Open a [GitHub issue](https://github.com/GizzZmo/Infiniti/issues) and include:

- A minimal reproducible example (FEN + UCI command sequence if applicable)
- Expected vs. actual behaviour
- Compiler, OS, and CMake version

---

## Pull Requests

1. Fork the repository and create a branch from `main`.
2. Make your changes. Keep commits focused and descriptive.
3. Ensure the project builds cleanly: `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build`.
4. Run the test suite: `cd build && ctest --output-on-failure`
5. For web platform changes, also run: `cd web && npm run lint && npm run build && npm test`
6. Open a pull request against `main` with a clear description of what changed and why.

---

## Branch Naming

Use a short, descriptive branch name with a prefix that reflects the type of change:

| Prefix | Use for |
|--------|---------|
| `feat/` | New features |
| `fix/` | Bug fixes |
| `docs/` | Documentation-only changes |
| `refactor/` | Code refactoring without behaviour change |
| `test/` | Test additions or improvements |
| `ci/` | CI/CD pipeline changes |

Example: `feat/aspiration-windows`, `fix/castle-rights-bug`, `docs/faq-update`

---

## Commit Messages

Follow the [Conventional Commits](https://www.conventionalcommits.org/) style:

```
<type>(<optional scope>): <short summary>

[optional body]
```

Examples:
- `feat(search): add aspiration windows`
- `fix(movegen): correct en passant pin detection`
- `docs: update FAQ with NNUE loading steps`

---

## Code Style

- C++20; match the style of the surrounding file.
- Prefer `constexpr` and `inline` over macros.
- No external dependencies beyond the C++ standard library and POSIX threads.
- Keep each translation unit focused on one component.
- Run `clang-tidy` (configured in the CI) before submitting; fix all warnings.

---

## Running CI Locally

The CI checks can be approximated locally:

```bash
# C++ engine
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure

# Web platform
cd web
npm install
npm run lint
npm run build
npm test
```

---

## License

By contributing you agree that your changes will be released under the [MIT License](../LICENSE).
