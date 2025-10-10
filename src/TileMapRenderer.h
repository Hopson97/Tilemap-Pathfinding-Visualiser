#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include "Util/Array2D.h"

#include "Constants.h"



sf::Vector2i world_to_tile_position(const sf::Vector2f world_position);

class TileMapGrid
{
  public:
    TileMapGrid();

    void draw(sf::RenderTarget& render_target);

  private:
    std::vector<sf::Vertex> grid_vertices_;
};

class TileMapRenderer
{
  public:
    TileMapRenderer();

    void set_tile_colour(const sf::Vector2i& tile_position, sf::Color colour);
    void set_tile_texture_rect(const sf::Vector2i& tile_position, const sf::FloatRect& rect);

    void draw(sf::RenderTarget& render_target, const sf::RenderStates& states = sf::RenderStates::Default);

  private:
    Array2D<sf::Vertex, 6> tile_vertices_;
};
