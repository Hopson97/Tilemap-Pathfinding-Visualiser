#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>

#include "TileMap.h"

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
    void on_update(sf::Time dt);
    void on_fixed_update(sf::Time dt);
    void on_render(sf::RenderWindow& window);

  private:
    const sf::RenderWindow* p_window = nullptr;

    TileMap tiles_;
    Camera camera_;

    sf::Color selected_colour_ = sf::Color::Black;
};