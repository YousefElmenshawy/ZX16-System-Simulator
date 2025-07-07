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

    void drawPixel(int x, int y, uint8_t colorIndex);


    void drawTile(int tileX, int tileY, uint8_t tileIndex);
    void draw(uint8_t* mem);
    void tick() {
        if (!window||!Gmemory) return;
        //draw(Gmemory->getMemory());
        render();
    }
    bool isOpen() const {
        return (window&&window->isOpen());
    }
    void render();
    ~Graphics() {
        delete window;
    }
    sf::RenderWindow* window = nullptr;
private:

    sf::RectangleShape tile;
    sf::RectangleShape pixel;

    int tilesX;
    int tilesY;
    int tileSize;

    void processEvents();


    GraphicsMemory* Gmemory = nullptr;

};

#endif // GRAPHICS_H
