# Programming Assignment 2 — Distance Vector Routing

**Course:** [COSC-370]  
**Authors:** EJ Knights, Lei Tapungot  
**Platform:** Linux (Ubuntu / WSL)

---

## Overview

This project implements a **distributed, asynchronous Distance Vector (DV) routing algorithm** across four simulated network nodes. Each node independently maintains a distance table and exchanges routing packets with its directly-connected neighbors, converging on optimal shortest-path costs using the Bellman-Ford equation.

The node files (`Node0.c` – `Node3.c`) plug into a provided network simulator (`Prog3.c`) which handles packet delivery, event scheduling, and timing.

---

## Network Topology

```
        1           1
  [0] ------- [1] ------- [2]
   |                       |
 7 |                       | 2
   |                       |
  [3] ----------------------
        (not direct — via 2)
```

| Link  | Cost |
|-------|------|
| 0 ↔ 1 | 1    |
| 0 ↔ 2 | 3    |
| 0 ↔ 3 | 7    |
| 1 ↔ 2 | 1    |
| 2 ↔ 3 | 2    |
| 1 ↔ 3 | ∞ (not connected) |

---

## Files

| File | Description |
|------|-------------|
| `Prog3.c` | Simulator provided by instructor — **do not modify** |
| `Node0.c` | DV routing for node 0 (neighbors: 1, 2, 3) |
| `Node1.c` | DV routing for node 1 (neighbors: 0, 2) |
| `Node2.c` | DV routing for node 2 (neighbors: 0, 1, 3) |
| `Node3.c` | DV routing for node 3 (neighbors: 0, 2) |

---

## How to Compile

```bash
cc Prog3.c Node0.c Node1.c Node2.c Node3.c -o dvrouting
```

> Warnings from `Prog3.c` about implicit function declarations and return types are expected — the simulator uses K&R-style C from the 1990s. They do not affect correctness.

---

## How to Run

```bash
./dvrouting
```

When prompted:
```
Enter TRACE: 2
```

| Trace Level | Output |
|-------------|--------|
| `0` | No output (silent run) |
| `1` | Basic simulator events |
| `2` | Full trace — packet arrivals, table updates, broadcasts *(use this for demo)* |

The simulator runs until no packets remain in transit, then prints:
```
Simulator terminated at t=..., no packets in medium
```

---

## Expected Convergence

After the algorithm converges, the minimum-cost paths are:

|        | to 0 | to 1 | to 2 | to 3 |
|--------|------|------|------|------|
| **Node 0** | 0 | 1 | 2 | 4 |
| **Node 1** | 1 | 0 | 1 | 3 |
| **Node 2** | 2 | 1 | 0 | 2 |
| **Node 3** | 4 | 3 | 2 | 0 |

> Note: Node 0 reaches Node 2 at cost **2** via the path `0→1→2`, not via the direct link (cost 3). The algorithm discovers this automatically.

---

## Algorithm Design

Each node file follows the same structure:

### `rtinit()`
1. Fill the 4×4 distance table with `INF` (9999)
2. Seed direct link costs: `dt[v][v] = lc[v]` for each direct neighbor
3. Compute per-destination minimums
4. Broadcast initial cost vector to all direct neighbors via `tolayer2()`

### `rtupdate()`
Bellman-Ford relaxation on every incoming packet:
```
dt[dest][from] = min(dt[dest][from], lc[from] + rcvd.mincost[dest])
```
If any table entry tightens **and** a route minimum changes → broadcast new costs to neighbors.

### Key Design Decisions

- **`lc[]` as single source of truth** — the static link-cost array drives all neighbor validation, link cost lookups, and broadcast targeting. No switch statements or hardcoded IDs.
- **Selective broadcast** — a snapshot of minimums is taken before each update. Broadcasts only fire when a minimum actually decreases, reducing unnecessary traffic.
- **INF overflow guard** — `rcvdpkt->mincost[d] >= INF` is checked before adding a link cost to prevent integer overflow corrupting the table.
- **File-level encapsulation** — all variables are `static`, invisible outside their file, matching the real-world constraint that routers do not share state.

---

## Distance Table Layout

```
dt[dest][via] = cost to reach 'dest' using 'via' as the first hop
```

Example for Node 0 after convergence:
```
                via
   D0 |    1     2    3
  ----|------------------
     1|    1   999   999
dest 2|    2     3   999
     3|    4     5     7
```
The minimum across each row gives the shortest-path cost to that destination.
