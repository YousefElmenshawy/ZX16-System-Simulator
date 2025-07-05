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
        return 1;
    }

    sim.loadBinaryFile(argv[1]); // Load binary file from args

    // Check for step mode argument
    if (argc >= 3 && std::string(argv[2]) == "step") {
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
    } else {
        // Normal run mode
        sim.run();
        std::cout << "Simulation ended.\n";
        sim.dumpRegisters();
        sim.dumpMemory(0, 256);
    }
    return 0;
}
