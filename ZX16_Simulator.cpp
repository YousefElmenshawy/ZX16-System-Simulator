//
// Created by youfi on 6/22/2025.
//

#include "ZX16_Simulator.h"
#include <iostream>
using namespace std;

ZX16_Simulator::ZX16_Simulator() {
    // Initialize members here
}


void ZX16_Simulator::loadBinaryFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        cerr<< "Failed to open file: " << filename << std::endl;
        return;
    }

    uint16_t value;
    while (file.read(reinterpret_cast<char*>(&value), sizeof(value))) {
        Instruction inst(value);
        program.push_back(inst);
    }

    // Use `data` as needed
}
void ZX16_Simulator::run ()
{
    pc = 0;

    /*while (pc < program.size()) {
        Instruction inst = program[pc];

        execute(inst);  // Run the logic of the instruction

        // If instruction doesn't modify PC directly (like jump), move to next
        if (!inst.changesPC) {
            pc++;
        }
    }*/ //later after decoding



}

