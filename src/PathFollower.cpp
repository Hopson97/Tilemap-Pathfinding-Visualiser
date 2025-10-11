#include "PathFollower.h"

#include "Constants.h"
#include <print>

PathFollower::PathFollower()
    : texture_{"assets/Textures/Character.png"}
    , sprite_({TILE_SIZE, TILE_SIZE * 2})
{
    sprite_.setTexture(&texture_);
}

void PathFollower::follow_path(const std::deque<PathfindingNode>& path)
{
    path_ = path;
    std::reverse(path_.begin(), path_.end());
    auto pos = front_position();
    std::println("Setting sprtie position {} {}", pos.x, pos.y);
    sprite_.setPosition(front_position());
    lerper_ = PositionLerper{};
}

void PathFollower::update(sf::Time dt)
{
    sprite_.setPosition(lerper_.lerp(dt));

    if (lerper_.done())
    {
        sprite_.setPosition(lerper_.end);
        begin_next_move();
    }
}

void PathFollower::draw(sf::RenderWindow& window)
{
    window.draw(sprite_);
}

bool PathFollower::finished() const
{
    return path_.empty();
}

void PathFollower::begin_next_move()
{
    if (path_.size() >= 1)
    {
        auto current = front_position();
        path_.pop_front();
        if (!path_.empty())
        {
            auto next = front_position();
            auto cost = path_.front().cost;



            lerper_.start_lerp(current, next, sf::milliseconds(std::sqrt(cost * 4) * 45));
        }

    }
}

sf::Vector2f PathFollower::front_position()
{
    return sf::Vector2f{path_.front().position} * TILE_SIZE - sf::Vector2f{0, TILE_SIZE};
}
