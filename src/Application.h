#pragma once


#include <SFML/Graphics/RenderWindow.hpp>
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

    const sf::RenderWindow* p_window = nullptr;

    TileMapRenderer tilemap_renderer_;
    Camera camera_;


    TileMap tile_map_;
    sf::Texture tile_map_texture_;

    TileType selected_tile = TileType::Grass;
};