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

struct PathFindingVisualiserConfig
{
    bool render_visited_tiles = true;
    bool render_pathing_tiles = true;

    sf::Color visited_colour = {50, 100, 255, 128};
    sf::Color path_colour = {0, 255, 255, 128};

};

class PathFindingVisualiser
{
  public:
    PathFindingVisualiser();

    void clear();

    void set_state(sf::Vector2i& tile_position, PathFindingState state);
    void draw(sf::RenderTarget& render_target);

    PathFindingVisualiserConfig config;

  private:
    Array2D<PathFindingState> states_;
    TileMapRenderer visited_tiles_;
    TileMapRenderer path_tiles_;
};
