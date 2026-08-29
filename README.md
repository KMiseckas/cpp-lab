# cpp-lab
The goal of the repo is to keep fresh, learn, and use as memory notes on common C++ topics, as well as to explore implementation details, trade-offs, correctness, and performance characteristics.

A small C++ learning repository for implementing core data structures, algorithms, and concurrency primitives from scratch. All **implemented by hand**, would defeat the point in using agents when it comes to learning and understanding these concepts.

(AI is used for Q&A, quick explanations, and other questions, it is an amazing tool when it comes to speeding up learning itself, as long as time is taken to implement the concepts without making AI give the answers for you.)

Note: These implementations are educational mostly.

## Current topics

| Topic | Coverage |
| --- | --- |
| Custom vector | Implementation |
| Hash maps | Separate chaining; open addressing |
| Directed graphs | BFS; DFS; path and cycle detection |
| Binary heap | Implementation |
| Blocking queue | Implementation |
| Thread pool | Implementation |
| LRU cache | Thread-safe and sharded variants |
| Ring buffers | SPSC lock-free ring buffer |
| Shared pointer | Implementation |
| Unit tests | Coverage for the main components |
| Performance benchmarks | Basic benchmarks |

## Build

The project uses CMake.

```bash
cmake --preset default
cmake --build --preset default
```

Presets may vary depending on the local toolchain.

## Tests

Tests are kept alongside the implementations under `tests/` and cover the main containers, algorithms, and concurrency components. Unit tests are mostly written with use of AI agents for speed.
