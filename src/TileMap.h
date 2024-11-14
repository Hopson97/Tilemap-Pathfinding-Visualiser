#pragma once

#include <array>

#include <SFML/Graphics/Rect.hpp>

#include "Util/Array2D.h"

enum class TileType
{
    Grass = 0,
    Sand,

    Empty,
    OOB,
    NUM_TILES
};

enum class TileMapKind
{
    SideView,
    TopDownView,
};

struct Tile
{
    void init(const char* name, TileType type, int cost, bool connect_to_neighbours_side_view,
              bool connect_to_neighbours_top_down);

    TileType type = TileType::Empty;
    const char* name;
    int cost = 1;
    sf::FloatRect texture;

    sf::FloatRect get_normalised_texture_rect(const sf::Vector2f& atlas_size) const;

    bool connect_to_neighbours_side_view = false;
    bool connect_to_neighbours_top_down = false;
};

struct TileMap
{
  public:
    TileMap();

    const Tile& get_tile(TileType type) const;
    const Tile& get_tile(const sf::Vector2i& tile_position) const;
    void set_tile(const sf::Vector2i& tile_position, TileType type);

    std::array<Tile, (int)TileType::NUM_TILES> tile_types;


  private:
    Tile oob_tile_;
    Array2D<TileType> tiles;
};