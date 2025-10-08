#include "Application.h"

#include <print>

#include <SFML/Window/Event.hpp>
#include <imgui.h>

#include "Util/ImGuiExtension.h"
#include "Util/Keyboard.h"
#include "Util/TimeStep.h"
#include "Util/Util.h"

namespace
{
    constexpr static float CAMERA_CAMERA_SPEED = 100.0f;

    constexpr const char* DEFAULT_SIDE_VIEW_FILE = "./data/default_side_view_map.txt";
    constexpr const char* DEFAULT_TOP_VIEW_FILE = "./data/default_top_view_map.txt";
} // namespace

Application::Application(const sf::RenderWindow& window)
    : p_window(&window)
    , tile_map_side_view_("assets/TileMaps/side_view_tiles_config.json")
    , tile_map_top_view_("assets/TileMaps/top_view_tiles_config.json")
    , placement_preview_({TILE_SIZE, TILE_SIZE})
{
    camera_.view.setCenter({TILE_SIZE * TILE_MAP_WIDTH / 2 + TILE_SIZE / 2,
                            TILE_SIZE * TILE_MAP_HEIGHT / 2 + TILE_SIZE / 2});
    set_tile_map_kind(TileMapKind::TopDownView);
    placement_preview_.setFillColor({255, 255, 255, 128});

    tile_map_side_view_.load(DEFAULT_SIDE_VIEW_FILE);
    tile_map_top_view_.load(DEFAULT_TOP_VIEW_FILE);
}

void Application::on_event(const sf::Event& e)
{
    assert(p_active_tile_map_);
    auto& tile_map = *p_active_tile_map_;

    static bool is_mouse_down = false;
    static sf::Mouse::Button button_pressed;
    static sf::Vector2i current_tile_position;

    auto try_place_or_remove_tiles = [&]()
    {
        if (!ImGui::GetIO().WantCaptureMouse && !path_finding_config_.visualiser_playing)
        {
            auto brush_size = get_brush_size();
            if (is_mouse_down && button_pressed == sf::Mouse::Button::Left)
            {
                for (int y = 0; y < brush_size.y; y++)
                {
                    for (int x = 0; x < brush_size.x; x++)
                    {
                        tile_map.set_tile(current_tile_position + sf::Vector2i{x, y},
                                          editor_config_.selected_tile);
                        path_finding_config_.draw_costs = false;
                    }
                }
            }
            else if (is_mouse_down && button_pressed == sf::Mouse::Button::Right)
            {
                for (int y = 0; y < brush_size.y; y++)
                {
                    for (int x = 0; x < brush_size.x; x++)
                    {
                        tile_map.remove_tile(current_tile_position + sf::Vector2i{x, y});
                        path_finding_config_.draw_costs = false;
                    }
                }
            }
        }
    };

    if (auto mouse_scroll = e.getIf<sf::Event::MouseWheelScrolled>())
    {
        camera_.zoom_level += mouse_scroll->delta / 15.0f;
        camera_.zoom_level = std::clamp(camera_.zoom_level, 0.2f, 5.0f);
    }
    else if (auto mouse_pressed = e.getIf<sf::Event::MouseButtonPressed>())
    {
        is_mouse_down = true;
        button_pressed = mouse_pressed->button;
        current_tile_position = world_to_tile_position(
            p_window->mapPixelToCoords(mouse_pressed->position, camera_.view));
        try_place_or_remove_tiles();
    }
    else if (auto mouse_released = e.getIf<sf::Event::MouseButtonReleased>())
    {
        is_mouse_down = false;
    }
    else if (auto mouse_moved = e.getIf<sf::Event::MouseMoved>())
    {
        auto new_tile_position =
            world_to_tile_position(p_window->mapPixelToCoords(mouse_moved->position, camera_.view));

        if (new_tile_position != current_tile_position)
        {
            current_tile_position = new_tile_position;
            try_place_or_remove_tiles();
        }

        placement_preview_.setPosition(sf::Vector2f{current_tile_position} * TILE_SIZE);
    }
}

void Application::on_update(const Keyboard& keyboard, sf::Time dt)
{
    // Move camera
    float CAMERA_SPEED = 15.0f;
    sf::Vector2f movement;
    if (keyboard.is_key_down(sf::Keyboard::Key::W))
    {
        movement.y -= CAMERA_SPEED;
    }
    else if (keyboard.is_key_down(sf::Keyboard::Key::S))
    {
        movement.y += CAMERA_SPEED;
    }
    if (keyboard.is_key_down(sf::Keyboard::Key::A))
    {
        movement.x -= CAMERA_SPEED;
    }
    else if (keyboard.is_key_down(sf::Keyboard::Key::D))
    {
        movement.x += CAMERA_SPEED;
    }
    camera_.velocity += movement;
    camera_.view.move(camera_.velocity * dt.asSeconds());
    camera_.velocity *= 0.95f;
}

