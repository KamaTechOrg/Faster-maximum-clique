# Algorithms Directory

תיקייה זו מכילה אלגוריתמים למציאת קליקה מקסימלית בגרפים.

## מבנה

### algorithm5/
אלגוריתם 5 למציאת קליקה מקסימלית.

#### helper_functions/
פונקציות עזר לאלגוריתם 5:

- **top_k_by_degree.hpp** - פונקציה לבחירת K הקודקודים עם הדרגה הגבוהה ביותר בגרף

## שימוש

כל אלגוריתם נמצא בתיקייה נפרדת ומכיל:
- קבצי קוד מקור (.cpp/.hpp)
- פונקציות עזר
- תיעוד

## Compilation

הפרויקט משתמש ב-CMake. לבנייה:

```bash
cmake -B build -S .
cmake --build build --config Release
```

## Testing

הטסטים נמצאים בתיקיית `tests/` ומשתמשים ב-Google Test.

להרצת הטסטים:
```bash
.\build\tests\Release\test_algorithm5_helpers.exe
```

או דרך CTest:
```bash
cd build
ctest -C Release
```
