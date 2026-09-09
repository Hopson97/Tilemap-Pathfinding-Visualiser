#include "PathFindingAlgorithms.h"

#include <print>

#include "PathFindingCostGrid.h"

PathFindingResult breadth_first_search(const PathFindingCostGrid& grid, const sf::Vector2i& start,
                                       const sf::Vector2i& finish)
{
    PathFindingResult result{{start, grid.get_cost(start)}, {finish, grid.get_cost(finish)}};

    // The current queue of tiles to next be processed
    std::deque<PathfindingNode> queue;

    // Push the start to the queue as the starting point of the search
    queue.push_back({start, grid.get_cost(start)});
    result.visited.push_back(start);

    while (!queue.empty())
    {
        // Get the next item in the queue
        auto current = queue.front();
        queue.pop_front();

        // Goal found, exit
        if (current.position == finish || result.finish_found)
        {
            result.finish_found = true;
            break;
        }

        // For every neighbour...
        for (const auto& neighbour : NEIGHBOUR_TILES)
        {
            // Check if the tile was visited - add to the queue if it not
            const auto next_tile = current.position + neighbour;
            if (!result.came_from.contains(next_tile) &&
                grid.traversable(current.position, next_tile))
            {
                queue.push_back({next_tile, grid.get_cost(next_tile)});
                result.push_node(current, next_tile);
            }

            // Goal found, exit
            if (next_tile == finish)
            {
                result.set_finish_found(current);
                break;
            }
        }
    }

    return result;
}