void Application::on_fixed_update([[maybe_unused]] sf::Time dt)
{
    if (path_finding_config_.visualiser_playing)
    {
        // Each update, highlight the currently visited node, FIFO from the pathing algorithm
        if (!path_finding_result_current_.visited.empty())
        {
            auto next = path_finding_result_current_.visited.front();
            path_finding_result_current_.visited.pop_front();
            path_finding_visualiser_.set_state(next, PathFindingState::Visited);
            visited_count_++;
        }
        else if (!final_path_.empty())
        {
            // Draw the path!
            auto next = final_path_.front();
            final_path_.pop_front();
            path_finding_visualiser_.set_state(next, PathFindingState::Path);
        }
    }
}

void Application::on_render(sf::RenderWindow& window)
{
    assert(p_active_tile_map_);
    auto& tile_map = *p_active_tile_map_;

    // Draw things relative to the camera view
    camera_.view.setSize(sf::Vector2f{window.getSize()} / camera_.zoom_level);
    window.setView(camera_.view);

    // Draw the active tile map itself
    tile_map.draw(window);

    // Draw the grid on-top
    if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F2))
    {
        grid_.draw(window);
    }

    // Draw the preview
    if (!ImGui::GetIO().WantCaptureMouse && !path_finding_config_.visualiser_playing)
    {
        auto current_preview_position = placement_preview_.getPosition();
        auto brush_size = get_brush_size();
        for (int y = 0; y < brush_size.y; y++)
        {
            for (int x = 0; x < brush_size.x; x++)
            {
                placement_preview_.setPosition(current_preview_position +
                                               sf::Vector2f{x * TILE_SIZE, y * TILE_SIZE});
                window.draw(placement_preview_);
            }
        }
        placement_preview_.setPosition(current_preview_position);
    }

    // Draw the pathfinding visualation
    if (path_finding_config_.draw_costs)
    {
        path_finding_grid_.draw(window);
    }

    // Draw the pathfinding result
    path_finding_visualiser_.draw(window);
}

void Application::on_gui(sf::RenderWindow& window, TimeStep& timestep, bool show_debug_info)
{
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
    draw_editor_ui();
    draw_pathfinding_ui(timestep);
}

void Application::save_tile_maps()
{
    tile_map_side_view_.save("./data/default_side_view_map.txt");
    tile_map_top_view_.save("./data/default_top_view_map.txt");
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
    set_selected_tile(0);

    path_finding_config_.draw_costs = false;
    path_finding_grid_.clear_all();
}

void Application::set_selected_tile(TileId selection)
{
    assert(p_active_tile_map_);

    // Update selection
    editor_config_.selected_tile = selection;

    // Update the preview based on the new selection
    auto& tile_map = *p_active_tile_map_;
    auto& tile_info = tile_map.tile_info(selection);
    auto texture_rect = tile_info.texture_rect;

    placement_preview_.setTexture(&tile_map.texture());
    placement_preview_.setTextureRect(sf::IntRect{texture_rect});
}

