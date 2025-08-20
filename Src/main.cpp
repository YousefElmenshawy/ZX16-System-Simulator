#include <iostream>
#include <cstdlib>
#include "ZX16_Simulator.h"
#include "Graphics.h"
#include <thread>
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

    // Require a binary path; backend always provides it
    if (argc < 2) {
       /* GraphicsMemory Gmem;
        Gmem.setMemory(sim.getMemoryPtr());

        Graphics gfx;
        gfx.runInteractive(&gfx);
        std::cout << "Simulation ended.\n";
        sim.dumpRegisters();
        sim.dumpMemory(0, 0x10000);*/
        return 1;
    }

    // Load program into simulator memory
    sim.loadBinaryFile(argv[1]);

    // Modes: step | interactive | default run
    if (argc >= 3 && std::string(argv[2]) == "step") {
        // Step mode: drive via stdin newlines
        while (sim.step()) {
            int c = std::cin.get();
            if (c == EOF) break;
            std::cout << std::flush;
            sim.dumpRegisters();
            sim.dumpMemory(0, 0x10000);
        }
        std::cout << "Simulation ended.\n";
        sim.dumpRegisters();
        sim.dumpMemory(0, 0x10000);
    } else if (argc >= 3 && std::string(argv[2]) == "interactive") {
        // Graphics interactive mode: hook graphics memory first
        GraphicsMemory gmem;
        gmem.setMemory(sim.getMemoryPtr());

        Graphics gfx;
        gfx.setmemory(&gmem);

        sim.runInteractive(&gfx);
        std::cout << "Simulation ended.\n";
        sim.dumpRegisters();
        sim.dumpMemory(0, 0x10000);
    } else {
        // Normal non-graphics run
        sim.run();
        std::cout << "Simulation ended.\n";
        sim.dumpRegisters();
        sim.dumpMemory(0, 0x10000);
    }

    return 0;
}
