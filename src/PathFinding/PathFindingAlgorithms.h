#pragma once

#include <array>
#include <deque>
#include <string>
#include <unordered_map>

#include <SFML/System/Vector2.hpp>

class PathFindingCostGrid;

struct HashVec2
{
    size_t operator()(const sf::Vector2i& v) const
    {
        return std::hash<int>()(v.x) ^ std::hash<int>()(v.y);
    }
};

/// Minimal struct for a given tile's position and "weight"
struct PathfindingNode
{
    sf::Vector2i position;
    int cost = 0;
};

struct ComparePathfindingNodeCosts
{
    bool operator()(const PathfindingNode& lhs, const PathfindingNode& rhs)
    {
        return lhs.cost > rhs.cost;
    }
};

struct PathFindingResult
{
    PathFindingResult() = default;
    PathFindingResult(PathfindingNode start, PathfindingNode finish);

    void push_node(const PathfindingNode& current, const sf::Vector2i& next_node);

    std::deque<PathfindingNode> create_path() const;

    void set_finish_found(const PathfindingNode& current);

    std::deque<sf::Vector2i> visited;

    // Keep track of where each visited node came from so the path can be constructed
    std::unordered_map<sf::Vector2i, PathfindingNode, HashVec2> came_from;

    bool finish_found = false;

    PathfindingNode start;
    PathfindingNode finish;
};

// All neighbour offsets (vertical, horizontal, and diagonal.
const std::array<sf::Vector2i, 8> NEIGHBOUR_TILES = {
    sf::Vector2i{0, 1}, {-1, 0}, {1, 0}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};

PathFindingResult breadth_first_search(const PathFindingCostGrid& grid, const sf::Vector2i& start,
                                       const sf::Vector2i& finish);

PathFindingResult dijkstra_algorithm(const PathFindingCostGrid& grid, const sf::Vector2i& start,
                                     const sf::Vector2i& finish);