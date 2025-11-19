# LazyMC CLI - Maximum Clique Finder

## שימוש בסיסי

```bash
lazymc_cli.exe [options] <graph_file>
```

## אופציות

- `-h, --help` - הצגת הודעת עזרה
- `-v, --verbose` - פלט מפורט עם מידע על זמן ריצה
- `-t, --timeout <seconds>` - הגדרת timeout בשניות (ברירת מחדל: ללא הגבלה)

## דוגמאות שימוש

### הרצה בסיסית

```bash
lazymc_cli.exe graph.txt
```

### הרצה עם פלט מפורט

```bash
lazymc_cli.exe -v graph.txt
```

### הרצה עם timeout

```bash
lazymc_cli.exe -v -t 30 large_graph.txt
```

## פורמט קובץ גרף

כרגע, ה-CLI משתמש בגרף דוגמה מוגדר מראש (K4 - גרף מלא של 4 קודקודים).

**TODO:** הוספת תמיכה בקריאת גרף מקובץ בפורמטים:

- DIMACS
- Edge list
- Adjacency list

## פלט

הפלט כולל:

- גודל הקליקה המקסימלית שנמצאה
- רשימת הקודקודים בקליקה
- זמן ריצה (במצב verbose)

### דוגמת פלט

```
Graph loaded: 4 vertices
Maximum clique size: 4
Vertices: 0 1 2 3
Time: 0 ms
```

## אלגוריתם

ה-CLI משתמש באלגוריתם **Systematic Search** עם backtracking ו-pruning:

- חיפוש רקורסיבי בכל הקליקות האפשריות
- Pruning: עצירת חיפוש כאשר אי אפשר לשפר את הפתרון הנוכחי
- מורכבות: אקספוננציאלית במקרה הגרוע, אך עם אופטימיזציות

## מגבלות ידועות

1. **קריאת קובץ לא ממומשת** - כרגע משתמש בגרף דוגמה קבוע
2. **Timeout לא ממומש** - האופציה קיימת אך לא פעילה
3. **אין תמיכה בגרפים משוקללים** - רק גרפים פשוטים

## שיפורים עתידיים

- [ ] הוספת תמיכה בקריאת קבצי DIMACS
- [ ] מימוש timeout אמיתי
- [ ] הוספת אופציה לשמירת פלט לקובץ
- [ ] תמיכה במציאת כל הקליקות המקסימליות
- [ ] הוספת סטטיסטיקות מפורטות (מספר צמתים שנבדקו, pruning rate וכו')
- [ ] שילוב אלגוריתם LazyMC המלא במקום Systematic Search בלבד

## קומפילציה

```bash
cmake -B build
cmake --build build
```

ה-executable ימצא ב: `build/Debug/lazymc_cli.exe` (Windows) או `build/lazymc_cli` (Linux/Mac)

## טסטים

להרצת הטסטים:

```bash
.\build\tests\Debug\lazymc_tests.exe
```

כל 30 הטסטים צריכים לעבור בהצלחה.
