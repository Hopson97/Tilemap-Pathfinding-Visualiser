#pragma once

#include <array>
#include <filesystem>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>

#include "TileMapRenderer.h"
#include "Util/Array2D.h"

const std::array<sf::Vector2i, 4> TILE_OFFSETS = {sf::Vector2i{0, 1}, {-1, 0}, {1, 0}, {0, -1}};

using TileId = int16_t;

enum class TileMapKind
{
    SideView,
    TopDownView,
};

/**
 * Used for determining whether a position is "non-existant" for example if the start/finish have
 * NOT been placed they "at" this.
 */
const sf::Vector2i NO_POSITION = {-1, -1};

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

/**
 *  When getting the tiles at a particular location, this wraps getting the info for all the layers
 */
struct TileLayer
{
    const TileType& background;
    const TileType& foreground;
};

/**
 * The grid of tiles
 */
struct TileMap
{
    enum class SetTileAction
    {
        Place,
        Remove
    };

  public:
    TileMap(const std::filesystem::path& tile_config);

    const TileType& tile_info(TileId tile_id) const;
    TileLayer get_tiles_at(const sf::Vector2i& tile_position) const;

    void set_tile(const sf::Vector2i& tile_position, TileId tile_id);
    void remove_tile(const sf::Vector2i& tile_position);

    size_t tile_type_count() const;
    const sf::Texture& texture() const;

    void draw(sf::RenderTarget& render_target);

    bool is_empty(const sf::Vector2i& tile_position) const;

    /**
     * Returns true if the PLACED DOWN tile is a blocking one on the given layer
     */
    bool is_blocking_tile(const sf::Vector2i& tile_position, TileType::Layer layer) const;

    /**
     * Returns true if the PLACED DOWN tile is a blocking one on either layer
     */
    bool is_blocking_tile(const sf::Vector2i& tile_position) const;

    std::optional<sf::Vector2i> start_position() const;
    std::optional<sf::Vector2i> finish_position() const;

  private:
    void set_tile(const sf::Vector2i& tile_position, TileId tile_id, SetTileAction action);

    void update_foreground_tile_variation(const sf::Vector2i& tile_position);
    void update_background_tile_variation(const sf::Vector2i& tile_position);

  private:
    std::vector<TileType> tile_types_;
    Array2D<TileId> tiles_foreground_;
    Array2D<TileId> tiles_background_;

    sf::Texture tiles_texture_;

    TileMapRenderer renderer_background_;
    TileMapRenderer renderer_foreground_;

    TileId empty_background_tile_ = -1;
    TileId empty_foreground_tile_ = -1;

    TileId start_finish_layer_empty_tile_ = -1;

    TileId start_tile_id_ = -1;
    TileId finish_tile_id_ = -1;


    sf::Vector2i start_position_ = NO_POSITION;
    sf::Vector2i finish_position_ = NO_POSITION;
};