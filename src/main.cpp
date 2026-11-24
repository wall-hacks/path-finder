// main.cpp: the pathfinder command-line tool.
//
//   pathfinder --map maps/maze.map --start 1,1 --goal 8,6 [--algo bfs] [--moves 4|8]
//
// exit codes: 0 path found, 1 bad input, 2 no path exists.
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "pathfinder/grid.hpp"
#include "pathfinder/search.hpp"

using pathfinder::Grid;
using pathfinder::Moves;
using pathfinder::Point;

namespace {

const char* const kUsage =
    "usage: pathfinder --map FILE --start X,Y --goal X,Y [--algo bfs] [--moves 4|8]\n"
    "\n"
    "  --map FILE    map in the moving ai .map format\n"
    "  --start X,Y   start cell (x is the column, y is the row, from 0 at the top left)\n"
    "  --goal X,Y    goal cell\n"
    "  --algo NAME   search algorithm: bfs (default)\n"
    "  --moves 4|8   4 = up/down/left/right (default), 8 = also diagonals\n";

struct Options {
    std::string map;
    Point start{};
    Point goal{};
    std::string algo = "bfs";
    Moves moves = Moves::Four;
};

// any problem with the command line becomes one of these, printed with the usage text
struct UsageError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// "3,4" -> {3, 4}
Point parse_point(const std::string& text, const std::string& flag) {
    auto comma = text.find(',');
    try {
        if (comma == std::string::npos) throw std::invalid_argument("no comma");
        std::size_t used_x = 0, used_y = 0;
        int x = std::stoi(text.substr(0, comma), &used_x);
        int y = std::stoi(text.substr(comma + 1), &used_y);
        if (used_x != comma || used_y != text.size() - comma - 1) throw std::invalid_argument("extra");
        return {x, y};
    } catch (const std::exception&) {
        throw UsageError(flag + " expects X,Y like 3,4 but got '" + text + "'");
    }
}

Options parse_args(int argc, char* argv[]) {
    Options opts;
    bool have_start = false, have_goal = false;

    for (int i = 1; i < argc; ++i) {
        std::string flag = argv[i];
        if (flag == "-h" || flag == "--help") {
            std::cout << kUsage;
            std::exit(0);
        }
        if (i + 1 >= argc) {
            throw UsageError(flag + " needs a value");
        }
        std::string value = argv[++i];

        if (flag == "--map") {
            opts.map = value;
        } else if (flag == "--start") {
            opts.start = parse_point(value, flag);
            have_start = true;
        } else if (flag == "--goal") {
            opts.goal = parse_point(value, flag);
            have_goal = true;
        } else if (flag == "--algo") {
            if (value != "bfs") throw UsageError("unknown --algo '" + value + "'; available: bfs");
            opts.algo = value;
        } else if (flag == "--moves") {
            if (value == "4") opts.moves = Moves::Four;
            else if (value == "8") opts.moves = Moves::Eight;
            else throw UsageError("--moves must be 4 or 8, got '" + value + "'");
        } else {
            throw UsageError("unknown option '" + flag + "'");
        }
    }

    if (opts.map.empty()) throw UsageError("--map is required");
    if (!have_start) throw UsageError("--start is required");
    if (!have_goal) throw UsageError("--goal is required");
    return opts;
}

// draw the map: # wall, . open, * path, S start, G goal
void print_map(const Grid& grid, const std::vector<Point>& path, Point start, Point goal) {
    std::vector<char> on_path(static_cast<std::size_t>(grid.width() * grid.height()), 0);
    for (Point p : path) on_path[static_cast<std::size_t>(grid.index(p))] = 1;

    for (int y = 0; y < grid.height(); ++y) {
        for (int x = 0; x < grid.width(); ++x) {
            Point p{x, y};
            char c = grid.passable(p) ? '.' : '#';
            if (on_path[static_cast<std::size_t>(grid.index(p))]) c = '*';
            if (p == start) c = 'S';
            if (p == goal) c = 'G';
            std::cout << c;
        }
        std::cout << '\n';
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        Options opts = parse_args(argc, argv);
        Grid grid = Grid::load(opts.map);
        auto result = pathfinder::bfs(grid, opts.start, opts.goal, opts.moves);

        print_map(grid, result.path, opts.start, opts.goal);
        std::cout << "\nalgorithm: " << opts.algo << " (" << (opts.moves == Moves::Four ? 4 : 8)
                  << "-way)\n";
        if (!result.found) {
            std::cout << "no path from start to goal\n"
                      << "explored:  " << result.explored << " cells\n";
            return 2;
        }
        std::cout << "length:    " << result.length << " moves\n"
                  << "explored:  " << result.explored << " cells\n";
        return 0;
    } catch (const UsageError& err) {
        std::cerr << "pathfinder: " << err.what() << "\n\n" << kUsage;
        return 1;
    } catch (const std::exception& err) {
        std::cerr << "pathfinder: " << err.what() << '\n';
        return 1;
    }
}