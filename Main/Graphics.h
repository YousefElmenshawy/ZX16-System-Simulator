//
// Created by ASUS Zenbook on 7/2/2025.
//

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SFML/Graphics.hpp>
#include "GraphicsMemory.h"
class Graphics {
public:
    Graphics(int width = 320, int height = 240, int tileSize = 16);
    void setmemory(GraphicsMemory* memory);
    void run();  // Starts the main rendering loop
    void updateTilePixel(int tileIndex, int x, int y, uint8_t colorIndex);

    void draw(uint8_t* mem);
    void tick() {
        if (!window||!Gmemory) return;
        draw(Gmemory->getMemory());
        render();
    }
    bool isOpen() const {
        return (window&&window->isOpen());
    }

    ~Graphics() {
        delete window;
    }
    sf::RenderWindow* window = nullptr;
    void render();
    void processEvents();
private:
    GraphicsMemory* Gmemory = nullptr;
    sf::RectangleShape tile;
    sf::RectangleShape pixel;
    int tileSize;
    int tilesX, tilesY;

    // Add for keyboard input

    bool keyPressedFlag = false;
public:
    sf::Keyboard::Key lastKeyPressed = sf::Keyboard::Unknown;
    void drawPixel(int x, int y, uint8_t colorIndex);
    void drawTile(int tileX, int tileY, uint8_t tileIndex);
    // Add for keyboard input
    void resetKeyFlag() { keyPressedFlag = false; }
    bool hasKeyPressed() const { return keyPressedFlag; }
    sf::Keyboard::Key getLastKeyPressed() const { return lastKeyPressed; }
    void setLastKeyPressed(sf::Keyboard::Key key) { lastKeyPressed = key; keyPressedFlag = true; }
};

#endif // GRAPHICS_H