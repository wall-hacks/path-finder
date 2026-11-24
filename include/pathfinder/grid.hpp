// grid.hpp: a 2d map of open and blocked cells.
#pragma once

#include <iosfwd>
#include <string>
#include <vector>

namespace pathfinder {

    // a cell position: x is the column, y is the row, both counted from 0 at the top left
    struct Point {
        int x = 0;
        int y = 0;
        bool operator==(const Point& other) const { return x == other.x && y == other.y; }
        bool operator!=(const Point& other) const { return !(*this == other); }
    };

    // which moves are allowed from a cell
    enum class Moves {
        Four,   // up, right, down, left
        Eight,  // the four above plus diagonals
    };

    class Grid {
    public:
        // open[i] is true when cell i can be walked on; cells are stored row by row
        Grid(int width, int height, std::vector<char> open);

        // read a map in the moving ai .map format; throws std::runtime_error on bad input
        static Grid parse(std::istream& in);
        static Grid load(const std::string& path);

        int width() const { return width_; }
        int height() const { return height_; }

        bool in_bounds(Point p) const;
        bool passable(Point p) const;  // false for blocked cells and anything off the map

        // position of a cell in the one flat vector
        int index(Point p) const { return p.y * width_ + p.x; }
        Point point(int index) const { return {index % width_, index / width_}; }

        // cells you can step to from p. diagonal moves can't cut past a blocked corner.
        std::vector<Point> neighbors(Point p, Moves moves) const;

    private:
        int width_;
        int height_;
        std::vector<char> open_;  // one contiguous block, row by row (char, not bool, so it's a plain array)
    };

}  // namespace pathfinder