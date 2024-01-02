#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include "Util/Array2D.h"

constexpr float TILE_SIZE = 32.0f;



class TileMap
{
  public:
    TileMap();

    void draw(sf::RenderTarget& render_target, const sf::RenderStates& states);

  private:
    Array2D<sf::Vertex, 4> tile_vertices_;
};
