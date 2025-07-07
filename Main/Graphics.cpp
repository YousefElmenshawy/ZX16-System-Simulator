//
// Created by ASUS Zenbook on 7/2/2025.
//

#include "Graphics.h"
#include "GraphicsMemory.h"
// initialize the graphics window & tile paremeters
Graphics::Graphics(int width, int height, int tileSize)
    : tileSize(tileSize)
{
    //calculate no. of tiles horizontally & vertically
    // Allocate a new window using a pointer
    window = new sf::RenderWindow(sf::VideoMode(width, height), "Tile Grid");
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
    while (window->isOpen()) {
        processEvents();  //handle window events
        render();  //draw the frame
    }
}
// window events
void Graphics::processEvents() {
    sf::Event event;
    while (window->pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window->close();  //for closing the window
    }
}
//renders the tile grid to the window

void Graphics::render() {
    window->clear();
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
                    window->draw(pixel);
                }
            }
        }
    }

    window->display();
}

//connect the memory to the graphics system
void Graphics::setmemory(GraphicsMemory* mem) {
    Gmemory = mem;
}
void Graphics::updateTilePixel(int tileIndex, int x, int y, uint8_t colorIndex) {
    int byteOffset = y * 8 + x / 2;
    uint8_t& byte = Gmemory->getMemory()[0xF200 + tileIndex * 128 + byteOffset];
    if (x % 2 == 0)
        byte = (byte & 0xF0) | (colorIndex & 0x0F);
    else
        byte = (byte & 0x0F) | ((colorIndex & 0x0F) << 4);
}
void Graphics::drawPixel(int x, int y, uint8_t colorIndex) {
    if (!Gmemory) return;

    sf::Color color = Gmemory->colorPalette(colorIndex & 0x0F);
    pixel.setFillColor(color);
    pixel.setPosition(static_cast<float>(x), static_cast<float>(y));
    window->draw(pixel);
}


void Graphics::drawTile(int tileX, int tileY, uint8_t tileIndex) {
    const int TILE_SIZE = 16; // 16x16 pixels
    const uint32_t TILE_DATA_BASE = 0xF200;
    const int BYTES_PER_TILE = 128;

    int screenX = tileX * TILE_SIZE;
    int screenY = tileY * TILE_SIZE;

    uint8_t* memory = Gmemory->getMemory();
    uint32_t tileAddress = TILE_DATA_BASE + tileIndex * BYTES_PER_TILE;

    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            int byteOffset = y * 8 + x / 2;
            uint8_t byte = memory[tileAddress + byteOffset];

            // Extract color index (4 bits)
            uint8_t colorIndex = (x % 2 == 0) ? (byte & 0x0F) : ((byte >> 4) & 0x0F);

            // Draw the pixel at (screenX + x, screenY + y)
            drawPixel(screenX + x, screenY + y, colorIndex);
        }
    }
}

void Graphics::draw(uint8_t* memory) {
    const uint16_t TILEMAP_BASE = 0xF000;
    const int TILEMAP_WIDTH = 20;
    const int TILEMAP_HEIGHT = 15;

    for (int y = 0; y < TILEMAP_HEIGHT; ++y) {
        for (int x = 0; x < TILEMAP_WIDTH; ++x) {
            uint8_t tileIndex = memory[TILEMAP_BASE + y * TILEMAP_WIDTH + x];
            drawTile(x, y, tileIndex);  // You must implement drawTile()
        }
    }
}
