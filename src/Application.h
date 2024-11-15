#pragma once


#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>

#include "TileMapRenderer.h"
#include "TileMap.h"

class Keyboard;

struct Camera
{
    sf::View view;
    sf::Vector2f velocity;

    float zoom_level = 1.0f;
};

struct TileMapWrapper
{
    TileMapWrapper(const std::filesystem::path& tile_config)
        : tile_map(tile_config)
    {
    }

    TileMap tile_map;
    TileMapRenderer renderer;
};

class Application
{
  public:
    Application(const sf::RenderWindow& window);

    void on_event(const sf::Event& e);
    void on_update(const Keyboard& keyboard, sf::Time dt);
    void on_fixed_update(sf::Time dt);
    void on_render(sf::RenderWindow& window, bool show_debug_info);

  private:
    void set_tile_to_selected(const sf::Vector2i& tile_position);
    void remove_tile(const sf::Vector2i& tile_position);
    void update_tile_variation(const sf::Vector2i& tile_position);

    void set_tile_map_kind(TileMapKind kind);

    void set_selected_tile(TileId selection);

  private:
    const sf::RenderWindow* p_window = nullptr;

    Camera camera_;

    TileMapKind tile_map_kind_ = TileMapKind::SideView;
    TileMapWrapper tile_map_side_view_;
    TileMapWrapper tile_map_top_view_;
    TileMapWrapper* p_active_tile_map_ = nullptr;

    TileId selected_tile_ = 0;

    sf::RectangleShape placement_preview_;
};