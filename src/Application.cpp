#include "Application.h"

#include <print>

#include <SFML/Window/Event.hpp>
#include <imgui.h>

#include "Util/ImGuiExtension.h"
#include "Util/Keyboard.h"
#include "Util/Util.h"

namespace
{
    const std::array<sf::Vector2i, 4> TILE_OFFSETS = {sf::Vector2i{0, 1}, {-1, 0}, {1, 0}, {0, -1}};

    constexpr static float CAMERA_CAMERA_SPEED = 100.0f;
} // namespace

Application::Application(const sf::RenderWindow& window)
    : p_window(&window)
    , tile_map_side_view_("assets/TileMaps/side_view_tiles_config.json")
    , tile_map_top_view_("assets/TileMaps/top_view_tiles_config.json")
    , placement_preview_({TILE_SIZE, TILE_SIZE})
{
    camera_.view.setCenter(TILE_SIZE * TILE_MAP_WIDTH / 2 + TILE_SIZE / 2,
                           TILE_SIZE * TILE_MAP_HEIGHT / 2 + TILE_SIZE / 2);
    set_tile_map_kind(TileMapKind::SideView);
    placement_preview_.setFillColor({255, 255, 255, 128});

    // The side view map should be empty by default as the idea is to place platforms on the map
    tile_map_side_view_.tile_map.fill_map(tile_map_side_view_.tile_map.empty_tile_id());

    // The top-down view should be "full" by default.
    tile_map_top_view_.tile_map.fill_map(0);
    for (int y = 0; y < TILE_MAP_HEIGHT; y++)
    {
        for (int x = 0; x < TILE_MAP_WIDTH; x++)
        {
            tile_map_top_view_.renderer.set_tile_colour({x, y}, sf::Color::White);
        }
    }
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

    else if (e.type == sf::Event::MouseMoved)
    {
        auto tile_position = world_to_tile_position(
            p_window->mapPixelToCoords({e.mouseMove.x, e.mouseMove.y}, camera_.view));
        placement_preview_.setPosition(sf::Vector2f{tile_position} * TILE_SIZE);
    }
}

void Application::on_update(const Keyboard& keyboard, sf::Time dt)
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
    int CAMERA_SPEED = 15;
    sf::Vector2f movement;
    if (keyboard.is_key_down(sf::Keyboard::W))
    {
        movement.y -= CAMERA_SPEED;
    }
    else if (keyboard.is_key_down(sf::Keyboard::S))
    {
        movement.y += CAMERA_SPEED;
    }
    if (keyboard.is_key_down(sf::Keyboard::A))
    {
        movement.x -= CAMERA_SPEED;
    }
    else if (keyboard.is_key_down(sf::Keyboard::D))
    {
        movement.x += CAMERA_SPEED;
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
    assert(p_active_tile_map_);
    auto& tile_map = p_active_tile_map_->tile_map;

    // Show the GUI for selecting different tile types
    auto native_handle = tile_map.texture().getNativeHandle();
    ImTextureID imgui_id = (void*)(intptr_t)native_handle;
    if (ImGui::Begin("Tools"))
    {
        ImGui::Text("Select Tile");
        for (int i = 0; i < (int)p_active_tile_map_->tile_map.tile_type_count() - 1; i++)
        {
            if (i % 4 != 0)
            {
                ImGui::SameLine();
            }
            auto& tile = tile_map.get_tile(i);
            auto rect =
                tile.get_normalised_texture_rect(sf::Vector2f{tile_map.texture().getSize()});
            if (ImGui::ImageButton(tile.name.c_str(), imgui_id, {32, 32}, {rect.left, rect.top},
                                   {rect.width, rect.height}))
            {
                set_selected_tile(tile.id);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            {
                ImGui::SetTooltip("Tile: %s\nWeight: %d", tile.name.c_str(), tile.cost);
            }
        }
        ImGui::Separator();
        if (ImGui::RadioButton("Side View", tile_map_kind_ == TileMapKind::SideView))
        {
            set_tile_map_kind(TileMapKind::SideView);
        }

        if (ImGui::RadioButton("Top View", tile_map_kind_ == TileMapKind::TopDownView))
        {
            set_tile_map_kind(TileMapKind::TopDownView);
        }
    }
    ImGui::End();

    // Draw things relative to the camera view
    camera_.view.setSize(sf::Vector2f{window.getSize()} / camera_.zoom_level);
    window.setView(camera_.view);

    sf::RenderStates states;
    states.texture = &tile_map.texture();
    p_active_tile_map_->renderer.draw(window, states);

    // Draw grid
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::F2))
    {
        p_active_tile_map_->renderer.draw_grid(window);
    }

    // Draw the preview
    window.draw(placement_preview_);

    // Draw things relative to the window (Imgui)
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
    assert(p_active_tile_map_);
    p_active_tile_map_->tile_map.set_tile(tile_position, selected_tile_);
    p_active_tile_map_->renderer.set_tile_colour(tile_position, sf::Color::White);

    update_tile_variation(tile_position);

    for (int i = 0; i < TILE_OFFSETS.size(); i++)
    {
        update_tile_variation(TILE_OFFSETS[i] + tile_position);
    }
}

void Application::remove_tile(const sf::Vector2i& tile_position)
{
    assert(p_active_tile_map_);
    p_active_tile_map_->tile_map.set_tile(tile_position,
                                          p_active_tile_map_->tile_map.empty_tile_id());
    p_active_tile_map_->renderer.set_tile_colour(tile_position, sf::Color::Transparent);

    for (int i = 0; i < TILE_OFFSETS.size(); i++)
    {
        update_tile_variation(TILE_OFFSETS[i] + tile_position);
    }
}

void Application::update_tile_variation(const sf::Vector2i& tile_position)
{
    assert(p_active_tile_map_);
    auto& tile_map = p_active_tile_map_->tile_map;
    auto tile = p_active_tile_map_->tile_map.get_tile(tile_position);

    int variation = 0;
    if (tile.connect_to_neighbours)
    {
        for (int i = 0; i < TILE_OFFSETS.size(); i++)
        {
            auto neighbour = tile_map.get_tile(TILE_OFFSETS[i] + tile_position);
            if (neighbour.id != tile_map.tile_type_count() - 1)
            {
                variation += static_cast<int>(std::pow(2, i));
            }
        }
    }
    auto texture_rect = tile.texture_rect;
    texture_rect.left = variation * TEXTURE_SIZE;
    p_active_tile_map_->renderer.set_tile_texture_rect(tile_position, texture_rect);
}

void Application::set_tile_map_kind(TileMapKind map_kind)
{
    switch (map_kind)
    {
        // Set the current tile map to the "side view" version
        case TileMapKind::SideView:
            p_active_tile_map_ = &tile_map_side_view_;
            break;

        // Set the current tile map to the "top view" version
        case TileMapKind::TopDownView:
            p_active_tile_map_ = &tile_map_top_view_;
            break;

        default:
            break;
    }
    tile_map_kind_ = map_kind;

    for (int y = 0; y < TILE_MAP_HEIGHT; y++)
    {
        for (int x = 0; x < TILE_MAP_WIDTH; x++)
        {
            update_tile_variation({x, y});
        }
    }
    set_selected_tile(0);
}

void Application::set_selected_tile(TileId selection)
{
    assert(p_active_tile_map_);

    // Update selection
    selected_tile_ = selection;

    // Update the preview based on the new selection
    auto& tile_map = p_active_tile_map_->tile_map;

    placement_preview_.setTexture(&tile_map.texture());
    placement_preview_.setTextureRect(sf::IntRect{tile_map.get_tile(selected_tile_).texture_rect});
}
