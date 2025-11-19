# TODO: Future Optimizations for Intersection Operations

## 1. Bitset Intersection (High Priority)

**File:** `src/core/intersect.cpp`  
**Function:** `bitset_intersection()`

### Current Status

- ❌ Not implemented - currently falls back to `sorted_intersection`

### Proposed Implementation

```cpp
std::vector<int> Intersect::bitset_intersection(
    const std::vector<int>& a,
    const std::vector<int>& b,
    int max_vertex)
{
    std::vector<bool> bitset_a(max_vertex, false);
    std::vector<bool> bitset_b(max_vertex, false);

    for (int v : a) bitset_a[v] = true;
    for (int v : b) bitset_b[v] = true;

    std::vector<int> result;
    for (int v = 0; v < max_vertex; v++) {
        if (bitset_a[v] && bitset_b[v]) {
            result.push_back(v);
        }
    }
    return result;
}
```

### Performance Characteristics

- **Time Complexity:** O(max_vertex) - very fast for dense graphs
- **Space Complexity:** O(max_vertex)
- **Best Use Case:** Dense graphs with max_vertex < 10,000
- **Speedup:** Up to 10x faster than hash_intersection for dense graphs

### Advanced Version (Using std::bitset or AVX2)

For even better performance with fixed-size graphs:

```cpp
// Using std::bitset for compile-time size
template<size_t N>
std::vector<int> bitset_intersection_fixed(
    const std::vector<int>& a,
    const std::vector<int>& b)
{
    std::bitset<N> bits_a, bits_b;
    for (int v : a) bits_a.set(v);
    for (int v : b) bits_b.set(v);

    auto result_bits = bits_a & bits_b;

    std::vector<int> result;
    for (size_t i = 0; i < N; i++) {
        if (result_bits.test(i)) {
            result.push_back(static_cast<int>(i));
        }
    }
    return result;
}
```

---

## 2. Smart Intersection Selector

**Priority:** Medium

### Idea

Create a function that automatically chooses the best intersection algorithm based on input characteristics:

```cpp
enum class IntersectionStrategy {
    SORTED,    // Use sorted_intersection
    HASH,      // Use hash_intersection
    BITSET     // Use bitset_intersection
};

IntersectionStrategy choose_strategy(
    size_t size_a,
    size_t size_b,
    int max_vertex,
    bool is_sorted_a,
    bool is_sorted_b)
{
    // If both sorted, use sorted algorithm
    if (is_sorted_a && is_sorted_b) {
        return IntersectionStrategy::SORTED;
    }

    // For small max_vertex and dense graphs, use bitset
    if (max_vertex < 10000 &&
        size_a > max_vertex * 0.1 &&
        size_b > max_vertex * 0.1) {
        return IntersectionStrategy::BITSET;
    }

    // Default to hash
    return IntersectionStrategy::HASH;
}

std::vector<int> smart_intersection(
    const std::vector<int>& a,
    const std::vector<int>& b,
    int max_vertex = -1);
```

---

## 3. Parallel Intersection

**Priority:** Low (for very large graphs)

### Idea

For very large intersections, use parallel processing:

```cpp
#include <execution>

std::vector<int> parallel_intersection(
    const std::vector<int>& a,
    const std::vector<int>& b)
{
    std::unordered_set<int> set_b(b.begin(), b.end());
    std::vector<int> result;

    // Parallel filter
    std::copy_if(
        std::execution::par,
        a.begin(), a.end(),
        std::back_inserter(result),
        [&set_b](int v) { return set_b.contains(v); }
    );

    return result;
}
```

---

## 4. Cache-Friendly Intersection

**Priority:** Medium

### Idea

Optimize memory access patterns for better cache utilization:

```cpp
// Sort by expected access patterns
std::vector<int> cache_friendly_intersection(
    std::vector<int> a,  // copy to allow sorting
    const std::vector<int>& b)
{
    // Build hash set
    std::unordered_set<int> set_b(b.begin(), b.end());

    // Sort 'a' to improve cache hits
    std::sort(a.begin(), a.end());

    std::vector<int> result;
    result.reserve(std::min(a.size(), b.size()));

    for (int v : a) {
        if (set_b.contains(v)) {
            result.push_back(v);
        }
    }

    return result;
}
```

---

## 5. SIMD-Accelerated Intersection

**Priority:** Low (requires advanced expertise)

### Idea

Use AVX2/AVX-512 instructions for ultra-fast bitset operations:

```cpp
#ifdef __AVX2__
#include <immintrin.h>

std::vector<int> simd_intersection(
    const std::vector<int>& a,
    const std::vector<int>& b,
    int max_vertex)
{
    // Use 256-bit SIMD registers
    // Process 256 bits at a time
    // Requires alignment and padding
    // Can be 4-8x faster than scalar version
}
#endif
```

---

## Testing Requirements

For each optimization:

- [ ] Add performance benchmarks
- [ ] Add correctness tests
- [ ] Measure memory usage
- [ ] Document when to use each variant
- [ ] Profile on real-world graph datasets

---

## Benchmarking Plan

Create comprehensive benchmarks:

```cpp
// tests/benchmark_intersect.cpp
void benchmark_all_intersections() {
    // Test cases:
    // 1. Sparse graphs (density < 0.1)
    // 2. Dense graphs (density > 0.5)
    // 3. Small graphs (n < 100)
    // 4. Large graphs (n > 10000)
    // 5. Various intersection sizes
}
```

---

## References

- [Fast Set Intersection in C++](https://arxiv.org/abs/1401.6399)
- [SIMD Compression and Intersection](https://github.com/lemire/SIMDCompressionAndIntersection)
- Bitset operations optimization techniques
