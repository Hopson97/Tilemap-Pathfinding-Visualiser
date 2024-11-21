#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

#include "../TileMap.h"
#include "../TileMapRenderer.h"

enum class PathFindingState
{
    NotVisited,
    Visited,
    Path
};

class PathFindingVisualiser
{
  public:
    PathFindingVisualiser();

    void clear();

    void set_state(sf::Vector2i& tile_position, PathFindingState state);
    void draw(sf::RenderTarget& render_target);

  private:
    Array2D<PathFindingState> states_;
    TileMapRenderer states_renderer_;
};
