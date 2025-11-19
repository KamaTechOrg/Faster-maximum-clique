// intersect.hpp
#pragma once

#include <vector>
#include <algorithm>
#include <unordered_set>

namespace lazymc {
namespace core {

/// \brief פעולות חיתוך על קבוצות קודקודים
///
/// בחירת אלגוריתם החיתוך המתאים:
/// - sorted_intersection: כאשר שתי הרשימות ממויינות (O(n+m))
/// - hash_intersection: למקרה הכללי, טוב לרוב המקרים (O(n+m))
/// - bitset_intersection: לגרפים צפופים עם max_vertex קטן (O(max_vertex/64))
///
/// TODO: בעתיד - הוסף פונקציה חכמה שבוחרת אוטומטית את האלגוריתם הטוב ביותר
class Intersect {
public:
    /// \brief חיתוך בין שתי רשימות ממויינות
    /// \note דורש שהקלט יהיה ממויין!
    /// \complexity O(n + m) זמן, O(k) מקום (k = גודל תוצאה)
    static std::vector<int> sorted_intersection(
        const std::vector<int>& a, 
        const std::vector<int>& b);
    
    /// \brief חיתוך מהיר עם bitset (לגרפים קטנים צפופים)
    /// \param max_vertex הקודקוד המקסימלי בגרף (לגודל ה-bitset)
    /// \note TODO: טרם מומש! כרגע משתמש ב-sorted_intersection
    /// \complexity O(max_vertex / 64) זמן לגרפים צפופים
    /// \use_case מומלץ כאשר max_vertex < 10000 והגרף צפוף
    static std::vector<int> bitset_intersection(
        const std::vector<int>& a, 
        const std::vector<int>& b,
        int max_vertex);
    
    /// \brief חיתוך כללי (לא דורש מיון) - O(n+m) עם hash set
    /// מחזיר את כל האלמנטים ב-a שגם נמצאים ב-b (בסדר של a)
    /// \complexity O(n + m) זמן ממוצע, O(min(n,m)) מקום
    /// \note זו הפונקציה המומלצת למקרה הכללי
    static std::vector<int> hash_intersection(
        const std::vector<int>& a,
        const std::vector<int>& b);
};

} // namespace core
} // namespace lazymc
