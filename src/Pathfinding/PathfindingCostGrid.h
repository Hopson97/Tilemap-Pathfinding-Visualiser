#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

#include "../TileMapRenderer.h"
#include "../TileMap.h"


class PathFindingCostGrid
{
  public:
    PathFindingCostGrid();

    void create_pathing_graph(const TileMap& tilemap, TileMapKind kind);

    void draw(sf::RenderTarget& render_target);

    void clear_all();

    int get_cost(const sf::Vector2i tile_position) const;

  private:
    void set_tile_cost(const sf::Vector2i tile_position, int cost);

    void create_path_cost_top_down(const TileMap& tilemap, const sf::Vector2i tile_position);
    void create_path_cost_side_view(const TileMap& tilemap, const sf::Vector2i tile_position);

    Array2D<int> tile_costs_;
    TileMapRenderer tile_costs_renderer_;

    sf::Font font_;
    sf::Text text_;
};
