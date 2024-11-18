#include "TileMap.h"

#include <cassert>
#include <fstream>
#include <print>

#include <nlohmann/json.hpp>

#include "TileMapRenderer.h"

TileType::TileType(TileId id, std::string name, int index, int cost, bool connect_to_neighbours,
                   Layer layer, const Special special)
    : id(id)
    , texture_rect{0, static_cast<float>(index) * TEXTURE_SIZE, TEXTURE_SIZE, TEXTURE_SIZE}
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
    auto x1 = texture_rect.left / atlas_size.x;
    auto y1 = texture_rect.top / atlas_size.y;
    auto x2 = x1 + TEXTURE_SIZE / atlas_size.x;
    auto y2 = y1 + TEXTURE_SIZE / atlas_size.y;

    return {x1, y1, x2, y2};
}

TileMap::TileMap(const std::filesystem::path& tile_config)
    : tiles_foreground_(TILE_MAP_WIDTH, TILE_MAP_HEIGHT)
    , tiles_background_(TILE_MAP_WIDTH, TILE_MAP_HEIGHT)
{
    std::ifstream file(tile_config);

    auto data = nlohmann::json::parse(file);
    std::string texture_file = data["texture"];
    if (!tiles_texture_.loadFromFile("assets/Textures/" + texture_file))
    {
        return;
    }

    // Load the tile set configurations from the given JSON file.
    nlohmann::json tiles = data["tiles"];
    TileType::Layer start_finish_layer;
    for (auto& tile : tiles)
    {
        std::string name = tile["name"];
        int texture_index = tile["texture_index"];
        bool connect_to_neighbours = tile["connect_to_neighbours"];

        // -1 means it cannot be passed by default
        int pathing_cost = -1;
        if (tile.contains("pathing_cost"))
        {
            pathing_cost = tile["pathing_cost"];
        }

        TileType::Layer layer;
        {
            std::string layer_name = tile["layer"];
            if (layer_name == "background")
            {
                layer = TileType::Layer::Background;
            }
            else if (layer_name == "foreground")
            {
                layer = TileType::Layer::Foreground;
            }
        }

        TileType::Special special = TileType::Special::No;

        if (tile.contains("special"))
        {
            std::string layer_name = tile["special"];
            if (layer_name == "start")
            {
                special = TileType::Special::Start;
                start_finish_layer = layer;
            }
            else if (layer_name == "finish")
            {
                special = TileType::Special::Finish;
                start_finish_layer = layer;
            }
        }

        auto tile_id = tile_types_
                           .emplace_back(tile_types_.size(), name, texture_index, pathing_cost,
                                         connect_to_neighbours, layer, special)
                           .id;

        if (special == TileType::Special::Start)
        {
            start_tile_id_ = tile_id;
        }
        else if (special == TileType::Special::Finish)
        {
            finish_tile_id_ = tile_id;
        }
    }

    // Add the "empty/error" tile and fill the map with it
    empty_background_tile_ = tile_types_
                                 .emplace_back(tile_types_.size(), "ERROR TILE", -1, -1, false,
                                               TileType::Layer::Background, TileType::Special::No)
                                 .id;
    empty_foreground_tile_ = tile_types_
                                 .emplace_back(tile_types_.size(), "ERROR TILE", -1, -1, false,
                                               TileType::Layer::Foreground, TileType::Special::No)
                                 .id;

    tiles_background_.fill(empty_background_tile_);
    tiles_foreground_.fill(empty_foreground_tile_);

    if (start_finish_layer == TileType::Layer::Background)
    {
        start_finish_layer_empty_tile_ = empty_background_tile_;
    }
    else if (start_finish_layer == TileType::Layer::Foreground)
    {
        start_finish_layer_empty_tile_ = empty_foreground_tile_;
    }
}

void TileMap::set_tile(const sf::Vector2i& tile_position, TileId tile_id)
{
    set_tile(tile_position, tile_id, SetTileAction::Place);
}

void TileMap::remove_tile(const sf::Vector2i& tile_position)
{
    auto tiles = get_tiles_at(tile_position);

    // Prefer removing the "foreground" tile over the "background" tile.
    if (tiles.foreground.id != empty_foreground_tile_)
    {
        set_tile(tile_position, empty_foreground_tile_, SetTileAction::Remove);
    }
    else if (tiles.background.id != empty_background_tile_)
    {
        set_tile(tile_position, empty_background_tile_, SetTileAction::Remove);
    }
}

