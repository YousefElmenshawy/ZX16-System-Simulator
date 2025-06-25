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

// U-Type encoding and test
uint16_t encodeUType(int16_t imm, uint8_t rd, bool flag) {
    int16_t shifted_imm = imm << 7;  // Shift to align with bits [15:7]

    uint16_t imm_high = (shifted_imm >> 10) & 0x3F; // Extract imm[15:10]
    uint16_t imm_mid  = (shifted_imm >> 7) & 0x7;   // Extract imm[9:7]

    return (flag << 15) |                   // [15] flag
           (imm_high << 9) |                // [14:9] imm[15:10]
           (rd << 6) |                      // [8:6] rd
           (imm_mid << 3) |                 // [5:3] imm[9:7]
           0b110;                        // [2:0] opcode
}

void testUType(const std::string &label, uint16_t imm12, uint8_t rd, bool flag) {
    uint16_t raw = encodeUType(imm12, rd, flag);
    Instruction inst(raw);
    inst.decode();
    std::cout << label << " (0x" << std::hex << raw << std::dec << "): ";
    std::cout << inst.AssemblyCode() << "\n";
}

// SYS-Type encoding and test
uint16_t encodeSysType(uint16_t svc) {
    return ((svc & 0x3FF) << 6) | (0b000 << 3) | 0b111;
}

void testSysType(const std::string& label, uint16_t svc) {
    uint16_t raw = encodeSysType(svc);
    Instruction inst(raw);
    inst.decode();
    std::cout << label << " (0x" << std::hex << raw << std::dec << "): ";
    std::cout << inst.AssemblyCode() << "\n";
}


uint16_t encodeBType(int8_t imm5, uint8_t rs2, uint8_t rs1, uint8_t func3) {
    uint8_t imm4to1 = (imm5 >> 1) & 0xF; // Right shift one for bits [4:1]
    uint16_t immPart = imm4to1 << 12;  // Shift to bits [15:12]
    return immPart | (rs2 << 9) | (rs1 << 6) | (func3 << 3) | 0b010;
}

void testBType(const std::string& label, int8_t imm5, uint8_t func3) {
    uint8_t rs1 = 1;  // ra
    uint8_t rs2 = 6;  // a0

    uint16_t raw = encodeBType(imm5, rs2, rs1, func3);
    Instruction inst(raw);
    inst.decode();

    std::cout << label << " (0x" << std::hex << raw << std::dec << "): ";
    std::cout << inst.AssemblyCode() << "\n";
}

uint16_t encodeSType(int8_t imm4, int8_t rs2, int8_t rs1, int8_t func3){
    uint16_t immpart = (imm4 & 0xF) << 12; // Mask to 4 bits and shift
    return immpart | (rs2 << 9) | (rs1 << 6) | (func3 << 3) | 0b011;
}
void testSType(const std::string& label, int8_t imm4, int8_t func3){
    uint8_t rs1 = 1;  // ra
    uint8_t rs2 = 3;  // x3

    uint16_t raw = encodeSType(imm4, rs2, rs1, func3);
    Instruction inst(raw);
    inst.decode();

    std::cout << label << " (0x" << std::hex << raw << std::dec << "): ";
    std::cout << inst.AssemblyCode() << "\n";
}



int main() {
    // TIP Press <shortcut actionId="RenameElement"/> when your caret is at the
    // <b>lang</b> variable name to see how CLion can help you rename it.
    string file = "../TestCase1.bin";

    ZX16_Simulator sim;

    // Load your binary program file here (make sure you have a .bin file)
    sim.loadBinaryFile(file);

    // Optionally print the disassembled program
    sim.printDisassembledProgram();

    // Run the simulation
    sim.run();

    // After running, dump the registers to see final state
    sim.dumpRegisters();



    return 0;
}

