# Deadlock-Sentinel: Multi-Threaded Resource Allocator

A C++17 implementation of the Banker's Algorithm with multi-threading, visualization, and recovery features.

## Features

### 1. Safety Algorithm (Core Logic)
- `ResourceManager` class managing Available, Max, Allocation, and Need matrices
- `isSafeState()` - checks if current state is safe (no deadlock possible)
- `request()` and `release()` for resource management

### 2. Multi-Threaded Simulation
- 5 concurrent threads simulating processes
- Thread-safe operations using `std::mutex`
- `std::condition_variable` for process waiting when resources unavailable

### 3. Resource Allocation Graph (RAG) Visualization
- `dumpState()` - text-based graph showing resource allocation
- `generateDotFile()` - generates Graphviz .dot file for visualization

### 4. Force Reclaim Mode
- `forceReclaim(processID)` - terminates process and reclaims resources
- Useful for recovering from starvation scenarios

## Build & Run

```bash
make
./safestate
```

## Visualize RAG

```bash
# After running, generate PNG from .dot file
dot -Tpng allocation_graph.dot -o graph.png
```

## Example Output

```
=== SafeState: Multi-Threaded Simulation ===
[P0] Requesting: 1 2 0 
[P0] Request granted
Safe Sequence: P1 -> P3 -> P4 -> P0 -> P2
...
=== Resource Allocation Graph ===
P0 holds: R1(2) | needs: R0(7) R1(3) R2(3)
...
```