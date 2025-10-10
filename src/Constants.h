#pragma once
#include <array>

const std::array<sf::Vector2i, 4> TILE_OFFSETS = {sf::Vector2i{0, 1}, {-1, 0}, {1, 0}, {0, -1}};

constexpr float TILE_SIZE = 32.0f;
constexpr float TEXTURE_SIZE = 8.0f;

constexpr int TILE_MAP_WIDTH = 150;
constexpr int TILE_MAP_HEIGHT = 80;