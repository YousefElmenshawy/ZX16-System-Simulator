//
// Created by Beba on 7/3/2025.
//

#ifndef GRAPHICSMEMORY_H
#define GRAPHICSMEMORY_H
#include <regex>
#include <SFML/Graphics/RenderWindow.hpp>

#include "ZX16_Simulator.h"
#include "Instruction.h"

#define TILE_MAP_BASE 0xF000
#define tilesPerCol 15
#define tilesPerRow 20
#define TILE_DEFS_BASE 0xF200
#define TILE_SIZE_BYTES 128
#define TILE_COUNT 16
using namespace std;

class GraphicsMemory {
private:
    uint8_t* memory = nullptr;

public:
    // For Tile map buffer
    void setMemory(uint8_t* mem);
    void setTileAt(int x, int y, uint8_t tile);
    uint8_t getTileAt(int x, int y) const;
    //For tile definition
    const uint8_t* getTileDefinition(int tileIndex) const;
    vector <vector<uint8_t>> decodeTile(const uint8_t* TileData);
    //For color Palette
    sf::Color colorPalette(uint8_t index) const;

};



#endif //GRAPHICSMEMORY_H