#pragma once

#include "../TileMapRenderer.h"
#include "../TileMap.h"

enum class TilePathingState
{
    Empty,
    Visited
};

class PathFindingGrid
{
  public:
    PathFindingGrid();

    void create_pathing_graph(const TileMap& tilemap, TileMapKind kind);

    void draw_costs(sf::RenderTarget& render_target);

  private:
    void set_tile_cost(const sf::Vector2i tile_position, int cost);

    void create_path_cost_top_down(const TileMap& tilemap, const sf::Vector2i tile_position);
    void create_path_cost_side_view(const TileMap& tilemap, const sf::Vector2i tile_position);

    Array2D<uint8_t> tile_costs_;
    TileMapRenderer tile_costs_renderer;
};
