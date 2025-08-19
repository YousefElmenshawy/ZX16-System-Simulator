//
// Created by Beba on 7/3/2025.
//

#include "GraphicsMemory.h"
#include<iostream>
#include <sstream>
using namespace std;
// To provide the interface to the tile map buffer
// pointer to the main memory
void GraphicsMemory::setMemory(uint8_t* mem){
    memory = mem;
}
// write a tile number to the tile map buffer at position (x,y)
void GraphicsMemory::setTileAt(int x, int y, uint8_t tile) {
    if(!memory) return; // if memory is clean / not in the bound range
    if (x < 0 || x >= tilesPerRow || y < 0 || y >= tilesPerCol){
        std::cerr << "Tile position out of bounds: (" << x << ", " << y << ")\n";
        return;
    }
    // row major index calculations
    int index = y * tilesPerRow + x;
    memory[TILE_MAP_BASE + index] = tile; //to write the correct tile number for the location
}
//read the tile no.
uint8_t GraphicsMemory::getTileAt(int x, int y) const {
    if(!memory) return 0;
    if (x < 0 || x >= tilesPerRow || y < 0 || y >= tilesPerCol) {
        std::cerr << "Tile position out of bounds: (" << x << ", " << y << ")\n";
        return 0;
    }

    int index = y * tilesPerRow + x;  // row-major index
    return memory[TILE_MAP_BASE + index];  //return the number
}
// return the pointer to 128 byte tile definition for the tile index
const uint8_t* GraphicsMemory :: getTileDefinition(int tileIndex) const {
    if(!memory|| tileIndex < 0 || tileIndex >= TILE_COUNT) return 0;
    return &memory[TILE_DEFS_BASE + tileIndex * TILE_SIZE_BYTES];
}
std::vector<std::vector<uint8_t>> GraphicsMemory::decodeTile(const uint8_t* TileData) {
    std::vector<std::vector<uint8_t>> pixels(16, std::vector<uint8_t>(16));
    for(int y = 0; y < 16; y++) { //row
        for(int x = 0; x < 16; x+=2) {  //column
            int byteIndex = y * 8 + x/2;
            uint8_t byte = TileData[byteIndex];
            pixels[y][x] = byte & 0x0F;  //extract the lower 4 bits for the even pixels -->column
            pixels[y][x + 1] =(byte>> 4) &0x0F;  //extract the upper 4bits for odd -->column +1
        }
    }
    return pixels;
}

sf::Color GraphicsMemory:: colorPalette(uint8_t index) const {
    if(!memory || index>=16) return sf::Color::Black;
    uint8_t colorbyte = memory[0xFA00 + index];
    int R = ((colorbyte >> 5) & 0x07) * 255 / 7;  //3 bits for R-> 5,6,7
    int G = ((colorbyte >> 2) & 0x07) * 255 / 7;  // 3 bits for G -> 2,3,4
    int B = (colorbyte & 0x03) * 255 / 3;  //2 bits for B -> 0, 1
    return sf :: Color(R,G,B);
}
uint8_t* GraphicsMemory::getMemory() {
    return memory;
}
void GraphicsMemory::printTileMap() const {
    if (!memory) {
        std::cout << "No memory set.\n";
        return;
    }
    for (int y = 0; y < tilesPerCol; ++y) {
        for (int x = 0; x < tilesPerRow; ++x) {
            std::cout << std::hex << (int)getTileAt(x, y) << " ";
        }
        std::cout << std::endl;
    }
}
