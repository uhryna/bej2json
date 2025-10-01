# bej2json

Мінімалістична утиліта та бібліотека на C для декодування **BEJ** (Binary Encoded JSON) у звичайний JSON за допомогою супутнього словника (`*.bin`).

## Можливості
- Підтримувані типи: **Set**, **String**, **Integer**
- Працює з BEJ-заголовком (`0xF0F1F000` / `0xF1F0F000`) у little-endian
- Простий pretty JSON writer (екранування, відступи)
- Тести на GoogleTest + приклади з документації, які збираються і запускаються через CTest
- Doxygen-документація (API + приклади)

## Збірка
Вимоги: CMake ≥ 3.16, gcc/clang, (опц.) Doxygen для доків.
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j
```

## Використання (CLI)
```bash
./build/bej2json -d <dict.bin> [-o out.json] <bej.bin>
# приклад:
./build/bej2json -d Memory_v1.bin myexample.bin -o out.json
```

## Тести
```bash
cd build
ctest -V
# або тільки приклади з документації:
ctest -V -R doxygen_examples
```

> Для інтеграційних тестів покладіть фікстури у `tests/fixtures/` (наприклад, `Memory_v1.bin`, `myexample.bin`).  
> Якщо потік містить непідтримані типи BEJ, можна ввімкнути “м’який” режим у декодері (`dc.strict = 0`) — тоді вони будуть пропущені.

## Документація
```bash
doxygen Doxyfile
# відкрийте docs/html/index.html
```

## Структура
```
include/   # заголовки (API)
source/    # реалізація
tests/     # GoogleTest + CTest
docs/      # Doxygen сторінки та приклади (docs/EXAMPLES)
```

## Ліцензія
MIT 
