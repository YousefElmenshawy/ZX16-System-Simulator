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

private:
    sf::RenderWindow window;
    sf::RectangleShape tile;
    sf::RectangleShape pixel;

    int tilesX;
    int tilesY;
    int tileSize;

    void processEvents();
    void render();

    GraphicsMemory* Gmemory = nullptr;

};

#endif // GRAPHICS_H
