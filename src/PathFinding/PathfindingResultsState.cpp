#include "PathfindingResultsState.h"

#include <numeric>

#include <imgui.h>

#include "PathFindingVisualiser.h"

namespace
{

    template <typename Stat, typename TotalType>
    void draw_progress_bar(const char* label, Stat so_far, TotalType total)
    {
        ImGui::Text(label, so_far, total);
        ImGui::ProgressBar(static_cast<float>(so_far) / static_cast<float>(total));
    }
} // namespace

PathFindingResultsState::PathFindingResultsState(const PathFindingResult& result)
    : path_finding_result_{result}
    , path_finding_result_current_{result}
{
    if (result.finish_found)
    {
        final_path_ = result.create_path();
        follower_.follow_path(final_path_);
    }

    stats_ = PathFindingStats{};
    stats_.total_path_length = final_path_.size();
    stats_.total_path_cost =
        std::accumulate(final_path_.begin(), final_path_.end(), 0,
                        [](auto sum, const auto& node) { return sum + node.cost; });
}

void PathFindingResultsState::on_update(sf::Time dt)
{

    if (visualisation_state_ == VisualisationState::Following)
    {
        follower_.update(dt);
        if (follower_.finished())
        {
            finished_current_stage_ = true;
        }
    }
}

void PathFindingResultsState::on_fixed_update(sf::Time dt)
{

    switch (visualisation_state_)
    {
        case VisualisationState::Searching:
            if (!path_finding_result_current_.visited.empty())
            {
                // Each update, highlight the currently visited node, FIFO from the pathing
                // algorithm
                auto next = path_finding_result_current_.visited.front();
                path_finding_result_current_.visited.pop_front();
                path_finding_visualiser_.set_state(next, PathFindingState::Visited);
                stats_.visited_count++;
            }
            else
            {
                finished_current_stage_ = true;
            }
            break;

        case VisualisationState::Pathing:
            if (!final_path_.empty() && path_finding_result_current_.finish_found)
            {
                // Draw the path!
                auto next = final_path_.front();
                final_path_.pop_front();
                path_finding_visualiser_.set_state(next.position, PathFindingState::Path);

                stats_.path_created_cost += next.cost;
                stats_.path_created_length++;
            }
            else
            {
                finished_current_stage_ = true;
            }
            break;

        default:
            break;
    }
}

void PathFindingResultsState::on_render(sf::RenderWindow& window)
{
    path_finding_visualiser_.draw(window);

    if (visualisation_state_ == VisualisationState::Following ||
        visualisation_state_ == VisualisationState::FollowingDone)
    {
        follower_.draw(window);
    }
}

void PathFindingResultsState::gui()
{

    draw_progress_bar("Search Progress: %d/%d tiles", stats_.visited_count,
                      path_finding_result_.visited.size());

    if (stats_.visited_count == path_finding_result_.visited.size())
    {
        ImGui::Text("Path Found: %s", path_finding_result_.finish_found ? "Yes" : "No");
        ImGui::Separator();
        draw_progress_bar("Creating path: %d/%d path tiles", stats_.path_created_length,
                          stats_.total_path_length);
        draw_progress_bar("Path cost so far: %d/%d", stats_.path_created_cost,
                          stats_.total_path_cost);
    }
}

bool PathFindingResultsState::has_finished_current() const
{
    return finished_current_stage_;
}

void PathFindingResultsState::begin_next_stage()
{
    if (finished_current_stage_)
    {
        switch (visualisation_state_)
        {
            case VisualisationState::Searching:
                visualisation_state_ = VisualisationState::Pathing;
                break;

            case VisualisationState::Pathing:
                visualisation_state_ = VisualisationState::Following;
                break;

            case VisualisationState::Following:
                visualisation_state_ = VisualisationState::FollowingDone;
                break;

            default:
                break;
        }

        finished_current_stage_ = false;
    }
}

const VisualisationState PathFindingResultsState::get_visualisation_state() const
{
    return visualisation_state_;
}
