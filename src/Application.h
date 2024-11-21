#pragma once


#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>

#include "TileMapRenderer.h"
#include "TileMap.h"

#include "PathFinding/PathfindingCostGrid.h"
#include "PathFinding/PathFindingVisualiser.h"
#include "PathFinding/PathfindingAlgorithms.h"

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

struct PathfindingConfig
{
    bool draw_costs = false;
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
    void draw_pathfinding_ui();
    
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
    PathfindingConfig path_finding_config_;
    PathFindingVisualiser path_finding_visualisor_;

    // The actual result from the pathfinding algorithm
    PathFindingResult path_finding_result_;

    // A copy of the result that is used to update the visuliser using FIFO to remove as 
    // visited/ pathing nodes are added to it.
    PathFindingResult path_finding_result_current_;

    int visisted_ = 0;

    bool play_visualiser_ = false;
};