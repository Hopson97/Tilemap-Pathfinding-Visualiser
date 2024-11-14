#include "TileMap.h"

#include "TileMapRenderer.h"

void Tile::init(const char* name, TileType type, int cost, bool connect_to_neighbours_side_view,
                bool connect_to_neighbours_top_down)
{
    this->name = name;
    this->type = type;
    this->cost = cost;
    this->texture = {
        0,
        static_cast<float>(type) * TEXTURE_SIZE,
        TEXTURE_SIZE,
        TEXTURE_SIZE,
    };

    this->connect_to_neighbours_side_view = connect_to_neighbours_side_view;
    this->connect_to_neighbours_top_down = connect_to_neighbours_top_down;
}

sf::FloatRect Tile::get_normalised_texture_rect(const sf::Vector2f& atlas_size) const
{
    auto x1 = texture.left / atlas_size.x;
    auto y1 = texture.top / atlas_size.y;
    auto x2 = x1 + TEXTURE_SIZE / atlas_size.x;
    auto y2 = y1 + TEXTURE_SIZE / atlas_size.y;

    return {x1, y1, x2, y2};
}

TileMap::TileMap()
    : tiles(TILE_MAP_WIDTH, TILE_MAP_HEIGHT)
{
    tiles.fill(TileType::Empty);

    tile_types[(int)TileType::Grass].init("Grass", TileType::Grass, 1, true, false);
    tile_types[(int)TileType::Sand].init("Sand", TileType::Sand, 2, false, true);

    tile_types[(int)TileType::Empty].init("Empty", TileType::Empty, -1, false, false);
    tile_types[(int)TileType::OOB].init("Out of Bounds", TileType::OOB, -1, false, false);
}

void TileMap::set_tile(const sf::Vector2i& tile_position, TileType type)
{
    if (tiles.contains(tile_position.x, tile_position.y))
    {
        tiles.set(tile_position.x, tile_position.y, type);
    }
}

const Tile& TileMap::get_tile(TileType type) const
{
    return tile_types[(int)type];
}

const Tile& TileMap::get_tile(const sf::Vector2i& tile_position) const
{
    if (tiles.contains(tile_position.x, tile_position.y))
    {
        return get_tile(tiles.get(tile_position.x, tile_position.y));
    }
    return oob_tile_;
}
