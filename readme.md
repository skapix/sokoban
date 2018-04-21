## Sokoban
Sokoban project contains a game and a solver. The solution, created by solver is neither push-optimal nor move-optimal.
Sover uses A\* algorithm, with a [hungarian algorithm](https://en.wikipedia.org/wiki/Hungarian_algorithm) heuristics.

### Dependencies
1. Qt5
2. Boost.Container (Header only)

### Supported compilers
1. GCC/Clang with c++17 support
2. Visual studio 2017

### How to Build
```
mkdir build
cd build
cmake ..
cmake --build .
```