#include "Application.h"

#include <numeric>
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

    , follower_texture_{"assets/Textures/Character.png"}
    , follower_sprite_({TILE_SIZE, TILE_SIZE * 2})
{
    follower_sprite_.setTexture(&follower_texture_);
    camera_.view.setCenter({TILE_SIZE * TILE_MAP_WIDTH / 2 + TILE_SIZE / 2,
                            TILE_SIZE * TILE_MAP_HEIGHT / 2 + TILE_SIZE / 2});
    set_tile_map_kind(TileMapKind::TopDownView);
    placement_preview_.setFillColor({255, 255, 255, 128});

    tile_map_side_view_.load(DEFAULT_SIDE_VIEW_FILE);
    tile_map_top_view_.load(DEFAULT_TOP_VIEW_FILE);

    pathfinding_algorithms_ = {
        PathFindingAlgorithmOption{
            .algorithm = breadth_first_search,
            .name = "Breadth First Search",
            .type = AlgorithmType::BreadthFirstSearch,
            .visited_colour = {50, 100, 255, 128},
            .path_colour = {0, 255, 255, 128},
        },
        PathFindingAlgorithmOption{
            .algorithm = dijkstra_algorithm,
            .name = "Dijkstra's Algorithm",
            .type = AlgorithmType::DijkstrasAlgorithm,
            .visited_colour = {200, 205, 155, 128},
            .path_colour = {0, 128, 255, 128},
        },
    };
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
                    }
                }
                path_finding_grid_.create_pathing_graph(tile_map, tile_map_kind_);
            }
            else if (is_mouse_down && button_pressed == sf::Mouse::Button::Right)
            {
                for (int y = 0; y < brush_size.y; y++)
                {
                    for (int x = 0; x < brush_size.x; x++)
                    {
                        tile_map.remove_tile(current_tile_position + sf::Vector2i{x, y});
                    }
                }
                path_finding_grid_.create_pathing_graph(tile_map, tile_map_kind_);
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

    for (auto& result : path_finding_results_)
    {
        result.update(dt);
    }
}

