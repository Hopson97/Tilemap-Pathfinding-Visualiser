#include "PathFollower.h"

#include "Constants.h"
#include <print>

PathFollower::PathFollower()
    : texture_{"assets/Textures/Character.png"}
    , sprite_({TILE_SIZE, TILE_SIZE * 2})
{
    sprite_.setTexture(&texture_);
}

void PathFollower::follow_path(const std::deque<sf::Vector2i>& path)
{
    path_ = path;
    std::reverse(path_.begin(), path_.end());
    sprite_.setPosition(sf::Vector2f{path_.front()} * TILE_SIZE - sf::Vector2f{0, TILE_SIZE});
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
        auto current = peek_next();
        path_.pop_front();
        if (!path_.empty())
        {
            auto next = peek_next();

            lerper_.start_lerp(current, next, sf::seconds(0.5));
        }

    }
}

sf::Vector2f PathFollower::peek_next()
{
    return sf::Vector2f{path_.front()} * TILE_SIZE - sf::Vector2f{0, TILE_SIZE};
}
