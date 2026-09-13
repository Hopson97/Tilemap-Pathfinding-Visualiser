#pragma once

#include "../PathFollower.h"
#include "PathFindingAlgorithms.h"
#include "PathFindingVisualiser.h"

struct PathFindingStats
{
    // The number of tiles explored so far
    int visited_count = 0;

    // The cost of the path so far when creating the path
    int path_created_cost = 0;
    int path_created_length = 0;

    // The actual total cost of the path
    size_t total_path_length = 0;
    int total_path_cost = 0;
};

enum class VisualisationState
{
    Searching,
    Pathing,
    Following,
    FollowingDone,
};

struct PathFindingResultsState
{
  public:
    PathFindingResultsState(const PathFindingResult& result, sf::RectangleShape& follower_sprite);

    void update(sf::Time dt);
    void fixed_update(sf::Time dt);
    void render_tile_layers(sf::RenderWindow& window);
    void render_follower(sf::RenderWindow& window);

    void results_gui();
    void config_gui();

    bool has_finished_current() const;
    void begin_next_stage();

    const VisualisationState get_visualisation_state() const;
    const char* get_name() const;
    AlgorithmType get_type() const;

  private:
    // The actual result from the path finding algorithm
    PathFindingResult path_finding_result_;
    std::deque<PathfindingNode> final_path_;

    // A copy of the result that is used to update the visualiser using FIFO to remove as
    // visited/ pathing nodes are added to it.
    PathFindingResult path_finding_result_current_;

    PathFollower follower_;

    PathFindingStats stats_;
    PathFindingVisualiser path_finding_visualiser_;

    VisualisationState visualisation_state_ = VisualisationState::Searching;

    bool finished_current_stage_ = false;
};