void Application::on_fixed_update(sf::Time dt)
{
    if (!path_finding_config_.visualiser_playing)
    {
        return;
    }

    // To make the comparision work with the tick rate, and for better comparision, all added stages
    // must complete before starting the next.
    bool all_ready_for_next_stage = true;
    for (auto& result : path_finding_results_)
    {
        result.fixed_update(dt);

        if (!result.has_finished_current())
        {
            all_ready_for_next_stage = false;
        }
    }

    if (all_ready_for_next_stage)
    {
        for (auto& result : path_finding_results_)
        {
            result.begin_next_stage();
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
    if (!path_finding_config_.draw_grid)
    {
        grid_.draw(window);
    }

    // Draw the preview for the user's selected tile.
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

    if (path_finding_config_.draw_costs)
    {
        path_finding_grid_.draw(window);
    }

    // Ensure the followers are rendered on top of the visualisation grid
    for (auto& result : path_finding_results_)
    {
        result.draw_visited(window);
    }
    for (auto& result : path_finding_results_)
    {
        result.draw_path(window);
    }
    for (auto& result : path_finding_results_)
    {
        result.draw_follower(window);
    }
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

    path_finding_grid_.create_pathing_graph(*p_active_tile_map_, tile_map_kind_);
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
        ImGui::SliderInt("Brush Width", &editor_config_.brush_size.x, 1, 12);
        ImGui::SliderInt("Brush Height", &editor_config_.brush_size.y, 1, 12);
    };

    if (!path_finding_config_.visualiser_playing)
    {
        if (ImGui::Begin("Tools"))
        {
            select_perspective_ui();
            ImGui::Separator();

            tile_selection_ui();
            ImGui::Separator();

            ImGui::Separator();
            sliders_ui();

            ImGui::Separator();
            ImGui::Text("Left Click -> Place tile");
            ImGui::Text("Right Click -> Remove tile");
        }
        ImGui::End();
    }
}

void Application::draw_pathfinding_ui(TimeStep& timestep)
{
    assert(p_active_tile_map_);
    auto& tile_map = *p_active_tile_map_;
    auto start = tile_map.start_position();
    auto finish = tile_map.finish_position();

    if (!path_finding_config_.visualiser_playing)
    {
        if (ImGui::Begin("Pathfinding Algorithm Selector"))
        {
            if (start && finish)
            {
                display_add_or_remove_algorithm_gui();
            }
            else
            {
                ImGui::Text("Please place a START and FINISH to");
                ImGui::Text("enable the pathfinding.");
            }
        }
        ImGui::End();
    }

    if (path_finding_config_.visualiser_playing)
    {
        if (ImGui::Begin("Pathing Controls"))
        {
            ImGui::Text("Speeds");
            ImGui::SliderInt("Searching", &path_finding_config_.tickrate_searching, 1, 1000);
            ImGui::SliderInt("Pathing", &path_finding_config_.tickrate_pathing, 1, 1000);
            // ImGui::SliderInt("Following Speed", &path_finding_config_.tickrate_following, 1,
            // 1000);

            if (path_finding_config_.visualiser_playing)
            {
                if (ImGui::Button("Stop"))
                {
                    path_finding_config_.visualiser_playing = false;
                    path_finding_results_.clear();
                    for (auto& option : pathfinding_algorithms_)
                    {
                        option.is_added = false;
                    }
                }

                for (auto& result : path_finding_results_)
                {
                    result.results_gui();
                }
            }
        }
        ImGui::End();
    }

    if (ImGui::Begin("Config"))
    {
        if (ImGui::Checkbox("Show/Hide costs", &path_finding_config_.draw_costs))
        {
            path_finding_grid_.create_pathing_graph(tile_map, tile_map_kind_);
        }

        ImGui::Checkbox("Show/Hide Grid", &path_finding_config_.draw_grid);

        for (auto& result : path_finding_results_)
        {
            result.config_gui();
        }
    }
    ImGui::End();

    // Ensure the tick rate is set to whatever stage is still running
    auto lowest_state = VisualisationState::Pathing;
    for (auto& result : path_finding_results_)
    {
        lowest_state = static_cast<VisualisationState>(std::min(
            static_cast<int>(lowest_state), static_cast<int>(result.get_visualisation_state())));
    }
    switch (lowest_state)
    {
        case VisualisationState::Searching:
            timestep.set_tick_rate(path_finding_config_.tickrate_searching);
            break;

        case VisualisationState::Pathing:
            timestep.set_tick_rate(path_finding_config_.tickrate_pathing);
            break;

        default:
            // timestep.set_tick_rate(path_finding_config_.tickrate_following);
            break;
    }
}

void Application::display_add_or_remove_algorithm_gui()
{
    auto start = p_active_tile_map_->start_position();
    auto finish = p_active_tile_map_->finish_position();

    // GUI to add a algorithm to the comparison. Clicking the buttons runs the algorithms
    // immediately via the function pointer.
    for (auto& option : pathfinding_algorithms_)
    {
        if (!option.is_added && ImGui::Button(std::format("Add {}", option.name).c_str()))
        {
            option.is_added = true;
            path_finding_grid_.create_pathing_graph(*p_active_tile_map_, tile_map_kind_);
            auto& result_state = path_finding_results_.emplace_back(
                option.algorithm(path_finding_grid_, *start, *finish), follower_sprite_);
            result_state.get_visual_config().path_colour = option.path_colour;
            result_state.get_visual_config().visited_colour = option.visited_colour;
        }
    }

    ImGui::Separator();

    // GUI to remove them from the comparison
    if (!path_finding_results_.empty())
    {
        ImGui::Text("Current Queued");
        for (auto& option : pathfinding_algorithms_)
        {
            if (option.is_added)
            {
                ImGui::Text("%s", option.name);
                ImGui::SameLine();
                ImGui::PushID(option.name);
                if (ImGui::Button("-"))
                {
                    option.is_added = false;

                    for (auto itr = path_finding_results_.begin();
                         itr != path_finding_results_.end();)
                    {
                        if (option.type == itr->get_type())
                        {
                            itr = path_finding_results_.erase(itr);
                        }
                        else
                        {
                            ++itr;
                        }
                    }
                }
                ImGui::PopID();
            }
        }

        if (ImGui::Button("Start"))
        {
            path_finding_config_.visualiser_playing = true;
        }
    }
}

sf::Vector2i Application::get_brush_size()
{
    assert(p_active_tile_map_);
    auto& tile_map = *p_active_tile_map_;
    return tile_map.tile_info(editor_config_.selected_tile).special != TileType::Special::No
               ? sf::Vector2i{1, 1}
               : editor_config_.brush_size;
}
