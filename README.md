[![CI](https://github.com/wall-hacks/path-finder/actions/workflows/ci.yml/badge.svg)](https://github.com/wall-hacks/path-finder/actions/workflows/ci.yml)

# path-finder

A C++ command-line tool that finds the shortest route across a 2D grid map.

It reads maps in the [Moving AI](https://movingai.com/benchmarks/formats.html) `.map` format, the standard format for grid path-finding benchmarks, and finds the shortest path with breadth-first search (BFS), moving in 4 or 8 directions.

```
$ ./build/pathfinder --map maps/maze.map --start 1,1 --goal 10,7
############
#S***#.....#
#.##*#.###.#
#.#**#...#.#
#.#*####.#.#
#.#******#.#
#.######*#.#
#......#**G#
############

algorithm: bfs (4-way)
length:    17 moves
explored:  33 cells
```

`#` is a wall, `.` is open ground, `*` is the path, and `S` and `G` are the start and goal. "Explored" counts the cells BFS took off its queue before reaching the goal.

## Build and test

Requires a C++17 compiler and CMake 3.16+. GoogleTest is downloaded automatically when you configure.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The build uses `-Wall -Wextra -Wpedantic -Werror`, so any compiler warning fails it. Release is the default; add `-DCMAKE_BUILD_TYPE=Debug` for a debug build.

## Usage

```
pathfinder --map FILE --start X,Y --goal X,Y [--algo bfs] [--moves 4|8]
```

| Option | Meaning |
|---|---|
| `--map FILE` | Map in the Moving AI `.map` format |
| `--start X,Y` | Start cell. `x` is the column and `y` the row, counted from 0 at the top left |
| `--goal X,Y` | Goal cell |
| `--algo NAME` | Search algorithm. `bfs` for now |
| `--moves 4\|8` | `4` moves up, down, left, and right (default); `8` adds diagonals |

Exit codes: `0` path found, `1` bad input (with a message saying what's wrong), `2` no path exists.

Diagonal moves can't cut past a wall: stepping diagonally needs both side cells open, so the path never squeezes between two blocked corners.

## Maps

| File | What it tests |
|---|---|
| `maps/maze.map` | A 12 × 9 maze with one route through |
| `maps/open.map` | An 8 × 5 grid with no walls, where the answer is easy to check by hand |
| `maps/walled.map` | A full wall down the middle, so the goal is unreachable |
| `maps/corners.map` | A start cell boxed in by two diagonal walls |

A `.map` file has a four-line header, then one row of characters per line:

```
type octile
height 5
width 7
map
...@...
```

`.`, `G`, and `S` are walkable. `@`, `O`, `T`, and `W` are blocked. Larger benchmark maps from [movingai.com](https://movingai.com/benchmarks/grids.html) work as-is.

## How it works

- **`Grid`** (`include/pathfinder/grid.hpp`) stores the map in one contiguous `std::vector`, row by row, and converts between `(x, y)` and a flat index. `neighbors()` returns the legal moves from a cell in a fixed order, so results are repeatable.
- **`bfs()`** (`include/pathfinder/search.hpp`) explores cells in order of distance from the start using a queue, so the first time it reaches the goal is along a shortest path. Each cell records the cell it was reached from, and the path is rebuilt by walking those back from the goal.
- **Tests** (`tests/`) use GoogleTest and cover map parsing and bad input, bounds checks, neighbor lists including corner cutting, and BFS on an open grid, a maze, an unreachable goal, and start equal to goal.

## Project structure

```
include/pathfinder/   public headers: grid.hpp, search.hpp
src/                  grid.cpp, search.cpp, main.cpp (the command-line tool)
tests/                GoogleTest suites, run with ctest
maps/                 example maps in the Moving AI format
.github/workflows/    CI: GCC and Clang, Debug and Release
```

## Roadmap

- [x] **v0.1.0:** grid maps, BFS, command-line tool, tests, and CI
- [ ] **v0.2.0:** Dijkstra and A*, with a comparison of cells explored on the same map
- [ ] **v0.3.0:** benchmarks on Moving AI maps, sanitizer builds, and docs

Work is tracked in Jira Scrum sprints.

## License

MIT. See `LICENSE`.