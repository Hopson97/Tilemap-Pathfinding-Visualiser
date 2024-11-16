#include "TileMap.h"

#include <cassert>
#include <fstream>

#include <nlohmann/json.hpp>

#include "TileMapRenderer.h"

TileType::TileType(TileId id, std::string name, int index, int cost, bool connect_to_neighbours, bool block_pathing)
    : id(id)
    , texture_rect{0, static_cast<float>(index) * TEXTURE_SIZE, TEXTURE_SIZE, TEXTURE_SIZE}
    , name{name}
    , cost{cost}
    , connect_to_neighbours{connect_to_neighbours}
    , block_pathing(block_pathing)
{
}

sf::FloatRect TileType::get_normalised_texture_rect(const sf::Vector2f& atlas_size) const
{
    auto x1 = texture_rect.left / atlas_size.x;
    auto y1 = texture_rect.top / atlas_size.y;
    auto x2 = x1 + TEXTURE_SIZE / atlas_size.x;
    auto y2 = y1 + TEXTURE_SIZE / atlas_size.y;

    return {x1, y1, x2, y2};
}

TileMap::TileMap(const std::filesystem::path& tile_config)
    : tiles_(TILE_MAP_WIDTH, TILE_MAP_HEIGHT)
{
    std::ifstream file(tile_config);

    auto data = nlohmann::json::parse(file);
    std::string texture_file = data["texture"];
    if (!tiles_texture_.loadFromFile("assets/Textures/" + texture_file))
    {
        return;
    }

    nlohmann::json tiles = data["tiles"];
    for (auto& tile : tiles)
    {
        std::string name = tile["name"];
        int texture_index = tile["texture_index"];
        int pathing_cost = tile["pathing_cost"];
        bool connect_to_neighbours = tile["connect_to_neighbours"];

        bool block_pathing = false;
        if (tile.contains("block_pathing"))
        {
            block_pathing = tile["block_pathing"];
        }

        tile_types_.emplace_back(tile_types_.size(), name, texture_index, pathing_cost,
                                     connect_to_neighbours, block_pathing);
    }

    // Add the "empty/error" tile and fill the map with it
    tile_types_.emplace_back(tile_types_.size(), "ERROR TILE", -1, -1, false);
}

void TileMap::fill_map(TileId tile)
{
    tiles_.fill(tile);
}

void TileMap::set_tile(const sf::Vector2i& tile_position, TileId tile_id)
{
    if (tiles_.contains(tile_position.x, tile_position.y))
    {
        tiles_.set(tile_position.x, tile_position.y, tile_id);
    }
}

size_t TileMap::tile_type_count() const
{
    return tile_types_.size();
}

const sf::Texture& TileMap::texture() const
{
    return tiles_texture_;
}

TileId TileMap::empty_tile_id() const
{
    return tile_types_.back().id;
}

const TileType& TileMap::get_tile(TileId tile_id) const
{
    assert(tile_id >= 0 && tile_id < tile_types_.size());
    return tile_types_[tile_id];
}

const TileType& TileMap::get_tile(const sf::Vector2i& tile_position) const
{
    if (tiles_.contains(tile_position.x, tile_position.y))
    {
        return get_tile(tiles_.get(tile_position.x, tile_position.y));
    }
    // Back has the "empty" tile
    return tile_types_.back();
}
