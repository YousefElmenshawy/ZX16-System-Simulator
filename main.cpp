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

uint16_t encodeIType(int8_t imm7, uint8_t rd, uint8_t func3) {
    uint16_t imm = imm7 & 0x7F;
    return (imm << 9) | (rd << 6) | (func3 << 3) | 0b001;
}

void testIType(const std::string& label, int8_t imm, uint8_t func3) {
    uint8_t rd = 6; // a0

    uint16_t raw = encodeIType(imm, rd, func3);
    Instruction inst(raw);
    inst.decode();

    std::cout << label << " (0x" << std::hex << raw << std::dec << "): ";
    std::cout << inst.AssemblyCode() << "\n";
}

uint16_t encodeLType(int8_t imm4, uint8_t rs2, uint8_t rd, uint8_t func3) {
    uint16_t immPart = (imm4 & 0xF) << 12;  // Mask to 4 bits and shift
    return immPart | (rs2 << 9) | (rd << 6) | (func3 << 3) | 0b100;
}

void testLType(const std::string& label, int8_t imm4, uint8_t func3) {
    uint8_t rd = 1;   // x1
    uint8_t rs2 = 2;  // x2

    uint16_t raw = encodeLType(imm4, rs2, rd, func3);
    Instruction inst(raw);
    inst.decode();

    std::cout << label << " (0x" << std::hex << raw << std::dec << "): ";
    std::cout << inst.AssemblyCode() << "\n";
}

uint16_t encodeJType(int16_t imm, uint8_t rd, bool link) {
    // Must be aligned (even number)
    imm &= 0x3FE;  // keep bits [9:1], force imm[0] = 0

    uint16_t imm_high = (imm >> 4) & 0x3F;  // bits [9:4]
    uint16_t imm_low  = (imm >> 1) & 0x7;   // bits [3:1]

    return (link << 15) |
           (imm_high << 9) |
           (rd << 6) |
           (imm_low << 3) |
           0b101;
}

void testJType(const std::string& label, int16_t imm, uint8_t rd, bool link) {
    uint16_t raw = encodeJType(imm, rd, link);
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


    /*std::cout << "Testing All R-Type Instructions:\n";

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

    std::cout << "Testing All I-Type Instructions:\n";*/

    testIType("ADDI",   -3, 0b000);
    testIType("SLTI",    5, 0b001);
    testIType("SLTUI",   7, 0b010);

    // Shift Instructions use imm7[6:4] to distinguish type
    // SLLI → imm7 = 0010000 (0x10), shift by 0
    testIType("SLLI",  0b0010000, 0b011);
    // SRLI → imm7 = 0100100 (0x24), shift by 4
    testIType("SRLI",  0b0100100, 0b011);
    // SRAI → imm7 = 1000011 (0x43), shift by 3
    testIType("SRAI",  0b1000011, 0b011);

    testIType("ORI",     8, 0b100);
    testIType("ANDI",   -2, 0b101);
    testIType("XORI",   12, 0b110);
    testIType("LI",     -69, 0b111);

    // L-Type Tests
    testLType("LB",   -3, 0b000);
    testLType("LW",    5, 0b001);
    testLType("LBU",   7, 0b010);

    // J-Type Tests
    testJType("J",     12, 0, false);
    testJType("J",    -16, 0, false);
    testJType("JAL",   10, 1, true);
    testJType("JAL",  -14, 2, true);

    return 0;
}

// TIP See CLion help at <a
// href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>.
//  Also, you can try interactive lessons for CLion by selecting
//  'Help | Learn IDE Features' from the main menu.