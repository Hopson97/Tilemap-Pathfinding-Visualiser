#include "Application.h"

#include <SFML/Window/Event.hpp>

#include "Util/ImGuiExtension.h"

#include <imgui.h>

#include "Util/Util.h"
namespace
{
    const std::array<sf::Vector2i, 4> TILE_OFFSETS = {sf::Vector2i{0, 1}, {-1, 0}, {1, 0}, {0, -1}};
}

Application::Application(const sf::RenderWindow& window)
    : p_window(&window)
//, tile_map_(WIDTH, HEIGHT)
{
    camera_.view.setCenter(TILE_SIZE * WIDTH / 2 + TILE_SIZE / 2,
                           TILE_SIZE * HEIGHT / 2 + TILE_SIZE / 2);

    tile_map_texture_.loadFromFile("assets/Textures/Tiles.png");
}

void Application::on_event(const sf::Event& e)
{
    static bool mouse_down = false;
    if (e.type == sf::Event::MouseButtonReleased)
    {
        auto tile_position = world_to_tile_position(
            p_window->mapPixelToCoords({e.mouseButton.x, e.mouseButton.y}, camera_.view));
    }

    else if (e.type == sf::Event::MouseWheelScrolled)
    {
        camera_.zoom_level += e.mouseWheelScroll.delta / 15.0f;
        camera_.zoom_level = std::clamp(camera_.zoom_level, 0.2f, 5.0f);
    }
}

void Application::on_update(sf::Time dt)
{

    if (!ImGui::GetIO().WantCaptureMouse)
    {
        auto mouse = sf::Mouse::getPosition(*p_window);
        auto tile_position =
            world_to_tile_position(p_window->mapPixelToCoords(mouse, camera_.view));

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            set_tile_to_selected(tile_position);
        }
        else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
        {
            remove_tile(tile_position);
        }
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

void Application::on_render(sf::RenderWindow& window, bool show_debug_info)
{
    // Show the GUI for selecting different tile types
    auto native_handle = tile_map_texture_.getNativeHandle();
    ImTextureID imgui_id = (void*)(intptr_t)native_handle;
    if (ImGui::Begin("Select Tile"))
    {
        for (int i = 0; i < (int)TileType::Empty; i++)
        {
            if (i % 3 != 0)
            {
                ImGui::SameLine();
            }
            auto tile = tile_map_.tile_types[i];
            auto rect = tile.get_normalised_texture_rect(sf::Vector2f{tile_map_texture_.getSize()});

            if (ImGui::ImageButton(tile.name, imgui_id, {32, 32}, {rect.left, rect.top},
                                   {rect.width, rect.height}))
            {
                selected_tile = tile.type;
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            {
                ImGui::SetTooltip("Tile: %s\nWeight: %d", tile.name, tile.cost);
            }
        }
    }
    ImGui::End();

    // Set up camera
    camera_.view.setSize(sf::Vector2f{window.getSize()} / camera_.zoom_level);

    // Draw things relative to the camera view
    window.setView(camera_.view);

    sf::RenderStates states;
    states.texture = &tile_map_texture_;
    tilemap_renderer_.draw(window, states);

    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::F2))
    {
        tilemap_renderer_.draw_grid(window);
    }

    // Draw things relative to the window
    window.setView(window.getDefaultView());

    if (show_debug_info)
    {

        if (ImGui::Begin("Info"))
        {
            ImGui::TextSFMLVector2("Camera Position", camera_.view.getCenter());
            ImGui::Text("Camera Zoom: %f", camera_.zoom_level);
        }
        ImGui::End();
    }
}

void Application::set_tile_to_selected(const sf::Vector2i& tile_position)
{
    tile_map_.set_tile(tile_position, selected_tile);
    tilemap_renderer_.set_tile_colour(tile_position, sf::Color::White);

    update_tile_variation(tile_position);

    for (int i = 0; i < TILE_OFFSETS.size(); i++)
    {
        update_tile_variation(TILE_OFFSETS[i] + tile_position);
    }
}

void Application::remove_tile(const sf::Vector2i& tile_position)
{
    tile_map_.set_tile(tile_position, TileType::Empty);
    tilemap_renderer_.set_tile_colour(tile_position, sf::Color::Transparent);

    for (int i = 0; i < TILE_OFFSETS.size(); i++)
    {
        update_tile_variation(TILE_OFFSETS[i] + tile_position);
    }
}

void Application::update_tile_variation(const sf::Vector2i& tile_position)
{
    auto tile = tile_map_.get_tile(tile_position);

    int variation = 0;
    if (tile.connect_to_neighbours)
    {
        for (int i = 0; i < TILE_OFFSETS.size(); i++)
        {
            auto neighbour = tile_map_.get_tile(TILE_OFFSETS[i] + tile_position);
            if (neighbour.type != TileType::Empty)
            {
                variation += static_cast<int>(std::pow(2, i));
            }
        }
    }
    auto texture = tile.texture;
    texture.left = variation * TEXTURE_SIZE;
    tilemap_renderer_.set_tile_texture_rect(tile_position, texture);
}
