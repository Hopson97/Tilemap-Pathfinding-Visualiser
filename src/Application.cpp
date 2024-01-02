#include "Application.h"

#include <SFML/Window/Event.hpp>

#include "Util/ImGuiExtension.h"

#include <imgui.h>

Application::Application(const sf::RenderWindow& window)
    : p_window(&window)
{
}

void Application::on_event(const sf::Event& e)
{
    if (e.type == sf::Event::MouseButtonReleased)
    {
        auto tile_position =
            world_to_tile_position(p_window->mapPixelToCoords({e.mouseButton.x, e.mouseButton.y}, camera_.view));
    }

    else if (e.type == sf::Event::MouseWheelScrolled)
    {
        camera_.zoom_level += e.mouseWheelScroll.delta / 15.0f;
        camera_.zoom_level = std::clamp(camera_.zoom_level, 0.5f, 5.0f);
    }
}

void Application::on_update(sf::Time dt)
{
    auto mouse = sf::Mouse::getPosition(*p_window);
    auto tile_position = world_to_tile_position(p_window->mapPixelToCoords(mouse, camera_.view));

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) )
    {
        tiles_.set_tile_colour(tile_position, selected_colour_);
    }
    else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
    {
        tiles_.set_tile_colour(tile_position, sf::Color::Transparent);
    }

    // Move camera
    int speed = 15;
    sf::Vector2f movement;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        movement.y -= speed;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        movement.y += speed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        movement.x -= speed;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        movement.x += speed;
    }
    camera_.velocity += movement;
    camera_.view.move(camera_.velocity * dt.asSeconds());
    camera_.velocity *= 0.95f;
}

void Application::on_fixed_update(sf::Time dt)
{
}

void Application::on_render(sf::RenderWindow& window)
{
    if (ImGui::Begin("Select Colour"))
    {
        // clang-format off
        if (ImGui::Button("Red")) { selected_colour_ = sf::Color::Red; }
        if (ImGui::Button("Green")) { selected_colour_ = sf::Color::Green; }
        if (ImGui::Button("Blue")) { selected_colour_ = sf::Color::Blue; }
        if (ImGui::Button("Black")) { selected_colour_ = sf::Color::Black; }
        // clang-format on
    }
    ImGui::End();

    // Set up camera
    camera_.view.setSize(sf::Vector2f{window.getSize()} / camera_.zoom_level);

    
    // Draw things relative to the camera view
    window.setView(camera_.view);
    tiles_.draw(window, sf::RenderStates::Default);
   // if (sf::Keyboard::isKeyPressed(sf::Keyboard::F2))
    {
        tiles_.draw_grid(window);
    }

    // Draw things relative to the window
    window.setView(window.getDefaultView());

    if (ImGui::Begin("Info"))
    {
        ImGui::TextSFMLVector2("Camera Position", camera_.view.getCenter());
        ImGui::Text("Camera Zoom: %f", camera_.zoom_level);
    }
    ImGui::End();
}