size_t TileMap::tile_type_count() const
{
    return tile_types_.size();
}

const sf::Texture& TileMap::texture() const
{
    return tiles_texture_;
}

void TileMap::draw(sf::RenderTarget& render_target)
{
    sf::RenderStates states;
    states.texture = &tiles_texture_;
    renderer_background_.draw(render_target, states);
    renderer_foreground_.draw(render_target, states);
}

void TileMap::set_tile(const sf::Vector2i& tile_position, TileId tile_id, SetTileAction action)
{
    if (tiles_background_.contains(tile_position.x, tile_position.y))

    {
        auto& info = tile_info(tile_id);

        // Ensure that start/finish position can only be replaced ONCE
        auto try_place_start_finish = [&](TileType::Special special, sf::Vector2i& position)
        {
            if (info.special == special)
            {
                if (position != NO_POSITION)
                {
                    std::println("Removing tile at {} {}", position.x, position.y);
                    set_tile(position, start_finish_layer_empty_tile_, SetTileAction::Remove);
                }

                position = tile_position;
            }
        };
        if (action == SetTileAction::Place)
        {
            try_place_start_finish(TileType::Special::Start, start_position_);
            try_place_start_finish(TileType::Special::Finish, finish_position_);
        }

        auto colour = action == SetTileAction::Place ? sf::Color::White : sf::Color::Transparent;

        switch (info.layer)
        {
            case TileType::Layer::Background:
                tiles_background_.set(tile_position.x, tile_position.y, tile_id);

                renderer_background_.set_tile_colour(tile_position, colour);

                update_background_tile_variation(tile_position);
                for (auto& offset : TILE_OFFSETS)
                {
                    update_background_tile_variation(offset + tile_position);
                }
                break;

            case TileType::Layer::Foreground:
                tiles_foreground_.set(tile_position.x, tile_position.y, tile_id);
                update_foreground_tile_variation(tile_position);
                renderer_foreground_.set_tile_colour(tile_position, colour);
                for (auto& offset : TILE_OFFSETS)
                {
                    update_foreground_tile_variation(offset + tile_position);
                }
                break;
            default:
                break;
        }
    }
}

void TileMap::update_foreground_tile_variation(const sf::Vector2i& tile_position)
{
    auto& tile = get_tiles_at(tile_position).foreground;

    int variation = 0;
    if (tile.connect_to_neighbours)
    {
        for (int i = 0; i < TILE_OFFSETS.size(); i++)
        {
            auto& neighbour = get_tiles_at(TILE_OFFSETS[i] + tile_position).foreground;
            if (neighbour.id != empty_foreground_tile_)
            {
                variation += static_cast<int>(std::pow(2, i));
            }
        }
    }
    auto texture_rect = tile.texture_rect;
    texture_rect.left = variation * TEXTURE_SIZE;
    renderer_foreground_.set_tile_texture_rect(tile_position, texture_rect);
}

void TileMap::update_background_tile_variation(const sf::Vector2i& tile_position)
{
    auto& tile = get_tiles_at(tile_position).background;

    int variation = 0;
    if (tile.connect_to_neighbours)
    {
        for (int i = 0; i < TILE_OFFSETS.size(); i++)
        {
            auto neighbour_position = TILE_OFFSETS[i] + tile_position;
            auto& neighbour = get_tiles_at(neighbour_position).background;
            if (neighbour.id != empty_background_tile_ && neighbour.id != start_tile_id_ && neighbour.id != finish_tile_id_)
            {
                variation += static_cast<int>(std::pow(2, i));
            }
        }
    }
    auto texture_rect = tile.texture_rect;
    texture_rect.left = variation * TEXTURE_SIZE;
    renderer_background_.set_tile_texture_rect(tile_position, texture_rect);
}

const TileType& TileMap::tile_info(TileId tile_id) const
{
    assert(tile_id >= 0 && tile_id < tile_types_.size());
    return tile_types_[tile_id];
}

TileLayer TileMap::get_tiles_at(const sf::Vector2i& tile_position) const
{
    if (tiles_background_.contains(tile_position.x, tile_position.y))
    {
        return {.background = tile_info(tiles_background_.get(tile_position.x, tile_position.y)),
                .foreground = tile_info(tiles_foreground_.get(tile_position.x, tile_position.y))};
    }

    // Back has the "empty" tile
    return {.background = tile_types_.back(), .foreground = tile_types_.back()};
}