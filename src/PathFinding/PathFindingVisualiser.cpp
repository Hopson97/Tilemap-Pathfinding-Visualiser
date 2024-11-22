#include "PathFindingVisualiser.h"

namespace
{
    sf::Color state_to_colour(PathFindingState state)
    {
        switch (state)
        {
            case PathFindingState::NotVisited:
                return sf::Color::Transparent;

            case PathFindingState::Visited:
                return {125, 200, 255, 128};

            case PathFindingState::Path:
                return {0, 255, 255, 128};

            default:
                return sf::Color::Magenta;
        }
    }
} // namespace

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
            states_renderer_.set_tile_colour({x, y}, sf::Color::Transparent);
        }
    }
}

void PathFindingVisualiser::set_state(sf::Vector2i& tile_position, PathFindingState state)
{
    states_.set(tile_position.x, tile_position.y, state);
    states_renderer_.set_tile_colour(tile_position, state_to_colour(state));
}

void PathFindingVisualiser::draw(sf::RenderTarget& render_target)
{
    states_renderer_.draw(render_target);
}