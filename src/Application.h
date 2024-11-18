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

struct EditorConfig
{
    TileId selected_tile = 0;
    sf::Vector2i brush_size = {1, 1};
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
    void set_tile_map_kind(TileMapKind kind);
    void set_selected_tile(TileId selection);
    void draw_editor_ui();

  private:
    const sf::RenderWindow* p_window = nullptr;

    Camera camera_;

    TileMapKind tile_map_kind_ = TileMapKind::SideView;
    TileMap tile_map_side_view_;
    TileMap tile_map_top_view_;
    TileMap* p_active_tile_map_ = nullptr;

    sf::RectangleShape placement_preview_;
    TileMapGrid grid_;

    EditorConfig editor_config_;
};