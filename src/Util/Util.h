#pragma once

#include <iostream>

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

template <typename T>
std::ostream& operator<<(std::ostream& stream, const sf::Vector2<T>& vector)
{
    stream << '(' << vector.x << ", " << vector.y << ')';
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, sf::Color colour)
{
    int red = static_cast<int>(colour.r);
    int green = static_cast<int>(colour.g);
    int blue = static_cast<int>(colour.b);
    int alpha = static_cast<int>(colour.a);
    stream << "R: " << red << " G: " << green << " B: " << blue << " A: " << alpha;
    return stream;
}