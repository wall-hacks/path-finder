// grid_test.cpp: map parsing, bounds checks, and neighbor lists.
#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>

#include "pathfinder/grid.hpp"

using pathfinder::Grid;
using pathfinder::Moves;
using pathfinder::Point;

namespace {

Grid from_text(const std::string& text) {
    std::istringstream in(text);
    return Grid::parse(in);
}

const char* const kSmall =
    "type octile\n"
    "height 3\n"
    "width 4\n"
    "map\n"
    "....\n"
    ".@G.\n"
    "TSWO\n";

}  // namespace

TEST(GridParse, ReadsSizeAndCells) {
    Grid g = from_text(kSmall);
    EXPECT_EQ(g.width(), 4);
    EXPECT_EQ(g.height(), 3);
    EXPECT_TRUE(g.passable({0, 0}));
    EXPECT_FALSE(g.passable({1, 1}));  // @ is a wall
}

TEST(GridParse, MovingAiTerrainTypes) {
    Grid g = from_text(kSmall);
    EXPECT_TRUE(g.passable({2, 1}));   // G: ground
    EXPECT_TRUE(g.passable({1, 2}));   // S: swamp, walkable
    EXPECT_FALSE(g.passable({0, 2}));  // T: tree
    EXPECT_FALSE(g.passable({2, 2}));  // W: water
    EXPECT_FALSE(g.passable({3, 2}));  // O: out of bounds marker
}

TEST(GridParse, RejectsBadHeader) {
    EXPECT_THROW(from_text("height 3\nwidth 4\nmap\n"), std::runtime_error);
    EXPECT_THROW(from_text("type octile\nheight 0\nwidth 4\nmap\n"), std::runtime_error);
}

TEST(GridParse, RejectsWrongRowLength) {
    EXPECT_THROW(from_text("type octile\nheight 2\nwidth 3\nmap\n...\n..\n"), std::runtime_error);
}

TEST(GridParse, RejectsMissingRows) {
    EXPECT_THROW(from_text("type octile\nheight 3\nwidth 2\nmap\n..\n..\n"), std::runtime_error);
}

TEST(GridParse, RejectsUnknownTerrain) {
    EXPECT_THROW(from_text("type octile\nheight 1\nwidth 3\nmap\n.x.\n"), std::runtime_error);
}

TEST(GridLoad, ReadsExampleMapAndRejectsMissingFile) {
    Grid g = Grid::load(std::string(MAPS_DIR) + "/maze.map");
    EXPECT_EQ(g.width(), 12);
    EXPECT_EQ(g.height(), 9);
    EXPECT_THROW(Grid::load("no/such/file.map"), std::runtime_error);
}

TEST(GridBounds, OffTheMapIsNeverPassable) {
    Grid g = from_text(kSmall);
    EXPECT_TRUE(g.in_bounds({3, 2}));
    EXPECT_FALSE(g.in_bounds({4, 0}));
    EXPECT_FALSE(g.in_bounds({0, -1}));
    EXPECT_FALSE(g.passable({-1, 0}));
    EXPECT_FALSE(g.passable({0, 3}));
}

TEST(GridIndex, RoundTripsThroughFlatIndex) {
    Grid g = from_text(kSmall);
    EXPECT_EQ(g.index({3, 1}), 7);  // row 1 starts at 4
    EXPECT_EQ(g.point(7), (Point{3, 1}));
}

TEST(GridNeighbors, FourWayInOpenSpaceAndCorners) {
    Grid g(3, 3, std::vector<char>(9, 1));
    auto middle = g.neighbors({1, 1}, Moves::Four);
    ASSERT_EQ(middle.size(), 4u);
    EXPECT_EQ(middle[0], (Point{1, 0}));  // fixed order: up, right, down, left
    EXPECT_EQ(middle[1], (Point{2, 1}));
    EXPECT_EQ(middle[2], (Point{1, 2}));
    EXPECT_EQ(middle[3], (Point{0, 1}));
    EXPECT_EQ(g.neighbors({0, 0}, Moves::Four).size(), 2u);
}

TEST(GridNeighbors, EightWayInOpenSpace) {
    Grid g(3, 3, std::vector<char>(9, 1));
    EXPECT_EQ(g.neighbors({1, 1}, Moves::Eight).size(), 8u);
    EXPECT_EQ(g.neighbors({0, 0}, Moves::Eight).size(), 3u);
}

TEST(GridNeighbors, DiagonalsCannotCutPastBlockedCorners) {
    // .@     from (0,0), the diagonal to (1,1) would squeeze past the wall at (1,0),
    // ..     so the only move left is straight down to (0,1)
    Grid g(2, 2, {1, 0, 1, 1});
    auto next = g.neighbors({0, 0}, Moves::Eight);
    ASSERT_EQ(next.size(), 1u);
    EXPECT_EQ(next[0], (Point{0, 1}));
}