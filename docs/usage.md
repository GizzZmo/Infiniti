# Using Infiniti

## With a Chess GUI

Infiniti speaks the standard UCI protocol, so it works with any UCI-compatible chess interface.

### Popular GUIs

| GUI | Platform | Notes |
|-----|----------|-------|
| [Arena](http://www.playwitharena.de/) | Windows | Free, feature-rich |
| [Cute Chess](https://cutechess.com/) | Win / Linux / macOS | Good for engine tournaments |
| [BanksiaGUI](https://banksiagui.com/) | Win / Linux / macOS | Modern, actively developed |
| [Lucas Chess](https://lucaschess.pythonanywhere.com/) | Windows | Training-focused |
| [Scid vs. PC](http://scidvspc.sourceforge.net/) | Win / Linux / macOS | Database + engine analysis |

### Adding Infiniti in Cute Chess

1. Open **Tools → Settings → Engines → Add**.
2. Set **Command** to the full path of the `infiniti` binary.
3. Set **Protocol** to **UCI**.
4. Click **OK** — Infiniti now appears in the engine list.

### Adding Infiniti in Arena

1. Open **Engines → Install New Engine**.
2. Browse to the `infiniti` binary.
3. Select **UCI** as the protocol when prompted.

---

## Command-Line (UCI Shell)

The engine reads UCI commands from standard input. You can interact with it directly in a terminal:

```
$ ./build/infiniti
uci
id name Infiniti
id author Infiniti Team
option name Hash type spin default 16 min 1 max 2048
option name UseNNUE type check default true
option name EvalFile type string default
uciok
isready
readyok
```

### Search from the starting position for 3 seconds

```
position startpos
go movetime 3000
info depth 1 seldepth 1 score cp 27 nodes 312 nps 312000 time 1 pv e2e4
info depth 2 seldepth 2 score cp 18 nodes 894 nps 447000 time 2 pv e2e4 e7e5
...
bestmove e2e4
```

### Search to a fixed depth

```
position startpos moves e2e4 e7e5 g1f3
go depth 10
```

### Search from a FEN string

```
position fen r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3
go movetime 2000
```

### Load an NNUE network and search

```
setoption name EvalFile value /path/to/network.nnue
setoption name UseNNUE value true
position startpos
go depth 12
```

---

## Time Management

When a GUI sends `wtime` / `btime` / `winc` / `binc`, Infiniti computes a per-move time budget as:

```
time_limit = time_left / 25 + increment / 2
```

This allocation is capped so that the engine never uses more than `time_left - 50 ms` on a single move.

You can override time management by using `go movetime <ms>` (fixed milliseconds) or `go depth <n>` (fixed depth) or `go infinite` (search until `stop`).
