#pragma once

#include <cstdint>
#include <string>

#include <SFML/Graphics/Rect.hpp>

using TileId = int16_t;

/**
 *   Data about a tile type - used as a flyweight.
 */
struct TileType
{
    enum class Layer
    {
        Background,
        Foreground
    };

    enum class Special
    {
        Start,
        Finish,
        No
    };

    TileType(TileId id, std::string name, int index, int cost, bool connect_to_neighbours,
             Layer layer, const Special special);

    const TileId id = 0;
    const sf::FloatRect texture_rect;

    const std::string name;
    const int cost = 1;

    const bool connect_to_neighbours = false;
    const bool block_pathing = false;

    const Layer layer;

    const Special special;

    sf::FloatRect get_normalised_texture_rect(const sf::Vector2f& atlas_size) const;
};