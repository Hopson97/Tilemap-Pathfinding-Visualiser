#include "TileMap.h"

#include <cassert>
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include "TileMapRenderer.h"

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

        TileType::Layer layer = TileType::Layer::Background;
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
                pathing_cost = 0;
            }
            else if (layer_name == "finish")
            {
                special = TileType::Special::Finish;
                start_finish_layer = layer;
                pathing_cost = 0;
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

bool TileMap::is_empty(const sf::Vector2i& tile_position) const
{
    auto info = get_tiles_at(tile_position);
    return info.background.id == empty_background_tile_ &&
           info.foreground.id == empty_foreground_tile_;
}

bool TileMap::is_blocking_tile(const sf::Vector2i& tile_position, TileType::Layer layer) const
{
    auto info = get_tiles_at(tile_position);
    if (layer == TileType::Layer::Background)
    {
        return info.background.id != empty_background_tile_ && info.background.block_pathing;
    }
    if (layer == TileType::Layer::Foreground)
    {
        return info.foreground.id != empty_foreground_tile_ && info.foreground.block_pathing;
    }
    return true;
}

bool TileMap::is_blocking_tile(const sf::Vector2i& tile_position) const
{
    return is_blocking_tile(tile_position, TileType::Layer::Background) ||
           is_blocking_tile(tile_position, TileType::Layer::Foreground);
}

std::optional<sf::Vector2i> TileMap::start_position() const
{
    if (start_position_ == NO_POSITION)
    {
        return {};
    }
    return start_position_;
}

std::optional<sf::Vector2i> TileMap::finish_position() const
{
    if (finish_position_ == NO_POSITION)
    {
        return {};
    }
    return finish_position_;
}

void TileMap::save(const std::filesystem::path path)
{
    // Check the correct save folders exist
    if (!std::filesystem::exists("./data"))
    {
        std::filesystem::create_directories("./data");
    }

    std::ofstream f(path);
    for (int x = 0; x < TILE_MAP_WIDTH; x++)
    {
        for (int y = 0; y < TILE_MAP_HEIGHT; y++)
        {
            f << static_cast<int>(tiles_background_.get(x, y)) << ' ';
            f << static_cast<int>(tiles_foreground_.get(x, y)) << ' ';
        }
    }
}

bool TileMap::load(const std::filesystem::path path)
{
    if (!std::filesystem::exists(path))
    {
        std::println(std::cerr, "Failed to load {}", path.string());
        return false;
    }

    std::ifstream f(path);
    for (int x = 0; x < TILE_MAP_WIDTH; x++)
    {
        for (int y = 0; y < TILE_MAP_HEIGHT; y++)
        {
            int background = 0;
            int foreground = 0;

            f >> background >> foreground;
            if (background != empty_background_tile_)
            {
                set_tile({x, y}, static_cast<TileId>(background), SetTileAction::Place);
            }
            if (foreground != empty_foreground_tile_)
            {
                set_tile({x, y}, static_cast<TileId>(foreground), SetTileAction::Place);
            }
        }
    }
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
    texture_rect.position.x = variation * TEXTURE_SIZE;
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
            if (neighbour.id != empty_background_tile_ && neighbour.id != start_tile_id_ &&
                neighbour.id != finish_tile_id_)
            {
                variation += static_cast<int>(std::pow(2, i));
            }
        }
    }
    auto texture_rect = tile.texture_rect;
    texture_rect.position.x = variation * TEXTURE_SIZE;
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