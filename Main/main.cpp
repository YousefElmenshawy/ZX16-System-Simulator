// #include <iostream>
// #include<cstdlib>
// #include "ZX16_Simulator.h"
// using namespace std;
//
// int main(int argc, char* argv[]) {
//     std::string file;
//     if (argc < 2) {
//         std::cout << "No file path provided. Running default test program.\n";
//         std::string asmFile = "../TestCases/TC-ZX16-03.s";
//         std::string binFile = "../TestCases/TC-ZX16-03.bin";
//         // Assemble the default test case using the assembler script
//         // This assumes you are running the executable from: Main/cmake-build-debug/
//         std::string command = "python ../../assembler/zx16asm.py " + asmFile + " -o " + binFile;
//         int status = system(command.c_str());
//         if (status != 0) {
//             std::cerr << "Default assembler failed.\n";
//             return 1;
//         }
//         file=binFile;
//     }
//     else {
//         std::string asmFile = argv[1];  // e.g. "prog.asm"
//         std::string binFile = asmFile.substr(0, asmFile.find_last_of(".")) + ".bin";
//
//         std::string command = "python ../../assembler/zx16asm.py " + asmFile + " -o " + binFile;
//         int status = system(command.c_str());
//         if (status != 0) {
//             std::cerr << "Assembler failed.\n";
//             return 1;
//         }
//
//         file = binFile;
//     }
//
//     ZX16_Simulator sim;
//     sim.loadBinaryFile(file);    // Load the binary program file
//     sim.run();// run the simulator
//
//     return 0;
// }
#include <iostream>

#include "ZX16_Simulator.h"
#include "Graphics.h"
using namespace std;

int main(int argc, char* argv[]) {
    std::string file;
    if (argc < 2) {
        std::cout << "No file path provided. Running default test program.\n";

        //file = "TestCases/TC-ZX16-05.bin"; // Default test program file path
        file = "../TestCases/TC-ZX16-03.bin";

    } else {
        file = argv[1]; // Get the file path from the command-line argument
    }
    ZX16_Simulator sim;
    sim.loadBinaryFile(file);    // Load the binary program file
    GraphicsMemory Gmem;
    Gmem.setMemory(sim.getMemoryPtr());
    sim.run();// run the simulator

    Graphics graphics;
    int base = 0xF200 + 128; //tile 1
    uint8_t* mem = sim.getMemoryPtr();
    for (int row = 0; row < 16; ++row) {
        for (int col = 0; col < 8; ++col) {
            uint8_t even = (row + col) % 2 == 0 ? 1 : 2;
            uint8_t odd = (even == 1) ? 2 : 1;
            mem[base + row * 8 + col] = (odd << 4) | even;
        }
    }

    // Put tile 1 in tile map at position (5, 5)
    //mem[0xF000 + (5 * 20 + 5)] = 1;


    //Different testing for the colorsss

    //Gmem.setTileAt(5, 5, 7);// For testing to check the color and here it will appears a brown tile
  //  Gmem.setTileAt(5, 3, 15); //here another testing with another color
    // Example: create a pattern of color as an example
    /*for (int y = 0; y < 15; ++y) {
        for (int x = 0; x < 20; ++x) {
            uint8_t tileNum = (x + y) % 16;
            Gmem.setTileAt(x, y, tileNum);
        }
    }*/
    // Fill the whole screen with tile 1--, result is all boxes are brown
   /* for (int y = 0; y < 15; ++y) {
        for (int x = 0; x < 20; ++x) {
            Gmem.setTileAt(x, y, 1);
        }
    }*/
    // Fill color palette at 0xFA00 with real RGB values
    std::vector<uint8_t> palette = {
        0b00000000, // Black
        0b11100000, // Red
        0b00011100, // Green
        0b00000011, // Blue
        0b11111100, // Yellow
        0b11100011, // Magenta
        0b00011111, // Cyan
        0b11111111, // White
        0b10000000, // Dark red
        0b00100000, // Dark green
        0b00000010, // Dark blue
        0b11011000, // Orange
        0b01101100, // Light green
        0b00011111, // Light cyan
        0b11101110, // Light magenta
        0b10101010  // Weird gray
    };

    for (int i = 0; i < 16; ++i) {
        mem[0xFA00 + i] = palette[i];
    }
    // Fill tile definitions: each tile with one solid color
    for (int tile = 0; tile <= 15; ++tile) {
        int base = 0xF200 + tile * 128;
        for (int i = 0; i < 128; ++i) {
            mem[base + i] = (tile << 4) | tile;  // Set each pixel's color index to 'tile'
        }
    }
    for (int y = 0; y < 15; ++y) {
        for (int x = 0; x < 20; ++x) {
            Gmem.setTileAt(x, y, ((x + y) % 16) ); // test with tile 0
            // Gmem.setTileAt(x, y, ((x + y) % 15)+1 );  test with tile 1
        }
    }
    graphics.setmemory((&Gmem));
    graphics.run();  // Calls your SFML drawing window



  return 0;
}


