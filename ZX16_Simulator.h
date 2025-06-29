
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
    uint16_t trailingZeroCount = 0;
    uint16_t pc;                            // Program counter
    uint16_t programEnd;                    // address of the Program Ending at Memory
    bool running;                           // Execution state

    int volume;                           //Needed for ecall/game

    std::vector<Instruction> program;       // Loaded program

    // bool executeInstruction(const Instruction& inst);   moved to public for now to test in main
    void handleEcall();
    int16_t sext_imm4(uint8_t imm);  // sign extend 4 bit
    bool executeRType(const Instruction& inst);
    bool executeIType(const Instruction& inst);
    bool executeSType(const Instruction& inst);
    bool executeBType( Instruction& inst);
    bool executeLType(const Instruction& inst);
    bool executeJType( Instruction& inst);
    bool executeUType(const Instruction& inst);
    bool executeSysType(const Instruction& inst);
    bool executeInstruction( Instruction& inst);


public:
    ZX16_Simulator();

    void run();
    void loadBinaryFile(const std::string& filename);
    void dumpRegisters() const;
    void dumpMemory(uint16_t address, uint16_t size) const;
    // For testing/debug
    void printDisassembledProgram();


    //use this to set register values directly so you can test the execution of your instructions in main easily
    void setRegister(int index, int16_t value) {
        if (index >= 0 && index < NUM_REGISTERS) {
            registers[index] = value;
        }
    }
};




#endif //ZX16_SIMULATOR_H
