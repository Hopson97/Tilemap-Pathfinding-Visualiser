#include "TileMap.h"

namespace
{
    void add_tile(Array2D<sf::Vertex, 4>& tile_vertices, const sf::Vector2i& tile_position)
    {
        float x = static_cast<float>(tile_position.x);
        float y = static_cast<float>(tile_position.y);
        auto i = tile_vertices.index(tile_position.x, tile_position.y);

        tile_vertices.get(i).position = {x * TILE_SIZE, y * TILE_SIZE};
        tile_vertices.get(i + 1).position = {(x + 1.0f) * TILE_SIZE, y * TILE_SIZE};
        tile_vertices.get(i + 2).position = {(x + 1.0f) * TILE_SIZE, (y + 1.0f) * TILE_SIZE};
        tile_vertices.get(i + 3).position = {x * TILE_SIZE, (y + 1.0f) * TILE_SIZE};

        tile_vertices.get(i + 0).color = sf::Color::Transparent;
        tile_vertices.get(i + 1).color = sf::Color::Transparent;
        tile_vertices.get(i + 2).color = sf::Color::Transparent;
        tile_vertices.get(i + 3).color = sf::Color::Transparent;
    }
} // namespace

TileMap::TileMap()
    : tile_vertices_(WIDTH, HEIGHT)
{
    // Create the actual tilemap
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < HEIGHT; x++)
        {
            add_tile(tile_vertices_, {x, y});
        }
    }

    // Create the grid lines
    sf::Color colour = sf::Color::Black;
    for (int x = 0; x < WIDTH + 1; x++)
    {
        grid_vertices_.push_back({{x * TILE_SIZE, 0}, colour});
        grid_vertices_.push_back({{x * TILE_SIZE, HEIGHT * TILE_SIZE}, colour});
    }
    for (int y = 0; y < HEIGHT + 1; y++)
    {
        grid_vertices_.push_back({{0, y * TILE_SIZE}, colour});
        grid_vertices_.push_back({{WIDTH * TILE_SIZE, y * TILE_SIZE}, colour});
    }
}

void TileMap::set_tile_colour(const sf::Vector2i& tile_position, sf::Color colour)
{
    if (tile_vertices_.contains(tile_position.x, tile_position.y))
    {
        auto i = tile_vertices_.index(tile_position.x, tile_position.y);
        tile_vertices_.get(i + 0).color = colour;
        tile_vertices_.get(i + 1).color = colour;
        tile_vertices_.get(i + 2).color = colour;
        tile_vertices_.get(i + 3).color = colour;
    }
}

void TileMap::set_tile_texture_rect(const sf::Vector2i& tile_position, const sf::FloatRect& rect)
{
    if (tile_vertices_.contains(tile_position.x, tile_position.y))
    {
        auto i = tile_vertices_.index(tile_position.x, tile_position.y);
        tile_vertices_.get(i + 0).texCoords = {rect.left, rect.top};
        tile_vertices_.get(i + 1).texCoords = {rect.left + rect.width, rect.top};
        tile_vertices_.get(i + 2).texCoords = {rect.left + rect.width, rect.top + rect.height};
        tile_vertices_.get(i + 3).texCoords = {rect.left, rect.top + rect.height};
    }
}

void TileMap::draw(sf::RenderTarget& render_target, const sf::RenderStates& states)
{
    render_target.draw(tile_vertices_.data(), tile_vertices_.size(), sf::Quads, states);
}

void TileMap::draw_grid(sf::RenderTarget& render_target)
{
    render_target.draw(grid_vertices_.data(), grid_vertices_.size(), sf::Lines);
}

sf::Vector2i world_to_tile_position(const sf::Vector2f world_position)
{
    return sf::Vector2i(world_position) / static_cast<int>(TILE_SIZE);
}
