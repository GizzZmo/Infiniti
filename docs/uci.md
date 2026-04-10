# UCI Protocol Reference

Infiniti implements the [Universal Chess Interface (UCI)](https://www.shredderchess.com/download/div/uci.zip) protocol. This page documents every command the engine recognises and every response it produces.

---

## Commands sent to the engine (stdin)

### `uci`

Identifies the engine and lists all available options.

**Response:**
```
id name Infiniti
id author Infiniti Team
option name Hash type spin default 16 min 1 max 2048
option name UseNNUE type check default true
option name EvalFile type string default
uciok
```

---

### `isready`

Asks the engine whether it is ready to accept commands. The engine responds once all initialisation is complete.

**Response:**
```
readyok
```

---

### `ucinewgame`

Notifies the engine that a new game is starting. Infiniti resets the internal position to the starting position.

---

### `position (startpos | fen <fenstring>) [moves <move1> … <moveN>]`

Sets up the board position.

| Form | Description |
|------|-------------|
| `position startpos` | Standard starting position |
| `position startpos moves e2e4 e7e5` | Starting position after the given moves |
| `position fen <fen>` | Arbitrary position given as a FEN string |
| `position fen <fen> moves <move1> …` | FEN position + move sequence |

Moves are given in **long algebraic notation** (`from``to`, e.g. `e2e4`). Promotion moves append the piece character: `a7a8q` (queen), `a7a8r`, `a7a8b`, `a7a8n`.

---

### `go [options]`

Starts the search. Options can be combined freely.

| Option | Type | Description |
|--------|------|-------------|
| `wtime <ms>` | integer | White's remaining clock in milliseconds |
| `btime <ms>` | integer | Black's remaining clock in milliseconds |
| `winc <ms>` | integer | White's increment per move in milliseconds |
| `binc <ms>` | integer | Black's increment per move in milliseconds |
| `movetime <ms>` | integer | Search exactly this many milliseconds |
| `depth <n>` | integer | Search to exactly this depth |
| `infinite` | flag | Search until `stop` is received |

**Response (one line per completed depth):**
```
info depth <d> seldepth <sd> score cp <cp> nodes <n> nps <nps> time <ms> pv <move1> [<move2> …]
```

Followed by:
```
bestmove <move>
```

---

### `stop`

Stops the current search immediately. The engine outputs `bestmove` with the best move found so far.

---

### `setoption name <name> value <value>`

Sets an engine option. See [Options](#options) below.

---

### `quit`

Exits the engine process.

---

## Options

### `Hash`

| Field | Value |
|-------|-------|
| Type | spin |
| Default | 16 |
| Range | 1 – 2048 |

Size of the transposition table in megabytes. Larger values reduce TT collisions at the cost of more memory usage. Typical values: 64–512 MB for analysis, 16–128 MB for engine-vs-engine play.

> **Note:** Hash resizing takes effect at the next `ucinewgame` or at engine start-up.

---

### `UseNNUE`

| Field | Value |
|-------|-------|
| Type | check |
| Default | true |

When `true` and an NNUE network file has been loaded via `EvalFile`, the engine uses the neural network evaluator. When `false`, the hand-crafted tapered evaluator is always used regardless of whether a network is loaded.

---

### `EvalFile`

| Field | Value |
|-------|-------|
| Type | string |
| Default | *(empty)* |

Path to a binary NNUE network file. The engine attempts to open the file immediately when this option is set. It prints a confirmation or error message to `info string`:

```
info string NNUE file loaded successfully: /path/to/net.nnue
```
```
info string NNUE load failed: cannot open file
```

---

## Info String Fields

| Field | Description |
|-------|-------------|
| `depth` | Completed search depth (plies) |
| `seldepth` | Maximum selective depth reached during quiescence search |
| `score cp <n>` | Score in centipawns from the side to move's perspective |
| `score mate <n>` | Mate in *n* moves (not yet emitted; planned) |
| `nodes` | Total nodes searched |
| `nps` | Nodes per second |
| `time` | Elapsed search time in milliseconds |
| `pv` | Principal variation (sequence of best moves) |
