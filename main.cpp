#include <iostream>

#include "ZX16_Simulator.h"
using namespace std;

uint16_t encodeRType(uint8_t func4, uint8_t rs2, uint8_t rd, uint8_t func3) {
    return (func4 << 12) | (rs2 << 9) | (rd << 6) | (func3 << 3) | 0b000;
}

void testRType(const std::string& label, uint8_t func4, uint8_t func3) {
    uint8_t rd = 1;   // ra
    uint8_t rs2 = 6;  // a0

    uint16_t raw = encodeRType(func4, rs2, rd, func3);
    Instruction inst(raw);
    inst.decode();

    std::cout << label << " (0x" << std::hex << raw << std::dec << "): ";
    std::cout << inst.AssemblyCode() << "\n";
}

int main() {
    // TIP Press <shortcut actionId="RenameElement"/> when your caret is at the
    // <b>lang</b> variable name to see how CLion can help you rename it.
    string file = "../sampleBINFILE-2instructions.bin";

    ZX16_Simulator Sim;

   // Sim.loadBinaryFile(file);// In ZX16_Simulator.cpp


    std::cout << "Testing All R-Type Instructions:\n";

    testRType("ADD",   0b0000, 0b000);
    testRType("SUB",   0b0001, 0b000);
    testRType("SLT",   0b0010, 0b001);
    testRType("SLTU",  0b0011, 0b010);
    testRType("SLL",   0b0100, 0b011);
    testRType("SRL",   0b0101, 0b011);
    testRType("SRA",   0b0110, 0b011);
    testRType("OR",    0b0111, 0b100);
    testRType("AND",   0b1000, 0b101);
    testRType("XOR",   0b1001, 0b110);
    testRType("MV",    0b1010, 0b111);
    testRType("JR",    0b1011, 0b000);  // rs2 ignored
    testRType("JALR",  0b1100, 0b000);
    return 0;
}

// TIP See CLion help at <a
// href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>.
//  Also, you can try interactive lessons for CLion by selecting
//  'Help | Learn IDE Features' from the main menu.