# Faster-maximum-clique

Implementation of LazyMC and other maximum clique algorithms.

## Project Status

### ✅ Completed

- Full LazyMC project structure (include/src/apps/tests)
- Core modules: Graph, Intersect, Candidate Operations, K-Core, Ordering
- Heuristics: Degree-based, Coreness-based
- Comprehensive test suite (30 tests, all passing)
- Working CLI application
- CMake build system with Google Test integration

### 🚧 In Progress

- **Systematic Search**: Currently implemented with **naive backtracking** to allow CLI to run
  - Basic recursive search with pruning
  - **TODO**: Replace with full LazyMC algorithm (lazy candidate maintenance, φ-density pruning, advanced ordering)
- Intersection optimizations (hash-based implemented, bitset planned)
- Graph file loading (DIMACS format)

### 📋 Planned

- Full LazyMC algorithm implementation (Algorithms 2, 8 from paper)
- Bitset intersection for dense graphs
- Smart intersection selector
- Parallel processing optimizations
- SIMD acceleration

## Building

```bash
cmake -B build
cmake --build build
```

## Running Tests

```bash
.\build\tests\Debug\lazymc_tests.exe    # Windows
./build/tests/lazymc_tests              # Linux/Mac
```

## Using the CLI

```bash
.\build\Debug\lazymc_cli.exe -v graph.txt
```

See [CLI_README.md](CLI_README.md) for detailed usage.

## Documentation

- [CLI_README.md](CLI_README.md) - CLI usage and options
- [README_LazyMC.md](README_LazyMC.md) - LazyMC architecture documentation
- [TODO_INTERSECT_OPTIMIZATIONS.md](TODO_INTERSECT_OPTIMIZATIONS.md) - Future intersection optimizations

## Project Structure

```
├── include/lazymc/          # Header files
│   ├── graph/              # Graph representations
│   ├── core/               # Core algorithms
│   ├── heuristics/         # Upper bound heuristics
│   └── search/             # Search algorithms
├── src/                    # Implementation files
├── apps/                   # CLI application
├── tests/                  # Google Test suite
└── cmake/                  # CMake modules
```

## Contributing

This is a research project implementing the LazyMC maximum clique algorithm with optimizations.

## Notes

⚠️ **Important**: Some functions like `SystematicSearch` are currently implemented naively to allow the CLI to run. These will be replaced with optimized LazyMC implementations.
