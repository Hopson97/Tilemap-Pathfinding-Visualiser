#include "PathFindingAlgorithms.h"

#include <print>
#include <unordered_map>

#include "PathFindingCostGrid.h"

PathFindingResult breadth_first_search(const PathFindingCostGrid& grid, const sf::Vector2i& start,
                                       const sf::Vector2i& finish)
{
    PathFindingResult result;

    // The current queue of tiles to next be processed
    std::deque<sf::Vector2i> queue;

    // Keep track of where each visited node came from so the path can be constructed
    std::unordered_map<sf::Vector2i, sf::Vector2i, HashVec2> came_from;

    // Push the start to the queue as the starting point of the search
    queue.push_back(start);
    result.visited.push_back(start);

    bool found = false;
    while (!queue.empty())
    {
        // Get the next item in the queue
        auto current = queue.front();
        queue.pop_front();

        // Goal found, exit
        if (current == finish || found)
        {
            break;
        }

        // For every neighbour...
        for (const auto& neighbour : NEIGHBOUR_TILES)
        {
            // Check if the tile was visited - add to the queue if it not
            const auto next_tile = current + neighbour;
            if (came_from.find(next_tile) == came_from.end() &&
                grid.traversable(current, next_tile))
            {
                came_from[next_tile] = current;
                queue.push_back(next_tile);
                result.visited.push_back(next_tile);
            }

            // Goal found, exit
            if (next_tile == finish)
            {
                found = true;
                break;
            }
        }
    }

    return result;
}