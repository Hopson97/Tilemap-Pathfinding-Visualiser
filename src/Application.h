#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>

#include "PathFinding/PathFindingAlgorithms.h"
#include "PathFinding/PathFindingCostGrid.h"
#include "PathFinding/PathfindingResultsState.h"
#include "TileMap.h"
#include "TileMapRenderer.h"

class Keyboard;
class TimeStep;

struct Camera
{
    sf::View view;
    sf::Vector2f velocity;

    float zoom_level = 0.5f;
};

struct EditorConfig
{
    TileId selected_tile = 0;
    sf::Vector2i brush_size = {1, 1};
};

struct PathFindingConfig
{
    bool draw_costs = false;
    bool draw_grid = false;
    bool visualiser_playing = false;

    int tickrate_searching = 50;
    int tickrate_pathing = 5;
};

using PathFindingAlgorithmFunction = PathFindingResult (*)(const PathFindingCostGrid&,
                                                           const sf::Vector2i&,
                                                           const sf::Vector2i&);
struct PathFindingAlgorithmOption
{
    PathFindingAlgorithmFunction algorithm;
    const char* name;
    AlgorithmType type;

    sf::Color visited_colour;
    sf::Color path_colour;

    bool is_added = false;

};

class Application
{
  public:
    Application(const sf::RenderWindow& window);

    void on_event(const sf::Event& e);
    void on_update(const Keyboard& keyboard, sf::Time dt);
    void on_fixed_update(sf::Time dt);
    void on_render(sf::RenderWindow& window);
    void on_gui(sf::RenderWindow& window, TimeStep& timestep, bool show_debug_info);

    void save_tile_maps();

  private:
    void set_tile_map_kind(TileMapKind kind);
    void set_selected_tile(TileId selection);
    void draw_editor_ui();
    void draw_pathfinding_ui(TimeStep& timestep);

    void display_add_or_remove_algorithm_gui();

    /// Gets the brush size, either will be the editor one or 1 if the tile is "special"
    sf::Vector2i get_brush_size();

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

    PathFindingCostGrid path_finding_grid_;
    PathFindingConfig path_finding_config_;

    bool is_playing_ = false;

    std::vector<PathFindingAlgorithmOption> pathfinding_algorithms_;

    // The actual result from the path finding algorithm
    std::vector<PathFindingResultsState> path_finding_results_;

    sf::Texture follower_texture_;
    sf::RectangleShape follower_sprite_;
};