// grid.cpp: loading moving ai maps and finding neighbors.
#include "pathfinder/grid.hpp"

#include <fstream>
#include <stdexcept>
#include <utility>

namespace pathfinder {

Grid::Grid(int width, int height, std::vector<char> open)
    : width_(width), height_(height), open_(std::move(open)) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("grid width and height must be positive");
    }
    if (open_.size() != static_cast<std::size_t>(width) * static_cast<std::size_t>(height)) {
        throw std::invalid_argument("grid needs exactly width * height cells");
    }
}

namespace {

// moving ai terrain: '.' 'G' 'S' are walkable; '@' 'O' 'T' 'W' are not
bool terrain_is_open(char c, bool& known) {
    known = true;
    switch (c) {
        case '.': case 'G': case 'S': return true;
        case '@': case 'O': case 'T': case 'W': return false;
        default: known = false; return false;
    }
}

// read "<word> <number>" from a header line, e.g. "height 8"
int read_header_number(std::istream& in, const std::string& word) {
    std::string got;
    int value = 0;
    if (!(in >> got >> value) || got != word) {
        throw std::runtime_error("expected '" + word + " <number>' in the map header");
    }
    if (value <= 0) {
        throw std::runtime_error(word + " must be positive, got " + std::to_string(value));
    }
    return value;
}

}  // namespace

Grid Grid::parse(std::istream& in) {
    // header: type <anything>, height <h>, width <w>, map
    std::string word, type;
    if (!(in >> word >> type) || word != "type") {
        throw std::runtime_error("expected 'type <name>' on the first line");
    }
    int height = read_header_number(in, "height");
    int width = read_header_number(in, "width");
    if (!(in >> word) || word != "map") {
        throw std::runtime_error("expected 'map' before the map rows");
    }

    // then exactly height rows of width characters
    std::vector<char> open;
    open.reserve(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
    for (int y = 0; y < height; ++y) {
        std::string row;
        if (!(in >> row)) {
            throw std::runtime_error("map ended after " + std::to_string(y) + " of " +
                                     std::to_string(height) + " rows");
        }
        if (static_cast<int>(row.size()) != width) {
            throw std::runtime_error("row " + std::to_string(y) + " has " +
                                     std::to_string(row.size()) + " cells, expected " +
                                     std::to_string(width));
        }
        for (int x = 0; x < width; ++x) {
            bool known = false;
            bool is_open = terrain_is_open(row[x], known);
            if (!known) {
                throw std::runtime_error("unknown terrain '" + std::string(1, row[x]) +
                                         "' at x=" + std::to_string(x) + ", y=" + std::to_string(y));
            }
            open.push_back(is_open ? 1 : 0);
        }
    }
    return Grid(width, height, std::move(open));
}

Grid Grid::load(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("can't open map file: " + path);
    }
    return parse(file);
}

bool Grid::in_bounds(Point p) const {
    return p.x >= 0 && p.x < width_ && p.y >= 0 && p.y < height_;
}

bool Grid::passable(Point p) const {
    return in_bounds(p) && open_[static_cast<std::size_t>(index(p))] != 0;
}

std::vector<Point> Grid::neighbors(Point p, Moves moves) const {
    std::vector<Point> result;

    // straight moves, in a fixed order so results are repeatable: up, right, down, left
    const Point straight[] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    for (const Point& d : straight) {
        Point next{p.x + d.x, p.y + d.y};
        if (passable(next)) {
            result.push_back(next);
        }
    }

    if (moves == Moves::Eight) {
        const Point diagonal[] = {{1, -1}, {1, 1}, {-1, 1}, {-1, -1}};
        for (const Point& d : diagonal) {
            Point next{p.x + d.x, p.y + d.y};
            // both side cells must be open, so we never squeeze between two walls
            bool sides_open = passable({p.x + d.x, p.y}) && passable({p.x, p.y + d.y});
            if (passable(next) && sides_open) {
                result.push_back(next);
            }
        }
    }
    return result;
}

}  // namespace pathfinder