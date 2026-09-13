#include "PathFollower.h"

#include <print>

#include "Constants.h"

void PositionLerper::start_lerp(sf::Vector2f start, sf::Vector2f end, sf::Time duration)

{
    this->start = start;
    this->end = end;
    this->duration = duration;
    elapsed = sf::Time::Zero;
}

sf::Vector2f PositionLerper::lerp(sf::Time dt)
{
    elapsed += dt;
    auto t = elapsed.asSeconds() / duration.asSeconds();
    return {
        std::lerp(start.x, end.x, t),
        std::lerp(start.y, end.y, t),
    };
}

bool PositionLerper::is_done() const
{
    return elapsed >= duration;
}

PathFollower::PathFollower(sf::RectangleShape& sprite)
    : sprite_(&sprite)
{
}

void PathFollower::follow_path(const std::deque<PathfindingNode>& path)
{
    path_ = path;
    std::reverse(path_.begin(), path_.end());
    follower_position_ = front_position();
    lerper_ = PositionLerper{};
}

void PathFollower::update(sf::Time dt)
{
    follower_position_ = lerper_.lerp(dt);

    if (lerper_.is_done())
    {
        follower_position_ = lerper_.end;
        begin_next_move();
    }
}

void PathFollower::draw(sf::RenderWindow& window)
{
    sprite_->setPosition(follower_position_);
    window.draw(*sprite_);
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
