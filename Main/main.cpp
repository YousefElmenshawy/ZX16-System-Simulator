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
    sim.run();// run the simulator

    Graphics graphics;
    graphics.run();  // Calls your SFML drawing window

    return 0;
}
