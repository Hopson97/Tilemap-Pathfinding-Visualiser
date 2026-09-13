#include "PathFindingVisualiser.h"

PathFindingVisualiser::PathFindingVisualiser()
    : states_(TILE_MAP_WIDTH, TILE_MAP_HEIGHT)
{
    clear();
}

void PathFindingVisualiser::clear()
{
    states_.fill(PathFindingState::NotVisited);
    for (int x = 0; x < TILE_MAP_WIDTH; x++)
    {
        for (int y = 0; y < TILE_MAP_HEIGHT; y++)
        {
            visited_tiles_.set_tile_colour({x, y}, sf::Color::Transparent);
            path_tiles_.set_tile_colour({x, y}, sf::Color::Transparent);
        }
    }
}

void PathFindingVisualiser::set_state(sf::Vector2i& tile_position, PathFindingState state)
{
    states_.set(tile_position.x, tile_position.y, state);

    switch (state)
    {
        case PathFindingState::Visited:
            visited_tiles_.set_tile_colour(tile_position, config.visited_colour);
            break;

        case PathFindingState::Path:
            path_tiles_.set_tile_colour(tile_position, config.path_colour);
            break;
    }
}

void PathFindingVisualiser::draw_visited(sf::RenderTarget& render_target)
{
    if (config.render_visited_tiles)
    {
        visited_tiles_.draw(render_target);
    }
}

void PathFindingVisualiser::draw_path(sf::RenderTarget& render_target)
{
    if (config.render_pathing_tiles)
    {
        path_tiles_.draw(render_target);
    }
}
