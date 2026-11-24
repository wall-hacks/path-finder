// search.hpp: shortest path search on a grid.
#pragma once

#include <vector>

#include "pathfinder/grid.hpp"

namespace pathfinder {

    struct SearchResult {
        bool found = false;
        std::vector<Point> path;  // start to goal, both included; empty when not found
        int length = 0;           // number of moves, path.size() - 1
        int explored = 0;         // cells taken off the queue and expanded
    };

    // breadth-first search: shortest path in number of moves.
    // throws std::invalid_argument if start or goal is off the map or blocked.
    SearchResult bfs(const Grid& grid, Point start, Point goal, Moves moves = Moves::Four);

}  // namespace pathfinder