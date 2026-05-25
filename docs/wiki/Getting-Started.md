# Getting Started with Infiniti

This guide walks you through getting Infiniti up and running — from cloning the repository to playing your first game.

---

## Prerequisites

Before you begin, make sure you have:

| Tool | Minimum version | Where to get it |
|------|----------------|-----------------|
| C++ compiler | GCC 10 / Clang 12 / MSVC 19.29 | [gcc.gnu.org](https://gcc.gnu.org) · [releases.llvm.org](https://releases.llvm.org) · [visualstudio.microsoft.com](https://visualstudio.microsoft.com) |
| CMake | 3.16 | [cmake.org/download](https://cmake.org/download/) |
| Git | any | [git-scm.com](https://git-scm.com) |

---

## Step 1 — Clone the Repository

```bash
git clone https://github.com/GizzZmo/Infiniti.git
cd Infiniti
```

---

## Step 2 — Build the Engine

### Linux / macOS

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

The compiled binary is at `build/infiniti`.

### Windows (MSVC)

Open a **x64 Native Tools Command Prompt** (from the Visual Studio start menu):

```bat
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The binary is at `build\Release\infiniti.exe`.

### Windows (MinGW / MSYS2)

```bash
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

The binary is at `build/infiniti.exe`.

---

## Step 3 — Verify the Build

Run a quick sanity check:

```bash
./build/infiniti
```

Type `uci` and press Enter. You should see:

```
id name Infiniti
id author Infiniti Team
option name Hash type spin default 16 min 1 max 2048
option name UseNNUE type check default true
option name EvalFile type string default
uciok
```

Type `quit` to exit.

---

## Step 4 — Run the Test Suite

```bash
cd build
ctest --output-on-failure
```

All tests should pass. If any fail, see [Building — Troubleshooting](../building.md#troubleshooting).

---

## Step 5 — Play a Game

### Option A: With a GUI (recommended)

1. Download a free UCI-compatible GUI such as [Cute Chess](https://cutechess.com/) or [Arena](http://www.playwitharena.de/).
2. Add a new engine and point it at the `infiniti` binary.
3. Set the protocol to **UCI**.
4. Start a game — enjoy!

### Option B: Command line

Start the engine and interact with it directly:

```
$ ./build/infiniti
uci
...
uciok
isready
readyok
position startpos
go movetime 2000
info depth 1 ...
bestmove e2e4
quit
```

---

## Step 6 (Optional) — Load an NNUE Network

For stronger play, load a compatible NNUE network file:

```
setoption name EvalFile value /path/to/network.nnue
setoption name UseNNUE value true
```

The engine confirms the load with:

```
info string NNUE file loaded successfully: /path/to/network.nnue
```

Without a network file, Infiniti automatically uses its built-in tapered hand-crafted evaluator.

---

## Step 7 (Optional) — Run the Web Platform

```bash
cd web
npm install
npm run start
```

Open `http://localhost:3000`. See [web/README.md](../../web/README.md) for environment variable configuration.

---

## Troubleshooting

See [Building — Troubleshooting](../building.md#troubleshooting) for common build errors, and the [FAQ](../faq.md) for usage questions.

---

## Next Steps

- Read the [UCI Reference](../uci.md) to understand all available commands and options.
- Explore the [Architecture](Architecture.md) wiki page to understand how the engine works internally.
- Check the [Roadmap](Roadmap.md) to see what features are coming.
- Read [Contributing](../contributing.md) if you want to improve the engine.
