#include "PathfindingGrid.h"

namespace
{
    const sf::Vector2i BELOW = {0, 1};
    const sf::Vector2i BELOW2 = {0, 2};
    const sf::Vector2i LEFT_BELOW = {-1, 1};
    const sf::Vector2i RIGHT_BELOW = {1, 1};

    constexpr int JUMP_GAP_COST = 4;

    namespace Colour
    {
        const sf::Color BLOCKED = {255, 0, 0, 128};
        const sf::Color FREE = {0, 255, 0, 128};
    } // namespace Colour
} // namespace

PathFindingGrid::PathFindingGrid()
    : tile_costs_(TILE_MAP_WIDTH, TILE_MAP_HEIGHT)
{
}

void PathFindingGrid::create_pathing_graph(const TileMap& tilemap, TileMapKind kind)
{
    for (int x = 0; x < TILE_MAP_WIDTH; x++)
    {
        for (int y = 0; y < TILE_MAP_HEIGHT; y++)
        {

            switch (kind)
            {
                case TileMapKind::SideView:
                    create_path_cost_side_view(tilemap, {x, y});
                    break;
                case TileMapKind::TopDownView:
                    create_path_cost_top_down(tilemap, {x, y});
                    break;
                default:
                    break;
            }
        }
    }
}

void PathFindingGrid::draw_costs(sf::RenderTarget& render_target)
{
    tile_costs_renderer.draw(render_target);
}

void PathFindingGrid::set_tile_cost(const sf::Vector2i tile_position, int cost)
{
    tile_costs_.set(tile_position.x, tile_position.y, cost);
    tile_costs_renderer.set_tile_colour(tile_position, cost == -1 ? Colour::BLOCKED : Colour::FREE);
}

void PathFindingGrid::create_path_cost_top_down(const TileMap& tilemap,
                                                const sf::Vector2i tile_position)
{
}

void PathFindingGrid::create_path_cost_side_view(const TileMap& tilemap,
                                                 const sf::Vector2i tile_position)
{
    if (!tilemap.is_empty(tile_position))
    {
        tile_costs_.set(tile_position.x, tile_position.y, -1);
        tile_costs_renderer.set_tile_colour(tile_position, Colour::BLOCKED);
        return;
    }

    auto below_tile = tile_position + BELOW;
    auto below_tile_2 = tile_position + BELOW2;

    // Check if the below tile is solid
    if (tile_position.y < TILE_MAP_HEIGHT - 1)
    {
        if (!tilemap.is_empty(below_tile))
        {
            set_tile_cost(tile_position, tilemap.get_tiles_at(below_tile).background.cost);

            // Check for "cliffs" up to 2-height
            if (tilemap.is_empty(tile_position + sf::Vector2i{0, -1}) &&
                (!tilemap.is_empty(tile_position + sf::Vector2i{-1, 0}) ||
                 !tilemap.is_empty(tile_position + sf::Vector2i{1, 0})))
            {
                set_tile_cost(tile_position + sf::Vector2i{0, -1}, JUMP_GAP_COST);
            }
            else if (tilemap.is_empty(tile_position + sf::Vector2i{0, -2}) &&
                     (!tilemap.is_empty(tile_position + sf::Vector2i{-1, -1}) ||
                      !tilemap.is_empty(tile_position + sf::Vector2i{1, -1})))
            {
                set_tile_cost(tile_position + sf::Vector2i{0, -2}, JUMP_GAP_COST);
            }
            return;
        }
    }

    // Check if there is a jumpable gap
    if (tile_position.x > 0 && tile_position.x < TILE_MAP_WIDTH - 1 &&
        tile_position.y < TILE_MAP_HEIGHT - 1)
    {
        if (tilemap.is_empty(tile_position + BELOW) &&
            !tilemap.is_empty(tile_position + LEFT_BELOW) &&
            !tilemap.is_empty(tile_position + RIGHT_BELOW))
        {
            set_tile_cost(tile_position, JUMP_GAP_COST);
            return;
        }
    }

    tile_costs_.set(tile_position.x, tile_position.y, -1);
    tile_costs_renderer.set_tile_colour(tile_position, Colour::BLOCKED);
}