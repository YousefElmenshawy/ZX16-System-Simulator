
#ifndef ZX16_SIMULATOR_H
#define ZX16_SIMULATOR_H



#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include "Instruction.h"

class ZX16_Simulator {
private:
    static const int MEMORY_SIZE = 65536;   // 64KB addressable memory
    static const int NUM_REGISTERS = 8;//8 Registers

    uint8_t memory[MEMORY_SIZE];           // Main memory
    int16_t registers[NUM_REGISTERS];      // General-purpose registers

    uint16_t pc;                            // Program counter
    uint16_t programEnd;                    // address of the Program Ending at Memory
    bool running;                           // Execution state

    std::vector<Instruction> program;       // Loaded program

    bool executeInstruction(const Instruction& inst);
    void handleEcall();

public:
    ZX16_Simulator();
    void loadBinaryFile(const std::string& filename);
    void run();
    void dumpRegisters() const;
    void dumpMemory(uint16_t address, uint16_t size) const;

    // For testing/debug
    void printDisassembledProgram() const;
};




#endif //ZX16_SIMULATOR_H
