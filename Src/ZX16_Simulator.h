#ifndef ZX16_SIMULATOR_H
#define ZX16_SIMULATOR_H



#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include "Instruction.h"
#include "Graphics.h"
#include <SFML/Audio.hpp>
using namespace std;

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
    void PrintDynamicDiassembley(uint16_t PC);
    void playHitSound();
    void playLoseSound();
    void dumpTileMap(uint16_t start, uint16_t width, uint16_t height) const ;

    Graphics* graphics= nullptr;
    // for sound
    sf::SoundBuffer hitBuffer;  // Sound buffers for hit
    sf::SoundBuffer loseBuffer; // Sound buffers for lose
    sf::Sound hitSound; // Sound objects for hit
    sf::Sound loseSound; // Sound objects for lose
public:
    void printState() const;
    void runInteractive(Graphics* g);
    uint8_t* getMemoryPtr();
    void reset();
    bool step();
    void dumpRegisters() const;
    void dumpMemory(uint32_t address, uint32_t size) const;
    void dumpMemoryPythonArray(uint32_t address, uint32_t size) const;
    void dumpMemoryJSON(uint32_t address, uint32_t size) const;
    void dumpMemoryCSV(uint32_t address, uint32_t size) const;
    ZX16_Simulator();
    void run();
    void loadBinaryFile(const std::string& filename);
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
