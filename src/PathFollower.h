#pragma once

#include <deque>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Time.hpp>

struct PositionLerper
{
    sf::Vector2f start;
    sf::Vector2f end;

    sf::Time duration;
    sf::Time elapsed;

    void start_lerp(sf::Vector2f start, sf::Vector2f end, sf::Time duration)
    {
        this->start = start;
        this->end = end;
        this->duration = duration;

        elapsed = sf::Time::Zero;
    }

    sf::Vector2f lerp(sf::Time dt)
    {
        elapsed += dt;
        auto t = elapsed.asSeconds() / duration.asSeconds();
        return {
            std::lerp(start.x, end.x, t),
            std::lerp(start.y, end.y, t),
        };
    }

    bool done() const
    {
        return elapsed >= duration;
    }
};

class PathFollower
{
  public:
    PathFollower();

    void follow_path(const std::deque<sf::Vector2i>& path);

    void update(sf::Time time);
    void draw(sf::RenderWindow& window);

    bool finished() const;

  private:
    void begin_next_move();

    sf::Vector2f peek_next();

    sf::Texture texture_;
    sf::RectangleShape sprite_;
    std::deque<sf::Vector2i> path_;

    PositionLerper lerper_;

    size_t current_ = 0;
};