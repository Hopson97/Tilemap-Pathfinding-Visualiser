#include "PathfindingCostGrid.h"

namespace
{
    const sf::Vector2i BELOW = {0, 1};
    const sf::Vector2i BELOW2 = {0, 2};
    const sf::Vector2i LEFT_BELOW = {-1, 1};
    const sf::Vector2i RIGHT_BELOW = {1, 1};

    const sf::Vector2i LEFT = {-1, 0};
    const sf::Vector2i RIGHT = {1, 0};

    constexpr int JUMP_GAP_COST = 4;

    namespace Colour
    {
        const sf::Color BLOCKED = {255, 0, 0, 128};
        const sf::Color FREE = {0, 255, 0, 128};
    } // namespace Colour
} // namespace

PathFindingCostGrid::PathFindingCostGrid()
    : tile_costs_(TILE_MAP_WIDTH, TILE_MAP_HEIGHT)
{
}

void PathFindingCostGrid::create_pathing_graph(const TileMap& tilemap, TileMapKind kind)
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

void PathFindingCostGrid::draw(sf::RenderTarget& render_target)
{
    tile_costs_renderer_.draw(render_target);
}

void PathFindingCostGrid::clear_all()
{
    // Clear the pathing cost grid
    tile_costs_.fill(0);
    for (int x = 0; x < TILE_MAP_WIDTH; x++)
    {
        for (int y = 0; y < TILE_MAP_HEIGHT; y++)
        {
            tile_costs_renderer_.set_tile_colour({x, y}, sf::Color::Transparent);
        }
    }
}

int PathFindingCostGrid::get_cost(const sf::Vector2i tile_position) const
{
    if (tile_costs_.contains(tile_position.x, tile_position.y))
    {
        return tile_costs_.get(tile_position.x, tile_position.y);
    }
}

void PathFindingCostGrid::set_tile_cost(const sf::Vector2i tile_position, int cost)
{
    tile_costs_.set(tile_position.x, tile_position.y, cost);
    tile_costs_renderer_.set_tile_colour(tile_position,
                                         cost == -1 ? Colour::BLOCKED : Colour::FREE);
}

void PathFindingCostGrid::create_path_cost_top_down(const TileMap& tilemap,
                                                const sf::Vector2i tile_position)
{
    auto tile = tilemap.get_tiles_at(tile_position);
    if (tile.background.block_pathing && tile.foreground.block_pathing ||
        tilemap.is_blocking_tile(tile_position))
    {
        set_tile_cost(tile_position, -1);
        return;
    }
    else if (tilemap.is_blocking_tile(tile_position, TileType::Layer::Background))
    {
        set_tile_cost(tile_position, -1);
        return;
    }
    set_tile_cost(tile_position, tile.background.cost);
}

void PathFindingCostGrid::create_path_cost_side_view(const TileMap& tilemap,
                                                 const sf::Vector2i tile_position)
{
    if (!tilemap.is_empty(tile_position))
    {
        set_tile_cost(tile_position, -1);
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

            for (int y = 2; y < 4; y++)
            {
                if (tilemap.is_empty(tile_position + sf::Vector2i{0, -y}) &&
                    (!tilemap.is_empty(tile_position + sf::Vector2i{-y - 1, -y - 1}) ||
                     !tilemap.is_empty(tile_position + sf::Vector2i{y - 1, -y - 1})))
                {
                    set_tile_cost(tile_position + sf::Vector2i{0, -y}, JUMP_GAP_COST);
                }
            }

            return;
        }
    }

    // Check if there is a jumpable gap
    if (tile_position.x > 0 && tile_position.x < TILE_MAP_WIDTH - 1 &&
        tile_position.y < TILE_MAP_HEIGHT - 1)
    {
        if (tilemap.is_empty(tile_position + LEFT) && tilemap.is_empty(tile_position + RIGHT) &&
            tilemap.is_empty(tile_position + BELOW) &&
            !tilemap.is_empty(tile_position + LEFT_BELOW) &&
            !tilemap.is_empty(tile_position + RIGHT_BELOW))
        {
            set_tile_cost(tile_position, JUMP_GAP_COST);
            return;
        }
    }

    set_tile_cost(tile_position, -1);
}