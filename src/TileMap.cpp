#include "TileMap.h"

namespace
{
    void add_tile(Array2D<sf::Vertex, 4>& tile_vertices,
                  const sf::Vector2i& tile_position)
    {
        float x = static_cast<float>(tile_position.x);
        float y = static_cast<float>(tile_position.y);

        auto i = tile_vertices.index(tile_position.x, tile_position.y);

        tile_vertices.get(i).position = {x * TILE_SIZE, y * TILE_SIZE};
        tile_vertices.get(i + 1).position = {(x + 1.0f) * TILE_SIZE, y * TILE_SIZE};
        tile_vertices.get(i + 2).position = {(x + 1.0f) * TILE_SIZE, (y + 1.0f) * TILE_SIZE};
        tile_vertices.get(i + 3).position = {x * TILE_SIZE, (y + 1.0f) * TILE_SIZE};

        tile_vertices.get(i).color = sf::Color::Red;
        tile_vertices.get(i + 1).color = sf::Color::Green;
        tile_vertices.get(i + 2).color = sf::Color::Yellow;
        tile_vertices.get(i + 3).color = sf::Color::Blue;
    }

    constexpr int WIDTH = 100;
    constexpr int HEIGHT = 100;
} // namespace

TileMap::TileMap()
    : tile_vertices_(WIDTH, HEIGHT)
{
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < HEIGHT; x++)
        {
            add_tile(tile_vertices_, {x, y});
        }
    }
}

void TileMap::draw(sf::RenderTarget& render_target, const sf::RenderStates& states)
{
    render_target.draw(tile_vertices_.data(), tile_vertices_.size(), sf::Quads, states);
}
