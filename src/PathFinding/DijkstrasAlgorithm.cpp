#include "PathFindingAlgorithms.h"

#include <print>
#include <queue>

#include "PathFindingCostGrid.h"

PathFindingResult dijkstra_algorithm(const PathFindingCostGrid& grid, const sf::Vector2i& start,
                                     const sf::Vector2i& finish)
{
    PathFindingResult result;

    // The current queue of tiles to next be processed,
    std::priority_queue<PathfindingNode, std::vector<PathfindingNode>, ComparePathfindingNodeCosts>
        queue;

    // Keep track of the total traversal cost so far
    std::unordered_map<sf::Vector2i, int, HashVec2> cost_so_far;

    // Push the start to the queue as the starting point of the search
    queue.push({start, 0});
    cost_so_far[start] = 0;
    result.visited.push_back(start);

    while (!queue.empty())
    {
        // Get the next item in the queue
        auto current = queue.top();
        queue.pop();

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

            // Get the cost the neighbour title and the total cost to get to it
            auto next_cost = grid.get_cost(next_tile);
            auto total_cost = cost_so_far[current.position] + next_cost;

            // Check if the tile has been visited
            // If it has, check the total cost - the new cost from a different direction might 
            // be a "cheaper" path
            bool visted = result.came_from.find(next_tile) != result.came_from.end();
            if (!visted && grid.traversable(current.position, next_tile) ||
                visted && total_cost < cost_so_far[next_tile])
            {

                cost_so_far[next_tile] = total_cost;
                queue.push({next_tile, total_cost});


                if (!visted)
                {
                    result.push_node(current, next_tile);
                }
            }

            // Goal found, exit
            if (next_tile == finish)
            {
                result.finish_found = true;
                break;
            }
        }
    }

    return result;
}