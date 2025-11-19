// systematic_search.cpp
#include "lazymc/search/systematic_search.hpp"

// NOTE: מימוש נאיבי/בסיסי של חיפוש סיסטמטי עם backtracking
// מטרה: לאפשר ל-CLI לרוץ ולמצוא קליקות מקסימליות
// TODO: להחליף במימוש LazyMC המלא עם כל האופטימיזציות:
//       - Lazy candidate maintenance (Algorithm 2)
//       - φ-density based pruning (Algorithm 8)
//       - K-core decomposition
//       - Advanced ordering strategies

namespace lazymc {
namespace search {

std::vector<int> SystematicSearch::find_maximum_clique(
    const std::vector<std::vector<int>>& adj) 
{
    if (adj.empty()) return {};
    
    std::vector<int> best;
    std::vector<int> current;
    std::vector<int> candidates;
    
    // התחלה עם כל הקודקודים כמועמדים
    for (int i = 0; i < static_cast<int>(adj.size()); ++i) {
        candidates.push_back(i);
    }
    
    backtrack(adj, current, candidates, best);
    return best;
}

std::vector<int> SystematicSearch::search_with_pruning(
    const std::vector<std::vector<int>>& adj,
    int lower_bound) 
{
    // TODO: Implement with pruning
    return find_maximum_clique(adj);
}

void SystematicSearch::backtrack(
    const std::vector<std::vector<int>>& adj,
    std::vector<int>& current,
    std::vector<int>& candidates,
    std::vector<int>& best) 
{
    // אם הקליקה הנוכחית גדולה יותר, עדכן את המיטב
    if (current.size() > best.size()) {
        best = current;
    }
    
    // Pruning: אם אפילו נוסיף את כל המועמדים לא נשפר את המיטב
    if (current.size() + candidates.size() <= best.size()) {
        return;
    }
    
    // נסה להוסיף כל מועמד
    for (size_t i = 0; i < candidates.size(); ++i) {
        int v = candidates[i];
        
        // הוסף את v לקליקה הנוכחית
        current.push_back(v);
        
        // צור רשימת מועמדים חדשה: רק שכנים של v שגם במועמדים הנוכחיים
        std::vector<int> new_candidates;
        for (size_t j = i + 1; j < candidates.size(); ++j) {
            int u = candidates[j];
            // בדוק אם u שכן של v
            bool is_neighbor = false;
            for (int neighbor : adj[v]) {
                if (neighbor == u) {
                    is_neighbor = true;
                    break;
                }
            }
            if (is_neighbor) {
                new_candidates.push_back(u);
            }
        }
        
        // המשך את החיפוש
        backtrack(adj, current, new_candidates, best);
        
        // הסר את v (backtrack)
        current.pop_back();
    }
}

} // namespace search
} // namespace lazymc
