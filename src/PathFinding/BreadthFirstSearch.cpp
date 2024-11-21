#include "PathfindingAlgorithms.h"

#include <print>
#include <unordered_map>

#include "PathfindingCostGrid.h"

PathFindingResult breadth_first_search(const PathFindingCostGrid& grid, const sf::Vector2i& start,
                                       const sf::Vector2i& finish)
{
    PathFindingResult result;
    std::deque<sf::Vector2i> queue;

    std::unordered_map<sf::Vector2i, sf::Vector2i, HashVec2> came_from;

    // Push the start to the queue as the starting point of the search
    queue.push_back(start);
    result.visited.push_back(start);

    while (!queue.empty())
    {
        // Get the next item in the queue
        auto current = queue.front();
        queue.pop_front();

        if (current == finish)
        {
            // Goal found, exit
            break;
        }

        // For every neighbour...
        for (const auto& neighbour : NEIGHBOUR_TILES)
        {
            // Check if the tile was visisted - add to the queue if it not
            const auto next_tile = current + neighbour;
            if (grid.traversable(current, next_tile) &&
                came_from.find(next_tile) == came_from.end())
            {
                came_from[next_tile] = current;
                queue.push_back(next_tile);
                result.visited.push_back(next_tile);
            }

            // Goal found, exit
            if (next_tile == finish)
            {

                break;
            }
        }
    }

    return result;
}