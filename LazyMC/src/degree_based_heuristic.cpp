#include "lazymc/degree_based_heuristic.h"
#include <algorithm>
#include <unordered_set>

namespace lazymc {

// Global incumbent clique (shared across the algorithm)
// In a real implementation, this should be thread-safe
extern std::vector<int32_t> global_incumbent_clique;

// Degree-based heuristic search (Algorithm 5 from paper)
// Returns the best clique found starting from top-K highest degree vertices
std::vector<int32_t> degree_based_heuristic(
    const std::vector<std::vector<int32_t>>& G, 
    int top_k) {
    
    if (G.empty()) return {};
    
    int n = static_cast<int>(G.size());
    std::vector<int32_t> best_clique;
    
    // Step 1: Identify top-K vertices with highest degree
    std::vector<std::pair<int32_t, int32_t>> degree_vertex;
    for (int32_t v = 0; v < n; ++v) {
        degree_vertex.push_back({static_cast<int32_t>(G[v].size()), v});
    }
    
    // Sort by degree (descending)
    std::sort(degree_vertex.begin(), degree_vertex.end(), 
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    // Limit to top_k vertices
    int k = std::min(top_k, n);
    
    // Step 2: For each of top-K vertices, try to build a clique
    for (int i = 0; i < k; ++i) {
        int32_t v = degree_vertex[i].second;
        
        // Step 3: N ← {u ∈ N_G(v) : |N_G(u)| ≥ |C*|}
        std::vector<int32_t> N;
        for (int32_t u : G[v]) {
            if (static_cast<int32_t>(G[u].size()) >= static_cast<int32_t>(best_clique.size())) {
                N.push_back(u);
            }
        }
        
        // Step 4: C = {v}
        std::vector<int32_t> C = {v};
        
        // Step 5-10: Greedy clique expansion
        while (!N.empty()) {
            // Step 7: Find u with maximum |N ∩ N_G(u)|
            int32_t best_u = -1;
            int32_t max_intersection = -1;
            
            for (int32_t w : N) {
                // Compute |N ∩ N_G(w)| using hash set for efficiency
                std::unordered_set<int32_t> neighbors_w(G[w].begin(), G[w].end());
                int32_t intersection_size = 0;
                
                for (int32_t x : N) {
                    if (neighbors_w.count(x) > 0) {
                        ++intersection_size;
                    }
                }
                
                if (intersection_size > max_intersection) {
                    max_intersection = intersection_size;
                    best_u = w;
                }
            }
            
            if (best_u == -1) break;
            
            // Step 8: C ← C ∪ {u}
            C.push_back(best_u);
            
            // Step 9: N ← N ∩ N_G(u)
            std::unordered_set<int32_t> neighbors_u(G[best_u].begin(), G[best_u].end());
            std::vector<int32_t> new_N;
            for (int32_t x : N) {
                if (x != best_u && neighbors_u.count(x) > 0) {
                    new_N.push_back(x);
                }
            }
            N = std::move(new_N);
        }
        
        // Step 11-12: Update best clique if better
        if (C.size() > best_clique.size()) {
            best_clique = std::move(C);
        }
    }
    
    return best_clique;
}

} // namespace lazymc
