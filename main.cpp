#include <iostream>

#include "ZX16_Simulator.h"
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
    sim.run();// run the simulator

    return 0;
}
