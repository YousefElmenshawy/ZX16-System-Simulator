//
// Created by ASUS Zenbook on 7/2/2025.
//

#include "Graphics.h"

Graphics::Graphics(int width, int height, int tileSize)
    : window(sf::VideoMode(width, height), "Tile Grid"),
      tileSize(tileSize)
{
    tilesX = width / tileSize;
    tilesY = height / tileSize;

    tile.setSize(sf::Vector2f(tileSize, tileSize));
    tile.setFillColor(sf::Color::Green);
    tile.setOutlineColor(sf::Color::Black);
    tile.setOutlineThickness(1);
}

void Graphics::run() {
    while (window.isOpen()) {
        processEvents();
        render();
    }
}

void Graphics::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
    }
}

void Graphics::render() {
    window.clear();
    for (int y = 0; y < tilesY; ++y) {
        for (int x = 0; x < tilesX; ++x) {
            tile.setPosition(x * tileSize, y * tileSize);
            window.draw(tile);
        }
    }
    window.display();
}
