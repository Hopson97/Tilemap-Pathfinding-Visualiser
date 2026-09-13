#pragma once

#include <deque>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Time.hpp>

#include "PathFinding/PathFindingAlgorithms.h"

struct PositionLerper
{
    sf::Vector2f start;
    sf::Vector2f end;

    sf::Time duration;
    sf::Time elapsed;

    void start_lerp(sf::Vector2f start, sf::Vector2f end, sf::Time duration);
    sf::Vector2f lerp(sf::Time dt);
    bool is_done() const;
};

class PathFollower
{
  public:
    PathFollower(sf::RectangleShape& sprite);

    void follow_path(const std::deque<PathfindingNode>& path);

    void update(sf::Time time);
    void draw(sf::RenderWindow& window);

    bool finished() const;

  private:
    void begin_next_move();

    sf::Vector2f front_position();
    sf::RectangleShape* sprite_ = nullptr;
    sf::Vector2f follower_position_{};
    std::deque<PathfindingNode> path_;

    PositionLerper lerper_;

    size_t current_ = 0;
};