#include "TileType.h"

#include "TileMapRenderer.h"

TileType::TileType(TileId id, std::string name, int index, int cost, bool connect_to_neighbours,
                   Layer layer, const Special special)
    : id(id)
    , texture_rect{{0, static_cast<float>(index) * TEXTURE_SIZE}, {TEXTURE_SIZE, TEXTURE_SIZE}}
    , name{name}
    , cost{cost}
    , connect_to_neighbours{connect_to_neighbours}
    , block_pathing{cost == -1}
    , layer{layer}
    , special{special}
{
}

sf::FloatRect TileType::get_normalised_texture_rect(const sf::Vector2f& atlas_size) const
{
    auto x1 = texture_rect.position.x / atlas_size.x;
    auto y1 = texture_rect.position.y / atlas_size.y;
    auto x2 = x1 + TEXTURE_SIZE / atlas_size.x;
    auto y2 = y1 + TEXTURE_SIZE / atlas_size.y;

    return {{x1, y1}, {x2, y2}};
}