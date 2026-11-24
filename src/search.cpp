// search.cpp: breadth-first search with path reconstruction.
#include "pathfinder/search.hpp"

#include <algorithm>
#include <queue>
#include <stdexcept>
#include <string>

namespace pathfinder {

namespace {

void check_endpoint(const Grid& grid, Point p, const char* name) {
    std::string where = "(" + std::to_string(p.x) + ", " + std::to_string(p.y) + ")";
    if (!grid.in_bounds(p)) {
        throw std::invalid_argument(std::string(name) + " " + where + " is off the map");
    }
    if (!grid.passable(p)) {
        throw std::invalid_argument(std::string(name) + " " + where + " is a blocked cell");
    }
}

}  // namespace

SearchResult bfs(const Grid& grid, Point start, Point goal, Moves moves) {
    check_endpoint(grid, start, "start");
    check_endpoint(grid, goal, "goal");

    const int cells = grid.width() * grid.height();
    // parent[i] is the cell we came from to reach cell i; -1 means not reached yet
    std::vector<int> parent(static_cast<std::size_t>(cells), -1);
    std::vector<char> seen(static_cast<std::size_t>(cells), 0);

    // bfs visits cells in order of distance, so the first time we reach the goal is the shortest way
    std::queue<Point> frontier;
    frontier.push(start);
    seen[static_cast<std::size_t>(grid.index(start))] = 1;

    SearchResult result;
    while (!frontier.empty()) {
        Point current = frontier.front();
        frontier.pop();
        ++result.explored;

        if (current == goal) {
            result.found = true;
            break;
        }

        for (Point next : grid.neighbors(current, moves)) {
            auto i = static_cast<std::size_t>(grid.index(next));
            if (!seen[i]) {
                seen[i] = 1;
                parent[i] = grid.index(current);
                frontier.push(next);
            }
        }
    }

    if (!result.found) {
        return result;
    }

    // walk back from the goal through the parents, then flip it to go start -> goal
    for (int i = grid.index(goal); i != grid.index(start); i = parent[static_cast<std::size_t>(i)]) {
        result.path.push_back(grid.point(i));
    }
    result.path.push_back(start);
    std::reverse(result.path.begin(), result.path.end());
    result.length = static_cast<int>(result.path.size()) - 1;
    return result;
}

}  // namespace pathfinder