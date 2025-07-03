//
// Created by ASUS Zenbook on 7/2/2025.
//

#include "Graphics.h"
#include "GraphicsMemory.h"
// initialize the graphics window & tile paremeters
Graphics::Graphics(int width, int height, int tileSize)
    : window(sf::VideoMode(width, height), "Tile Grid"),
      tileSize(tileSize)
{
    //calculate no. of tiles horizontally & vertically
    tilesX = width / tileSize;
    tilesY = height / tileSize;
    // SFML rectangle shape for a single tile
    tile.setSize(sf::Vector2f(tileSize, tileSize));
    tile.setFillColor(sf::Color::Green);  //default color for the tiles
    tile.setOutlineColor(sf::Color::Black);  //outline color
    tile.setOutlineThickness(1);  //outline thickness
    pixel.setSize(sf::Vector2f(1, 1)); // Each pixel is drawn as 1x1 and positioned accordingly
}
// run the graphics window
void Graphics::run() {
    while (window.isOpen()) {
        processEvents();  //handle window events
        render();  //draw the frame
    }
}
// window events
void Graphics::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();  //for closing the window
    }
}
//renders the tile grid to the window

void Graphics::render() {
    window.clear();
    if (!Gmemory) return;

    for (int y = 0; y < tilesY; ++y) {
        for (int x = 0; x < tilesX; ++x) {
            uint8_t tileNumber = Gmemory->getTileAt(x, y);
            const uint8_t* tileData = Gmemory->getTileDefinition(tileNumber);
            if (!tileData) continue;

            auto pixels = Gmemory->decodeTile(tileData);

            for (int row = 0; row < 16; ++row) {
                for (int col = 0; col < 16; ++col) {
                    uint8_t colorIndex = pixels[row][col];
                    // color palette
                    sf::Color color = Gmemory->colorPalette(colorIndex);
                    pixel.setFillColor(color);
                    pixel.setPosition((x * 16) + col, (y * 16) + row);
                    window.draw(pixel);
                }
            }
        }
    }

    window.display();
}

//connect the memory to the graphics system
void Graphics::setmemory(GraphicsMemory* mem) {
    Gmemory = mem;
}

