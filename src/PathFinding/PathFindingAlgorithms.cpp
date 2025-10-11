#include "PathFindingAlgorithms.h"

PathFindingResult::PathFindingResult(PathfindingNode start, PathfindingNode finish)

    : start{start}
    , finish{finish}
{
}

void PathFindingResult::push_node(const PathfindingNode& current, const sf::Vector2i& next_node)

{
    came_from[next_node] = current;
    visited.push_back(next_node);
}

std::deque<PathfindingNode> PathFindingResult::create_path() const

{
    std::deque<PathfindingNode> path;

    PathfindingNode current = finish;

    // Back track through the came_from map until the start is found
    while (current.position != start.position)
    {
        path.push_back(current);
        current = came_from.at(current.position);
    }
    path.push_back(start);
    return path;
}

void PathFindingResult::set_finish_found(const PathfindingNode& current)
{
    // Ensure the finish is actually in the "came_from" map such that a complete path can be
    // created
    if (came_from.find(finish.position) == came_from.end())
    {
        push_node(current, finish.position);
    }
    finish_found = true;
}
