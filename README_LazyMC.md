# LazyMC - Lazy Maximum Clique Algorithm

**LazyMC** הוא אלגוריתם למציאת קליקה מקסימלית בגרפים עם אופטימיזציות "עצלות" (lazy evaluation).

## 🏗️ מבנה הפרויקט

```
LazyMC/
├── include/lazymc/          # קבצי Header
│   ├── graph/              # ייצוגי גרף
│   ├── core/               # אלגוריתמים בסיסיים
│   ├── heuristics/         # היוריסטיקות
│   ├── search/             # אלגוריתמי חיפוש
│   └── lazymc.hpp          # Header ראשי
├── src/                    # קבצי מימוש
├── apps/                   # אפליקציות CLI
├── tests/                  # טסטים
└── CMakeLists.txt          # בנייה
```

## 🚀 בנייה

### דרישות

- CMake 3.14+
- C++17 compiler
- Git

### הוראות בנייה

```bash
# Configuration
cmake -B build -S .

# בנייה
cmake --build build

# הרצת טסטים
.\build\tests\Debug\lazymc_tests.exe
```

## 📚 מודולים

### Graph

- **Graph**: ייצוג גרף בסיסי עם רשימת שכנויות
- **LazyGraph**: גרסה עם אופטימיזציות עצלות
- **Kernelisation**: הפחתת גודל גרף

### Core

- **CandidateOps**: פעולות על מועמדים לקליקה
- **Intersect**: חיתוך קבוצות קודקודים
- **KCore**: k-core decomposition
- **Ordering**: אסטרטגיות מיון קודקודים

### Heuristics

- **DegreeHeuristic**: בחירה לפי דרגה
- **CorenessHeuristic**: בחירה לפי coreness

### Search

- **SystematicSearch**: חיפוש שיטתי (branch & bound)
- **NeighborSearch**: חיפוש מבוסס שכנים
- **MaxCliqueSearch**: ממשק ראשי

## 🎯 שימוש

### כספרייה

```cpp
#include "lazymc/lazymc.hpp"

std::vector<std::vector<int>> adj = {
    {1, 2, 3},
    {0, 2, 3},
    {0, 1, 3},
    {0, 1, 2}
};

auto max_clique = lazymc::find_maximum_clique(adj);
```

### CLI

```bash
# הצגת עזרה
.\build\Debug\lazymc_cli.exe --help

# הרצה עם גרף
.\build\Debug\lazymc_cli.exe graph.txt
```

## ✅ טסטים

הפרויקט כולל 14 טסטים - כולם עוברים בהצלחה!

---

**Status**: ✅ Build passing | ✅ All tests passing (14/14)
