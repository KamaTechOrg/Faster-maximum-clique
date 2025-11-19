// lazymc.cpp
#include "lazymc/lazymc.hpp"
#include "lazymc/search/max_clique_search.hpp"

namespace lazymc {

std::vector<int> find_maximum_clique(const std::vector<std::vector<int>>& adj) {
    // קריאה לאלגוריתם החיפוש הסיסטמטי
    return search::MaxCliqueSearch::find_max_clique(adj);
}

// ניתן להוסיף פונקציות עזר גלובליות או initialization code

} // namespace lazymc