void Application::draw_editor_ui()
{

    assert(p_active_tile_map_);
    auto& tile_map = *p_active_tile_map_;

    auto tile_selection_ui = [&]()
    {
        auto native_handle = tile_map.texture().getNativeHandle();
        ImTextureID imgui_id = (ImTextureID)native_handle;
        ImGui::Text("Select Tile");
        for (int tile_id = 0; tile_id < (int)tile_map.tile_type_count() - 2; tile_id++)
        {
            if (tile_id % 4 != 0)
            {
                ImGui::SameLine();
            }
            auto& tile = tile_map.tile_info(tile_id);

            // Highlight the button if it is the selected one
            auto button_colour = tile.id == editor_config_.selected_tile
                                     ? ImVec4{0.8f, 0.8f, 0.8f, 0.8f}
                                     : ImVec4{0, 0, 0, 0};
            auto rect =
                tile.get_normalised_texture_rect(sf::Vector2f{tile_map.texture().getSize()});
            if (ImGui::ImageButton(tile.name.c_str(), imgui_id, {32, 32},
                                   {rect.position.x, rect.position.y}, {rect.size.x, rect.size.y},
                                   button_colour))
            {
                set_selected_tile(tile.id);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            {
                ImGui::SetTooltip("%s\n------------\nWeight: %s\nLayer: %s", tile.name.c_str(),
                                  tile.cost == -1 ? "N/A" : std::to_string(tile.cost).c_str(),
                                  [&]()
                                  {
                                      switch (tile.layer)
                                      {
                                          case TileType::Layer::Background:
                                              return "Background";
                                          case TileType::Layer::Foreground:
                                              return "Foreground";
                                          default:
                                              return "???";
                                      }
                                  }());
            }
        }
    };

    auto select_perspective_ui = [&]()
    {
        ImGui::Text("Select Perspective");

        if (ImGui::RadioButton("Top-Down View", tile_map_kind_ == TileMapKind::TopDownView))
        {
            set_tile_map_kind(TileMapKind::TopDownView);
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("Side-On View", tile_map_kind_ == TileMapKind::SideView))
        {
            set_tile_map_kind(TileMapKind::SideView);
        }
    };

    auto sliders_ui = [&]()
    {
        ImGui::Text("Select Tile Brush Size");
        ImGui::SliderInt("X Size", &editor_config_.brush_size.x, 1, 12);
        ImGui::SliderInt("Y Size", &editor_config_.brush_size.y, 1, 12);
    };

    if (ImGui::Begin("Tools"))
    {
        if (path_finding_config_.visualiser_playing)
        {
            ImGui::Text("Editing is disabled in pathfinding");
        }
        else
        {
            select_perspective_ui();
            ImGui::Separator();

            tile_selection_ui();
            ImGui::Separator();

            ImGui::Separator();
            sliders_ui();
        }
    }
    ImGui::End();
}

void Application::draw_pathfinding_ui(TimeStep& timestep)
{
    assert(p_active_tile_map_);
    auto& tile_map = *p_active_tile_map_;
    auto start = tile_map.start_position();
    auto finish = tile_map.finish_position();

    auto reset_visualiser = [&](const PathFindingResult& result)
    {
        path_finding_config_.draw_costs = false;
        path_finding_config_.visualiser_playing = true;

        path_finding_result_ = result;
        path_finding_result_current_ = result;
        visited_count_ = 0;

        path_finding_visualiser_.clear();
        final_path_ = result.create_path(*start, *finish);
    };

    if (ImGui::Begin("Path Finding"))
    {
        if (ImGui::Button("Show costs"))
        {
            path_finding_grid_.create_pathing_graph(tile_map, tile_map_kind_);
            path_finding_config_.draw_costs = true;
        }
        if (path_finding_config_.draw_costs && ImGui::Button("Hide costs"))
        {
            path_finding_config_.draw_costs = false;
        }

        ImGui::Separator();



        if (start && finish)
        {

            if (ImGui::Button("Breadth First Search"))
            {
                path_finding_grid_.create_pathing_graph(tile_map, tile_map_kind_);
                reset_visualiser(breadth_first_search(path_finding_grid_, *start, *finish));
            }
            /*
            if (ImGui::Button("Dijkstra's algorithm"))
            {
                path_finding_grid_.create_pathing_graph(tile_map, tile_map_kind_);
                reset_visualiser(dijkstra_algorithm(path_finding_grid_, *start, *finish));
            }*/
        }
        else
        {
            ImGui::Text("Please place a START and FINISH to");
            ImGui::Text("enable the pathfinding.");
        }
    }
    ImGui::End();

    if (ImGui::Begin("Player"))
    {
        int tick_rate = timestep.tick_rate();
        if (ImGui::SliderInt("Pathfinding Speed", &tick_rate, 1, 1000))
        {
            timestep.set_tick_rate(tick_rate);
        }

        if (path_finding_config_.visualiser_playing)
        {
            if (ImGui::Button("Stop"))
            {
                path_finding_config_.draw_costs = false;
                path_finding_config_.visualiser_playing = false;
                path_finding_visualiser_.clear();
            }

            ImGui::Text("%d/%d", visited_count_, (int)path_finding_result_.visited.size());
            ImGui::ProgressBar((float)visited_count_ / (float)path_finding_result_.visited.size());

            if (visited_count_ == path_finding_result_.visited.size())
            {
                ImGui::Text("Path Found: %s", path_finding_result_.finish_found ? "Yes" : "No");
            }
        }
        else
        {
            if (ImGui::Button("Clear"))
            {
                path_finding_config_.draw_costs = false;
                path_finding_visualiser_.clear();
            }
        }
    }
    ImGui::End();
}

sf::Vector2i Application::get_brush_size()
{
    assert(p_active_tile_map_);
    auto& tile_map = *p_active_tile_map_;
    return tile_map.tile_info(editor_config_.selected_tile).special != TileType::Special::No
               ? sf::Vector2i{1, 1}
               : editor_config_.brush_size;
}
