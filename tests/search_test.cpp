// search_test.cpp: bfs on an open grid, a maze, an unreachable goal, and start == goal.
#include <gtest/gtest.h>

#include <cstdlib>
#include <stdexcept>
#include <string>

#include "pathfinder/grid.hpp"
#include "pathfinder/search.hpp"

using pathfinder::bfs;
using pathfinder::Grid;
using pathfinder::Moves;
using pathfinder::Point;
using pathfinder::SearchResult;

namespace {

Grid map(const std::string& name) { return Grid::load(std::string(MAPS_DIR) + "/" + name); }

// a path is valid if it runs start -> goal over open cells, one legal step at a time
void expect_valid_path(const Grid& g, const SearchResult& r, Point start, Point goal, Moves moves) {
    ASSERT_TRUE(r.found);
    ASSERT_FALSE(r.path.empty());
    EXPECT_EQ(r.path.front(), start);
    EXPECT_EQ(r.path.back(), goal);
    EXPECT_EQ(r.length, static_cast<int>(r.path.size()) - 1);
    for (std::size_t i = 1; i < r.path.size(); ++i) {
        auto next = g.neighbors(r.path[i - 1], moves);
        bool step_ok = false;
        for (Point p : next) step_ok = step_ok || p == r.path[i];
        EXPECT_TRUE(step_ok) << "illegal step at path index " << i;
    }
}

}  // namespace

TEST(Bfs, OpenGridFourWayIsManhattanDistance) {
    Grid g = map("open.map");  // 8 x 5, no walls
    auto r = bfs(g, {0, 0}, {7, 4});
    expect_valid_path(g, r, {0, 0}, {7, 4}, Moves::Four);
    EXPECT_EQ(r.length, 7 + 4);
}

TEST(Bfs, OpenGridEightWayUsesDiagonals) {
    Grid g = map("open.map");
    auto r = bfs(g, {0, 0}, {7, 4}, Moves::Eight);
    expect_valid_path(g, r, {0, 0}, {7, 4}, Moves::Eight);
    EXPECT_EQ(r.length, 7);  // max(7, 4) when diagonal steps count as one move
}

TEST(Bfs, SolvesTheMaze) {
    Grid g = map("maze.map");
    auto r = bfs(g, {1, 1}, {10, 7});
    expect_valid_path(g, r, {1, 1}, {10, 7}, Moves::Four);
    EXPECT_EQ(r.length, 17);
    EXPECT_GT(r.explored, r.length);  // bfs looks at more cells than the path uses
}

TEST(Bfs, UnreachableGoalIsNotFound) {
    Grid g = map("walled.map");  // a full wall down column 3
    auto r = bfs(g, {0, 0}, {6, 0});
    EXPECT_FALSE(r.found);
    EXPECT_TRUE(r.path.empty());
    EXPECT_EQ(r.explored, 15);  // every cell left of the wall, 3 x 5
}

TEST(Bfs, DiagonalCannotSlipThroughCorners) {
    Grid g = map("corners.map");  // (0,0) is boxed in by walls at (1,0) and (0,1)
    EXPECT_FALSE(bfs(g, {0, 0}, {2, 2}, Moves::Eight).found);
}

TEST(Bfs, StartEqualsGoal) {
    Grid g = map("open.map");
    auto r = bfs(g, {3, 2}, {3, 2});
    EXPECT_TRUE(r.found);
    EXPECT_EQ(r.length, 0);
    ASSERT_EQ(r.path.size(), 1u);
    EXPECT_EQ(r.path[0], (Point{3, 2}));
    EXPECT_EQ(r.explored, 1);
}

TEST(Bfs, RejectsBlockedOrOffMapEndpoints) {
    Grid g = map("maze.map");
    EXPECT_THROW(bfs(g, {0, 0}, {10, 7}), std::invalid_argument);   // start is a wall
    EXPECT_THROW(bfs(g, {1, 1}, {50, 50}), std::invalid_argument);  // goal is off the map
}