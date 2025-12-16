#include <vector>
#include <cstdint>
#include <unordered_set>


namespace lazymc {

// INTERSECT-GT(A, B, C, θ)
// A: array (vector)
// B: hash table (unordered_set)
// C: output array, intersection elements will be written at the front
// θ: threshold
// Caller must check (return_value > θ).
int32_t intersect_gt(const std::vector<int32_t>& A,
                     const std::unordered_set<int32_t>& B,
                     std::vector<int32_t>& C,
                     int32_t theta)
{
    // n = |A|; m = |B|
    const int32_t n = static_cast<int32_t>(A.size());
    const int32_t m = static_cast<int32_t>(B.size());

    // if n < θ or m < θ then return -1
    if (n < theta || m < theta) {
        return -1;
    }

    // h ← n − θ  (number of missing elements tolerated)
    int32_t h = n - theta;

    // C must be able to hold up to n elements of A
    const std::size_t ns = static_cast<std::size_t>(n);
    if (C.size() < ns) C.resize(ns);


    // for a ← 0 to n − 1 do
    for (int32_t a = 0; a < n; ++a) {
        int32_t val = A[static_cast<std::size_t>(a)];

        // if !B.contains(A[a]) then
        if (B.find(val) == B.end()) {
            // h ← h − 1
            --h;
            // if h ≤ 0 then return −1  (exit early)
            if (h <= 0) {
                return -1;
            }
        } else {
            // C[a + h − (n − θ)] ← A[a]
            int32_t idx = a + h - (n - theta);
            C[static_cast<std::size_t>(idx)] = val;
        }
    }

    // return h + θ   (this is exactly the intersection size)
    return h + theta;
}

// INTERSECT-SIZE-GT-BOOL(A, B, θ)
// Returns true if |A ∩ B| > theta, false otherwise.
// A is provided as an array (vector) of length n,
// B is provided as a hash table (unordered_set) of size m.
bool intersect_size_gt_bool(const std::vector<int32_t>& A,
                            const std::unordered_set<int32_t>& B,
                            int32_t theta)
{
    //n = |A|; m = |B|
    const int32_t n = static_cast<int32_t>(A.size());
    const int32_t m = static_cast<int32_t>(B.size());

    // if n ≤ θ or m ≤ θ then return false
    if (n <= theta || m <= theta) {
        return false;
    }

    // h ← n − θ  (number of missing elements tolerated)
    int32_t h = n - theta;

    // for a ← 0 to n − 1 do
    for (int32_t a = 0; a < n; ++a) {
        int32_t val = A[static_cast<std::size_t>(a)];

        // if !B.contains(A[a]) then
        if (B.find(val) == B.end()) {
            // h ← h − 1
            --h;
            // if h ≤ 0 then return false  (exit early)
            if (h <= 0) {
                return false;
            }
        }
        // else if h > n − a − 1 then return true  (exit early)
        else if (h > n - a - 1) {
            return true;
        }
    }

    // Line 10: return h > 0
    return h > 0;
}

}