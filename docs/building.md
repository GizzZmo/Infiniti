# Building Infiniti

## Prerequisites

| Requirement | Minimum version | Notes |
|-------------|----------------|-------|
| C++ compiler | GCC 10 / Clang 12 / MSVC 19.29 | C++20 support required |
| CMake | 3.16 | |
| POSIX threads | any | Linked automatically via `Threads::Threads` |

---

## Quick Start (Linux / macOS)

```bash
git clone https://github.com/GizzZmo/Infiniti.git
cd Infiniti

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

./build/infiniti
```

---

## Quick Start (Windows — MSVC)

Open a **Developer Command Prompt** or **x64 Native Tools Command Prompt**:

```bat
git clone https://github.com/GizzZmo/Infiniti.git
cd Infiniti

cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

build\Release\infiniti.exe
```

---

## Quick Start (Windows — MinGW / MSYS2)

```bash
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

./build/infiniti.exe
```

---

## Build Types

| Type | Flags | Use when |
|------|-------|----------|
| `Release` | `-O2` / `/O2` | Playing games, benchmarking |
| `Debug` | `-g` | Debugging; sanitizers are active in the CI |
| `RelWithDebInfo` | `-O2 -g` | Profiling |

Specify the type with `-DCMAKE_BUILD_TYPE=<type>` on the initial `cmake` configure step.

---

## Compiler Selection

To explicitly choose a compiler, set `CC` / `CXX` before invoking CMake:

```bash
CC=clang CXX=clang++ cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

---

## Running the Tests

The project includes a basic CMake/CTest smoke test:

```bash
cmake --build build
cd build
ctest --output-on-failure
```

---

## Sanitizer Builds

The CI runs with AddressSanitizer and UndefinedBehaviorSanitizer. To replicate locally with CMake:

```bash
cmake -B build-san \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"
cmake --build build-san -j$(nproc)
```

---

## Troubleshooting

**`C++20 not supported`** — upgrade your compiler. GCC 10+, Clang 12+, or MSVC 2019 16.8+ are required.

**`Threads::Threads not found`** — install `libpthread`. On Ubuntu/Debian: `sudo apt install build-essential`.

**Link errors on Windows with MinGW** — make sure you are using a 64-bit toolchain (`x86_64-w64-mingw32`).
