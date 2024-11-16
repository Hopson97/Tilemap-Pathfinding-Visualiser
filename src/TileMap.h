#pragma once

#include <array>
#include <filesystem>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "Util/Array2D.h"


using TileId = int16_t;

enum class TileMapKind
{
    SideView,
    TopDownView,
};

/**
*   Data about a tile type - used as a flyweight.
*/
struct TileType
{
    TileType(TileId id, std::string name, int index, int cost, bool connect_to_neighbours, bool blocks = false);

    const TileId id = 0;
    const sf::FloatRect texture_rect;

    const std::string name;
    const int cost = 1;

    const bool connect_to_neighbours = false;
    const bool block_pathing = false;

    sf::FloatRect get_normalised_texture_rect(const sf::Vector2f& atlas_size) const;

};

/**
* The grid of tiles
*/
struct TileMap
{
  public:
    TileMap(const std::filesystem::path& tile_config);

    void fill_map(TileId tile);

    const TileType& get_tile(TileId tile_id) const;
    const TileType& get_tile(const sf::Vector2i& tile_position) const;
    void set_tile(const sf::Vector2i& tile_position, TileId tile_id);

    size_t tile_type_count() const;
    const sf::Texture& texture() const;
    TileId empty_tile_id() const;

  private:
    std::vector<TileType> tile_types_;
    Array2D<TileId> tiles_;
    sf::Texture tiles_texture_;
};