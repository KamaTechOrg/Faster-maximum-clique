// degree_heuristic.hpp
#pragma once

#include <vector>
#include <numeric>     // std::iota
#include <algorithm>   // std::partial_sort

namespace lazymc {
namespace heuristics {

/// \brief מחזיר את K הקודקודים בעלי הדרגה הגבוהה ביותר בגרף.
/// \param adj  רשימת שכנויות: adj[v] = רשימת שכנים של v
/// \param K    מספר הקודקודים שרוצים לקבל
/// \return וקטור של אינדקסי קודקודים (0..n-1),
///         ממויינים מהדרגה הגבוהה לנמוכה.
inline std::vector<int> select_top_k_by_degree(
    const std::vector<std::vector<int>>& adj,
    int K
) {
    const int n = static_cast<int>(adj.size());
    if (n == 0 || K <= 0) {
        return {};
    }

    if (K > n) {
        K = n;
    }

    // רשימת כל הקודקודים: 0..n-1
    std::vector<int> vertices(n);
    std::iota(vertices.begin(), vertices.end(), 0);

    // partial_sort: ממיין רק את K הראשונים לפי דרגה (גבוה->נמוך).
    //
    // אחרי הקריאה:
    // - vertices[0..K-1] הם ה-K בעלי הדרגה הגבוהה ביותר, וביניהם יש סדר מלא.
    // - שאר האלמנטים יכולים להיות בסדר שרירותי (לא אכפת לנו).
    std::partial_sort(
        vertices.begin(),
        vertices.begin() + K,
        vertices.end(),
        [&](int a, int b) {
            const auto da = adj[a].size();
            const auto db = adj[b].size();
            if (da != db)
                return da > db;  // דרגה גבוהה קודם
            return a < b;        // שבירת שוויון לפי אינדקס
        }
    );

    vertices.resize(K);
    return vertices;
}

} // namespace heuristics
} // namespace lazymc
