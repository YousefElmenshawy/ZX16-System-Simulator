
#ifndef ZX16_SIMULATOR_H
#define ZX16_SIMULATOR_H



#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include "Instruction.h"
using namespace std;
#include "Graphics.h"
#include <SFML/Audio.hpp>

class ZX16_Simulator {
private:
    static const int MEMORY_SIZE = 65536;   // 64KB addressable memory
    static const int NUM_REGISTERS = 8;//8 Registers
    uint8_t memory[MEMORY_SIZE];           // Main memory
    int16_t registers[NUM_REGISTERS];      // General-purpose registers
    uint16_t trailingZeroCount = 0;
                                // Program counter
    uint16_t programEnd;                    // address of the Program Ending at Memory
                             // Execution state
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

    void dumpMemory(uint16_t address, uint16_t size) const;
    Graphics* graphics= nullptr;
    // for sound
    sf::SoundBuffer hitBuffer;  // Sound buffers for hit
    sf::SoundBuffer loseBuffer; // Sound buffers for lose
    sf::Sound hitSound; // Sound objects for hit
    sf::Sound loseSound; // Sound objects for lose
public:
    void dumpRegisters() const;
    uint16_t pc;
    bool halted = false;
    ZX16_Simulator();
    void run();
    void loadBinaryFile(const std::string& filename);
    // For testing/debug
    void printDisassembledProgram();
    uint8_t* getMemoryPtr();

    //use this to set register values directly so you can test the execution of your instructions in main easily
    void setRegister(int index, int16_t value) {
        if (index >= 0 && index < NUM_REGISTERS) {
            registers[index] = value;
        }
    }
    void dumpTileMap(uint16_t start, uint16_t width, uint16_t height)const;
    void step();
    bool running;
    void runInteractive(Graphics* graphics);
    void playHitSound();
    void playLoseSound();
};




#endif //ZX16_SIMULATOR_H