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
#include <thread>
using namespace std;

// Function to run the simulator loop
void runSimulator(ZX16_Simulator& sim) {
    sim.run();
}

// Function to run the graphics loop
void runGraphics(Graphics& graphics) {
    graphics.run();
}
//
// void draw(uint8_t* memory) {
//     const uint16_t BALL_DATA_ADDR = 0x8000;
//     const uint16_t TILEMAP_BASE = 0xF000;
//     const int TILEMAP_WIDTH = 20;
//     const uint8_t BALL_TILE = 1;
//
//     // Clear screen
//     for (int y = 0; y < 15; ++y) {
//         for (int x = 0; x < TILEMAP_WIDTH; ++x) {
//             memory[TILEMAP_BASE + y * TILEMAP_WIDTH + x] = 0;
//         }
//     }
//
//     // Read ball position
//     uint8_t x = memory[BALL_DATA_ADDR];
//     uint8_t y = memory[BALL_DATA_ADDR + 1];
//
//     // Draw ball if within bounds
//     if (x < TILEMAP_WIDTH && y < 15) {
//         memory[TILEMAP_BASE + y * TILEMAP_WIDTH + x] = BALL_TILE;
//     }
// }

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



    Graphics graphics;
    graphics.setmemory(&Gmem);

    uint8_t* mem = Gmem.getMemory();
    // for (int i = 0; i < 64; i += 2) {
    //     uint16_t inst = mem[i] | (mem[i+1] << 8);
    //     std::cout << "[" << std::hex << i << "] = 0x" << inst << std::endl;
    // }

    std::vector<uint8_t> palette = {
        0b00000000, // Black
        0b11111111, // White
        0b11100000, // Red
        0b00011100, // Green
        0b00000011, // Blue
        0b11111100, // Yellow
        0b11100011, // Magenta
        0b00011111, // Cyan
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

    for (int i = 0; i < 128; ++i) {
        mem[0xF200 + i] = (0 << 4) | 0;  // black tile
        mem[0xF200 + 1 * 128 + i] = (1 << 4) | 1;  // white tile
    }

    // thin vertical line tile: black background, vertical white line at x = 7
    int base = 0xF200 + 2 * 128;
    for (int row = 0; row < 16; ++row) {
        for (int col = 0; col < 8; ++col) {
            uint8_t color = (col == 3) ? 1 : 0;
            mem[base + row * 8 + col] = (color << 4) | color;
        }
    }

    for (int y = 0; y < 15; ++y) {
        for (int x = 0; x < 20; ++x) {
            Gmem.setTileAt(x, y, 0); // fill background black
        }
    }

    Gmem.setTileAt(12, 7, 1); // Ball

    Gmem.setTileAt(1, 6, 1);
    Gmem.setTileAt(1, 7, 1);
    Gmem.setTileAt(1, 8, 1);
    Gmem.setTileAt(18, 6, 1);
    Gmem.setTileAt(18, 7, 1);
    Gmem.setTileAt(18, 8, 1);

    // vertical thin white line in center of screen (x = 10)  (can be removed if we will do the version without walls )
    for (int y = 0; y < 15; ++y) {
        Gmem.setTileAt(10, y, 2);  // tile 2 = thin vertical white line
    }

    //graphics.run();

    //setting initial values for movement
    mem[0x8000] = 10;  // x
    mem[0x8001] = 7;   // y
    mem[0x8002] = 1;   // dx
    mem[0x8003] = 1;   // dy


    graphics.tick();
    sim.run();
    // Run in main thread — no need for thread////  (to test moving bal run this and comment simulation and graphics run)
    //sim.runInteractive(&graphics);


    // while (graphics.isOpen()) {
    //     if (!sim.halted) {
    //         sim.step();  // Execute one instruction from assembly code
    //     }
    //     graphics.draw(sim.getMemoryPtr());  // Draw the screen based on memory
    //     graphics.tick();     // Update the window
    //     std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    // }

    // while (graphics.isOpen()) {
    //     if (!sim.halted) {
    //         sim.step();  // Only step if not halted
    //         std::cout << "PC: 0x" << std::hex << sim.pc << " Instruction: 0x" << ((mem[sim.pc + 1] << 8) | mem[sim.pc]) << std::endl;
    //     }
    //     graphics.draw(mem);  // Draw the ball again from updated memory
    //     graphics.tick();
    //     std::cout << "Ball x: " << (int)mem[0x8000] << " y: " << (int)mem[0x8001] << std::endl;
    //     std::this_thread::sleep_for(std::chrono::milliseconds(50));
    // }

    // //=== Multithreaded simulation and graphics ===
    //  std::thread simThread([&]() {
    //      while (graphics.isOpen()) {
    //          sim.step(); // execute one instruction
    //          std::this_thread::sleep_for(std::chrono::milliseconds(1));
    //      }
    //  });
    //
    //  while (graphics.isOpen()) {
    //      graphics.draw(mem);                      // update tilemap based on memory
    //      graphics.tick();                // redraw screen
    //      std::cout << "Ball x: " << (int)mem[0x8000] << " y: " << (int)mem[0x8001] << std::endl;
    //      std::this_thread::sleep_for(std::chrono::milliseconds(16));
    //  }
    //
    //
    //  if (simThread.joinable())
    //      simThread.join();

  return 0;
}


