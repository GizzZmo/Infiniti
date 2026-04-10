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
4. Open a pull request against `main` with a clear description of what changed and why.

---

## Code Style

- C++20; match the style of the surrounding file.
- Prefer `constexpr` and `inline` over macros.
- No external dependencies beyond the C++ standard library and POSIX threads.
- Keep each translation unit focused on one component.

---

## License

By contributing you agree that your changes will be released under the [MIT License](../LICENSE).
