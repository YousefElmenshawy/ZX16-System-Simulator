#include <iostream>

#include "ZX16_Simulator.h"
using namespace std;

int main(int argc, char* argv[]) {
    std::string file;
    if (argc < 2) {
        std::cout << "No file path provided. Running default test program.\n";

        file = "TestCases/TC-ZX16-05.bin"; // Default test program file path

    } else {
        file = argv[1]; // Get the file path from the command-line argument
    }
    ZX16_Simulator sim;

    // Load the binary program file

    sim.loadBinaryFile(file);
    sim.dumpRegisters();

    // Optionally print the disassembled program
    sim.printDisassembledProgram();

    // Run the simulation
    sim.run();

    // After running, dump the registers to see the final state
    sim.dumpRegisters();

    return 0;
}
