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
<<<<<<< Updated upstream
=======
        sim.loadBinaryFile("../RecursiveSum.s");
sim.dumpMemory(0x0000, 0x10000); // Dump memory for debugging
>>>>>>> Stashed changes
        return 1;
    }

    sim.loadBinaryFile(argv[1]); // Load binary file from args

    // Check for step mode argument
    if (argc >= 3 && std::string(argv[2]) == "step") {
<<<<<<< Updated upstream
        bool interactive = isatty(fileno(stdin));
        if (interactive) {
            std::string dummy;
            while (sim.step()) {
                std::getline(std::cin, dummy);
            }
            std::cout << "Simulation ended.\n";
            sim.dumpRegisters();
            sim.dumpMemory(0, 256);
        } else if (argc >= 4) {
            // Execute exactly N steps (N = argv[3])
            int steps_to_execute = std::stoi(argv[3]);
            bool running = true;
            for (int i = 0; i < steps_to_execute && running; i++) {
                running = sim.step();
            }
            if (!running) {
                std::cout << "Simulation ended.\n";
                sim.dumpRegisters();
                sim.dumpMemory(0, 256);
            }
        } else {
            // Just one step
            bool running = sim.step();
            if (!running) {
                std::cout << "Simulation ended.\n";
                sim.dumpRegisters();
                sim.dumpMemory(0, 256);
            }
        }
=======
        // Always run interactive step mode
        while (sim.step()) {
            int c = std::cin.get();
            if (c == EOF) break; // Exit if input is closed
            std::cout << std::flush;
        }
        std::cout << "Simulation ended.\n";
        sim.dumpRegisters();
        sim.dumpMemory(0, 0x10000);
>>>>>>> Stashed changes
    } else {
        // Normal run mode
        sim.run();
        std::cout << "Simulation ended.\n";
        sim.dumpRegisters();
<<<<<<< Updated upstream
        sim.dumpMemory(0, 256);
=======
        sim.dumpMemory(0, 0x10000);
>>>>>>> Stashed changes
    }
    return 0;
}
