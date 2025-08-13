#include <iostream>
#include <cstdlib>
#include "ZX16_Simulator.h"
#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif
using namespace std;

int main(int argc, char* argv[]) {
    ZX16_Simulator sim;
    if (argc < 2) {
        std::cerr << "No .bin file provided.\n";
        sim.loadBinaryFile("../RecursiveSum.s");
sim.dumpMemory(0x0000, 0x10000); // Dump memory for debugging
        return 1;
    }

    sim.loadBinaryFile(argv[1]); // Load binary file from args

    // Check for step mode argument
    if (argc >= 3 && std::string(argv[2]) == "step") {
        // Always run interactive step mode
        while (sim.step()) {
            int c = std::cin.get();
            if (c == EOF) break; // Exit if input is closed
            std::cout << std::flush;
        }
        std::cout << "Simulation ended.\n";
        sim.dumpRegisters();
        sim.dumpMemory(0, 0x10000);
    } else {
        // Normal run mode
        sim.run();
        std::cout << "Simulation ended.\n";
        sim.dumpRegisters();
        sim.dumpMemory(0, 0x10000);
    }
    return 0;
}
