#pragma once

#include <array>
#include <deque>
#include <string>

#include <SFML/System/Vector2.hpp>

class PathFindingCostGrid;

struct HashVec2
{
    size_t operator()(const sf::Vector2i& v) const
    {
        return std::hash<int>()(v.x) ^ std::hash<int>()(v.y);
    }
};

struct PathFindingResult
{
    std::deque<sf::Vector2i> visited;
    std::deque<sf::Vector2i> path;
};

// All neighbour offsets (vertical, horizontal, and diagonal.
const std::array<sf::Vector2i, 8> NEIGHBOUR_TILES = {
    sf::Vector2i{0, 1}, {-1, 0}, {1, 0}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, -1}};

PathFindingResult breadth_first_search(const PathFindingCostGrid& grid, const sf::Vector2i& start,
                                       const sf::Vector2i& finish);