//
// Created by youfi on 6/22/2025.
//

#include "ZX16_Simulator.h"
#include <iostream>
#include<bitset>
#include <iomanip>  // for setw, setfill
#include "Graphics.h"
#include <thread>

using namespace std;


static const std::string regs[8] = {
    "t0", "ra", "sp", "s0", "s1", "t1", "a0", "a1"
};
ZX16_Simulator::ZX16_Simulator() {
    volume = 100;
    for (int i = 0; i < 8; i++) {
        registers[i] = 0;
    }
    if (!hitBuffer.loadFromFile("ballhit.wav"))
        cerr << "Error loading hit sound file.\n";
    else hitSound.setBuffer(hitBuffer);
    if (!loseBuffer.loadFromFile("balllose.wav"))
        cerr << "Error loading lose sound file.\n";
    else loseSound.setBuffer(loseBuffer);

}


void ZX16_Simulator::loadBinaryFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

   //file.seekg(0x20);  // Skip header

    uint16_t address = 0;
    uint8_t bytes[2];
    std::vector<Instruction> tempProgram;
    std::vector<uint16_t> rawInstructions;

    while (file.read(reinterpret_cast<char*>(bytes), 2)) {
        if (address >= MEMORY_SIZE-2 ) {
            std::cerr << "Error: Program exceeds memory bounds.\n";
            break;
        }

uint16_t value = bytes[0] | (bytes[1] << 8);  // Little endian
        memory[address]     = bytes[0];
            memory[address + 1] = bytes[1];

if (address>20&&address<0xF000) {
    // Skip first 20 bytes and tile map area
    Instruction inst(value);
    tempProgram.push_back(inst);
    rawInstructions.push_back(value);
}


        address += 2;
    }

    // Set programEnd to last loaded address
    programEnd = address;

    // Count trailing zero (NOP) instructions
    int trailingZeros = 0;
    for (int i = rawInstructions.size() - 1; i >= 0; --i) {
        if (rawInstructions[i] == 0x0000)
            ++trailingZeros;
        else
            break;
    }

    trailingZeroCount = trailingZeros;
    program = std::move(tempProgram);  // Save parsed instructions

    std::cout << "Program loaded into memory.\n";
}

bool ZX16_Simulator::executeRType(const Instruction& inst) {
    uint8_t rd = inst.getRd();
    uint8_t rs2 = inst.getRs2();
    uint8_t func4 = inst.getFunc4();
    uint8_t func3 = inst.getFunc3();

    if (func4 == 0b0000 && func3 == 0b000) {
        registers[rd] += registers[rs2];
        std::cout << "Executed: ADD " << regs[rd] << ", " << regs[rs2] << "\n";

    } else if (func4 == 0b0001 && func3 == 0b000) {
        registers[rd] -= registers[rs2];
        std::cout << "Executed: SUB " << regs[rd] << ", " << regs[rs2] << "\n";

    } else if (func4 == 0b0010 && func3 == 0b001) {
        registers[rd] = (registers[rd] < registers[rs2]) ? 1 : 0;
        std::cout << "Executed: SLT " << regs[rd] << ", " << regs[rs2] << "\n";

    } else if (func4 == 0b0011 && func3 == 0b010) {
        registers[rd] = (static_cast<uint16_t>(registers[rd]) < static_cast<uint16_t>(registers[rs2])) ? 1 : 0;
        std::cout << "Executed: SLTU " << regs[rd] << ", " << regs[rs2] << "\n";

    } else if (func4 == 0b0100 && func3 == 0b011) {
        registers[rd] <<= (registers[rs2] & 0xF);
        std::cout << "Executed: SLL " << regs[rd] << ", " << regs[rs2] << "\n";

    } else if (func4 == 0b0101 && func3 == 0b011) {
        registers[rd] = static_cast<uint16_t>(registers[rd]) >> (registers[rs2] & 0xF);
        std::cout << "Executed: SRL " << regs[rd] << ", " << regs[rs2] << "\n";

    } else if (func4 == 0b0110 && func3 == 0b011) {
        registers[rd] >>= (registers[rs2] & 0xF);
        std::cout << "Executed: SRA " << regs[rd] << ", " << regs[rs2] << "\n";

    } else if (func4 == 0b0111 && func3 == 0b100) {
        registers[rd] |= registers[rs2];
        std::cout << "Executed: OR " << regs[rd] << ", " << regs[rs2] << "\n";

    } else if (func4 == 0b1000 && func3 == 0b101) {
        registers[rd] &= registers[rs2];
        std::cout << "Executed: AND " << regs[rd] << ", " << regs[rs2] << "\n";

    } else if (func4 == 0b1001 && func3 == 0b110) {
        registers[rd] ^= registers[rs2];
        std::cout << "Executed: XOR " << regs[rd] << ", " << regs[rs2] << "\n";

    } else if (func4 == 0b1010 && func3 == 0b111) {
        registers[rd] = registers[rs2];
        std::cout << "Executed: MV " << regs[rd] << ", " << regs[rs2] << "\n";

    } else if (func4 == 0b1011 && func3 == 0b000) {
        pc = registers[rd];
        std::cout << "Executed: JR " << regs[rd] << " -> PC = " << pc << "\n";
        return true;

    } else if (func4 == 0b1100 && func3 == 0b000) {
        registers[rd] = pc + 2;
        pc = registers[rs2];
        std::cout << "Executed: JALR " << regs[rd] << ", " << regs[rs2] << " -> PC = " << pc << "\n";
        return true;

    } else {
        std::cerr << "Unknown R-Type instruction with func4=" << std::bitset<4>(func4)
                  << " func3=" << std::bitset<3>(func3) << " at PC=" << pc << "\n";
    }
    return false;
}

bool ZX16_Simulator::executeIType(const Instruction& inst) {
    uint8_t rd = inst.getRd();
    uint8_t func3 = inst.getFunc3();
    uint8_t func4 = inst.getFunc4(); // imm7[6:4] stored here (bits 15-12)
    int16_t imm = inst.getImmediate(); // 7-bit immediate signed (imm7)
    uint8_t shamt = imm & 0xF; ; // Extract 5-bit shift amount

    uint16_t uimm = static_cast<uint16_t>(imm); // unsigned interpretation

    switch (func3) {
        case 0b000: // ADDI
            registers[rd] = registers[rd] + imm;
            std::cout << "Executed: ADDI " << regs[rd] << ", " << imm << "\n";
            break;

        case 0b001: // SLTI (signed)
            registers[rd] = (static_cast<int16_t>(registers[rd]) < imm) ? 1 : 0;
            std::cout << "Executed: SLTI " << regs[rd] << ", " << imm << "\n";
            break;

        case 0b010: // SLTUI (unsigned)
            registers[rd] = (registers[rd] < uimm) ? 1 : 0;
            std::cout << "Executed: SLTUI " << regs[rd] << ", " << uimm << "\n";
            break;

        case 0b011: // Shift instructions: SLLI, SRLI, SRAI
            if (func4 == 0b001) { // SLLI
                registers[rd] = registers[rd] << shamt;
                std::cout << "Executed: SLLI " << regs[rd] << ", " << (int)shamt << "\n";
            } else if (func4 == 0b010) { // SRLI
                registers[rd] = static_cast<uint16_t>(registers[rd]) >> shamt;
                std::cout << "Executed: SRLI " << regs[rd] << ", " << (int)shamt << "\n";
            } else if (func4 == 0b100) { // SRAI
                registers[rd] = static_cast<int16_t>(registers[rd]) >> shamt;
                std::cout << "Executed: SRAI " << regs[rd] << ", " << (int)shamt << "\n";
            } else {
                std::cerr << "Unknown shift immediate instruction func4=" << std::bitset<4>(func4) << "\n";
            }
            break;

        case 0b100: // ORI
            registers[rd] = registers[rd] | imm;
            std::cout << "Executed: ORI " << regs[rd] << ", " << imm << "\n";
            break;

        case 0b101: // ANDI
            registers[rd] = registers[rd] & imm;
            std::cout << "Executed: ANDI " << regs[rd] << ", " << imm << "\n";
            break;

        case 0b110: // XORI
            registers[rd] = registers[rd] ^ imm;
            std::cout << "Executed: XORI " << regs[rd] << ", " << imm << "\n";
            break;

        case 0b111: // LI - load immediate directly
            registers[rd] = imm;
            std::cout << "Executed: LI " << regs[rd] << ", " << imm << "\n";
            break;

        default:
            std::cerr << "Unknown I-Type func3=" << std::bitset<3>(func3) << " at PC=" << pc << "\n";
            break;
    }

    return false; // PC not manually changed
}


bool ZX16_Simulator::executeSType(const Instruction& inst) {
    uint8_t rs1 = inst.getRs1();
    uint8_t rs2 = inst.getRs2();
    uint8_t func3 = inst.getFunc3();
    int16_t imm = inst.getImmediate();
    uint16_t address = registers[rs1] + imm;

    if (func3 == 0b000) { // SB - store byte
        if (address < MEMORY_SIZE) {
            memory[address] = registers[rs2] & 0xFF; // store 1 byte only
            std::cout << "Executed: SB " << regs[rs2] << " → Memory[" << address << "]\n";
        } else {
            std::cerr << "Memory write out of bounds at address " << address << "\n";
        }
    } else if (func3 == 0b001) { // SW - store word (2 bytes)
        if (address < MEMORY_SIZE - 1) {
            memory[address] = registers[rs2] & 0xFF;
            memory[address + 1] = (registers[rs2] >> 8) & 0xFF;
            std::cout << "Executed: SW " << regs[rs2] << " -> Memory[" << address << "]\n";
        } else {
            std::cerr << "Memory write out of bounds at address " << address << "\n";
        }
    }

    return false;
}

bool ZX16_Simulator::executeBType(Instruction& inst) {
    uint8_t rs1 = inst.getRs1();
    uint8_t rs2 = inst.getRs2();
    uint8_t func3 = inst.getFunc3();
    int16_t imm = inst.getImmediate()+2;
    inst.readPC(pc);

    switch (func3) {
        case 0b000:
            if (registers[rs1] == registers[rs2]) {
                pc += imm;
                std::cout << "Executed: BEQ " << regs[rs1] << ", " << regs[rs2] << " -> PC = " << pc << "\n";
                return true;
            }
            std::cout << "BEQ condition not met, PC remains " << pc << "\n";
            break;

        case 0b001:
            if (registers[rs1] != registers[rs2]) {
                pc += imm;
                std::cout << "Executed: BNE " << regs[rs1] << ", " << regs[rs2] << " -> PC = " << pc << "\n";
                return true;
            }
            std::cout << "BNE condition not met, PC remains " << pc << "\n";
            break;

        case 0b010:
            if (registers[rs1] == 0) {
                pc += imm;
                std::cout << "Executed: BZ " << regs[rs1] << " -> PC = " << pc << "\n";
                return true;
            }
            std::cout << "BZ condition not met, PC remains " << pc << "\n";
            break;

        case 0b011:
            if (registers[rs1] != 0) {
                pc += imm;
                std::cout << "Executed: BNZ " << regs[rs1] << " -> PC = " << pc << "\n";
                return true;
            }
            std::cout << "BNZ condition not met, PC remains " << pc << "\n";
            break;

        case 0b100:
            if (registers[rs1] < registers[rs2]) {
                pc += imm;
                std::cout << "Executed: BLT " << regs[rs1] << ", " << regs[rs2] << " -> PC = " << pc << "\n";
                return true;
            }
            std::cout << "BLT condition not met, PC remains " << pc << "\n";
            break;

        case 0b101:
            if (registers[rs1] >= registers[rs2]) {
                pc += imm;
                std::cout << "Executed: BGE " << regs[rs1] << ", " << regs[rs2] << " -> PC = " << pc << "\n";
                return true;
            }
            std::cout << "BGE condition not met, PC remains " << pc << "\n";
            break;

        case 0b110:
            if (static_cast<uint16_t>(registers[rs1]) < static_cast<uint16_t>(registers[rs2])) {
                pc += imm;
                std::cout << "Executed: BLTU " << regs[rs1] << ", " << regs[rs2] << " -> PC = " << pc << "\n";
                return true;
            }
            std::cout << "BLTU condition not met, PC remains " << pc << "\n";
            break;

        case 0b111:
            if (static_cast<uint16_t>(registers[rs1]) >= static_cast<uint16_t>(registers[rs2])) {
                pc += imm;
                std::cout << "Executed: BGEU " << regs[rs1] << ", " << regs[rs2] << " -> PC = " << pc << "\n";
                return true;
            }
            std::cout << "BGEU condition not met, PC remains " << pc << "\n";
            break;
    }
    return false;
}

bool ZX16_Simulator::executeLType(const Instruction& inst) {
    uint8_t rd = inst.getRd();
    uint8_t rs2 = inst.getRs2();
    uint8_t func3 = inst.getFunc3();
    int16_t imm = inst.getImmediate();
    uint16_t address = registers[rs2] + imm;

    if (func3 == 0b000) {
        int8_t byte = static_cast<int8_t>(memory[address]);
        registers[rd] = static_cast<int16_t>(byte);
        std::cout << "Executed: LB " << regs[rd] << " <- MEM[" << address << "] (signed byte)\n";

    } else if (func3 == 0b001) {
        uint16_t word = memory[address] | (memory[address + 1] << 8);
        registers[rd] = static_cast<int16_t>(word);
        std::cout << "Executed: LW " << regs[rd] << " <- MEM[" << address << "] (16-bit word)\n";

    } else if (func3 == 0b100) {
        uint8_t byte = memory[address];
        registers[rd] = static_cast<uint16_t>(byte);
        std::cout << "Executed: LBU " << regs[rd] << " <- MEM[" << address << "] (unsigned byte)\n";

    } else {
        std::cerr << "Unknown L-Type instruction with func3=" << std::bitset<3>(func3)
                  << " at PC=" << pc << "\n";
    }
    return false;
}

bool ZX16_Simulator::executeJType( Instruction& inst) {
    uint8_t rd = inst.getRd();
    uint8_t flag = inst.getflag();
    int16_t imm = inst.getImmediate();
    int16_t offset = imm+2;
    //inst.readPC(pc);

    if (flag == 0) {
        pc += offset;

        std::cout << "Executed: JUMP -> PC <- PC + " << offset << " = " << pc << "\n";
        return true;

    } else if (flag == 1) {
        registers[rd] = pc + 2;
        pc += offset;
        std::cout << "Executed: JAL " << regs[rd] << " <- PC+2, PC <- PC + " << offset << " = " << pc << "\n";
        return true;

    } else {
        std::cerr << "Unknown J-Type instruction with flag=" << flag << " at PC=" << pc << "\n";
        return false;
    }
}

bool ZX16_Simulator::executeUType(const Instruction& inst) {
    uint8_t rd = inst.getRd();
    int16_t imm = inst.getImmediate();
    uint8_t flag = inst.getflag();

    if (flag == 0) {
        registers[rd] = imm << 7;
        std::cout << "Executed: LUI " << regs[rd] << " <- " << (imm << 7) << "\n";

    } else if (flag == 1) {
        registers[rd] = pc + (imm << 7);
        std::cout << "Executed: AUIPC " << regs[rd] << " <- PC + " << (imm << 7) << "\n";

    } else {
        std::cerr << "Unknown U-Type instruction with flag=" << flag << "\n";
    }
    return false;
}
bool ZX16_Simulator::executeSysType(const Instruction& inst) {
    uint16_t svc = inst.getSvc();
    std::cout << "ECALL executed. Halting the simulator." << std::endl;
    running = false;

    switch (svc) {
        case 1: {
            char* buffer = reinterpret_cast<char*>(&memory[registers[6]]);
            int maxLength = registers[7];
            std::cin.getline(buffer, maxLength);
            registers[6] = std::cin.gcount();

            if (registers[6] < maxLength) {
                buffer[registers[6]] = '\0';
            } else {
                buffer[maxLength - 1] = '\0';
            }

            std::cout << "ECALL done. Continuing the simulator." << std::endl;
            running = true;
            return false;
        }
        case 2: {
            int value;
            std::cin >> value;
            registers[6] = value;
            std::cout << "ECALL done. Continuing the simulator." << std::endl;
            running = true;
            return false;
        }
        case 3: { // Print null-terminated string
            uint16_t addr = registers[6];
            if (addr >= MEMORY_SIZE) {
                std::cerr << "Error: Invalid memory address in a0 (0x" << std::hex << addr << ")\n";
                running = true;
                return false;
            }

            while (addr < MEMORY_SIZE && memory[addr] != '\0') {
                std::cout << static_cast<char>(memory[addr]);
                addr++;
            }
            std::cout.flush(); // Ensure output is displayed immediately

            std::cout << "ECALL done. Continuing the simulator." << std::endl;
            running = true;
            return false;
        }
        case 4: {
            int frequency = registers[6];
            int duration = registers[7];
            std::cout << "Playing tone: Frequency=" << frequency << " Hz, Duration=" << duration << " ms\n";
            if (frequency >= 10) {
                playHitSound();  // for hitting the ball
            } else {
                playLoseSound(); // for losing
            }
            std::cout << "ECALL done. Continuing the simulator." << std::endl;
            running = true;
            return false;
        }
        case 5: { // Set Audio Volume
            int value = registers[6];
            if (value < 0 || value > 255) {
                std::cerr << "Error: Volume must be between 0 and 255. Received: " << value << "\n";
            } else {
                volume = value;
                std::cout << "Setting audio volume to " << volume << "\n";
                hitSound.setVolume(static_cast<float>(volume));
                loseSound.setVolume(static_cast<float>(volume));
            }

            std::cout << "ECALL done. Continuing the simulator." << std::endl;
            running = true;
            return false;
        }
        case 6: { // Stop Audio Playback
            std::cout << "Stopping audio playback\n";
            if (hitSound.getStatus() == sf::Sound::Playing) {
                hitSound.stop();
            }
            if (loseSound.getStatus() == sf::Sound::Playing) {
                loseSound.stop();
            }


            std::cout << "ECALL done. Continuing the simulator." << std::endl;
            running = true;
            return false;
        }
        case 7: { // Read Keyboard


                if (graphics) {
                    if (graphics->hasKeyPressed()) {

                        // Map SFML key code to a simple value (e.g., just use the code)
                        registers[6] = static_cast<int>(graphics->getLastKeyPressed()); // a0 = key code
                        registers[7] = 1; // a1 = key pressed

                        graphics->resetKeyFlag(); // clear flag after reading
                    } else {
                        registers[7] = 0; // a1 = no key pressed
                    }
                } else {
                    registers[7] = 0; // a1 = no key pressed
                }

                std::cout << "ECALL done. Continuing the simulator." << std::endl;
                running = true;

            return false;
        }
        case 8: { // Registers Dump
            dumpRegisters();

            std::cout << "ECALL done. Continuing the simulator." << std::endl;
            running = true;
            return false;
        }
        case 9: { // Memory Dump
            uint16_t address = registers[6];
            uint16_t size = registers[7];
            dumpMemory(address, size);

            std::cout << "ECALL done. Continuing the simulator." << std::endl;
            running = true;
            return false;
        }
        case 10: { // Program Exit
            std::cout << "Program exiting...\n";
            running = false;
            return false;
        }
        case 11: { //render the screen
            if (graphics) {
                graphics->tick();  // Trigger screen update
            }
            std::cout << "ECALL done. Continuing the simulator." << std::endl;
            running = true;
            return false;
            break;
        }
        default:
            std::cerr << "Unknown service number: " << svc << "\n";
            break;
    }
    return false;
}




void ZX16_Simulator::run() {

    pc = 0;
    running = true;

    while (running && pc < programEnd) {
        uint16_t BinaryInstruction = memory[pc] | (memory[pc + 1] << 8);

        Instruction inst(BinaryInstruction);
        PrintDynamicDiassembley(BinaryInstruction);
        bool jumped = executeInstruction(inst);


        //inst.generateAssemblyString();

        if (!jumped) pc += 2;
        if (pc >= programEnd) {
            std::cerr << "PC out of program bounds at 0x" << std::hex << pc << ". Halting execution.\n";
            running = false;
        }
    }
}



void ZX16_Simulator::dumpRegisters() const {

    for (int i = 0; i < NUM_REGISTERS; ++i) {
        cout << regs[i] << " = " << std::dec << (registers[i]) << "\n";
    }
}

void ZX16_Simulator::PrintDynamicDiassembley(uint16_t binary) {
    Instruction inst(binary);
    inst.readPC(pc);  // To handle PC-relative instructions properly

    std::cout << "["
              << std::setw(4) << std::setfill('0') << std::hex << pc
              << "]  0x"
              << std::setw(4) << std::setfill('0') << std::hex << binary
              << "  " << inst.AssemblyCode()
              << std::endl;
}
void ZX16_Simulator::printDisassembledProgram() {//Static decoding For Testing Purposes Only
    cout << "Disassembled Program:\n";
    size_t lastUsefulIndex = program.size() - trailingZeroCount;
    for (size_t i = 0; i < lastUsefulIndex; ++i) {
        uint16_t address = i * 2;  // each instruction is 2 bytes

        uint16_t raw = program[i].get_CompleteInstruction();  // Add this function if needed
        program[i].readPC(address);
        program[i].decode();
        cout << "[" << std::hex << std::setw(4) << std::setfill('0') << address << "]  "
             << "0x" << std::setw(4) << raw << "  "
             << program[i].AssemblyCode() << "\n";
    }
}
bool ZX16_Simulator::executeInstruction(Instruction& inst) {
    switch (inst.getType()) {
        case InstructionType::R_TYPE:
            return executeRType(inst);

        case InstructionType::I_TYPE:
            return executeIType(inst);

        case InstructionType::S_TYPE:
            return executeSType(inst);

        case InstructionType::B_TYPE:
            inst.readPC(pc);
            return executeBType(inst);

        case InstructionType::L_TYPE:
            return executeLType(inst);

        case InstructionType::J_TYPE:
            inst.readPC(pc);
            return executeJType(inst);

        case InstructionType::U_TYPE:
            return executeUType(inst);

        case InstructionType::SYS_Type:
            return executeSysType(inst);

        default:
            if (inst.get_CompleteInstruction() == 0x0000) {
                //std::cout << "No operation (NOP) at PC=" << pc << "\n";
                return false; // NOP does not change PC
            }
            std::cerr << "Unknown instruction type at PC=" << pc << "\n";
            return false;
    }
}


void ZX16_Simulator::dumpMemory(uint16_t address, uint16_t size) const {
    if (address + size > MEMORY_SIZE) {
        std::cerr << "Memory dump out of bounds: address=" << address << ", size=" << size << "\n";
        return;
    }

    std::cout << "Memory Dump (address=" << address << ", size=" << size << "):\n";
    for (uint16_t i = 0; i < size; ++i) {
        if (i % 16 == 0) {
            std::cout << std::hex << std::setw(4) << std::setfill('0') << (address + i) << ": ";
        }
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(memory[address + i]) << " ";
        if (i % 16 == 15) {
            std::cout << "\n";
        }
    }
    std::cout << std::dec << std::endl;
}
uint8_t* ZX16_Simulator::getMemoryPtr() { return memory; }

void ZX16_Simulator::step() {
    if (!running) {
        std::cout << "Simulator is halted.\n";
        return;
    }

    if (halted) return;

    if (pc >= programEnd) {
        std::cerr << "PC out of program bounds at 0x" << std::hex << pc << ". Halting execution.\n";
        running = false;
        return;
    }

    // Fetch 16-bit instruction from memory (little-endian)
    uint16_t binaryInstruction = memory[pc] | (memory[pc + 1] << 8);

    // Decode instruction
    Instruction inst(binaryInstruction);
    PrintDynamicDiassembley(binaryInstruction);

    // Execute instruction
    bool jumped = executeInstruction(inst);

    // If instruction didn't jump, increment PC normally
    if (!jumped) {
        pc += 2;
    }

    // Check bounds again
    if (pc >= programEnd) {
        std::cerr << "PC out of program bounds at 0x" << std::hex << pc << ". Halting execution.\n";
        running = false;
    }
}
void ZX16_Simulator::runInteractive(Graphics* g) {
    graphics = g;
    if (!graphics) return;

    pc = 0;
    running = true;

    int frameCounter = 0;
    const int RENDER_EVERY_N_FRAMES = 60; // Render every 10 instruction cycles

    while (graphics->isOpen() && running) {
        // Process events every cycle to maintain responsiveness
        graphics->processEvents();
        // Execute instruction
        if (pc < programEnd) {
            uint16_t instBin = memory[pc] | (memory[pc + 1] << 8);
            Instruction inst(instBin);
            PrintDynamicDiassembley(instBin);
            bool jumped = executeInstruction(inst);
            if (!jumped) pc += 2;

            if (pc >= programEnd) {
                running = false;
            }
        }

        // Render less frequently to maintain speed
        frameCounter++;
        if (frameCounter >= RENDER_EVERY_N_FRAMES) {
            graphics->render();
            frameCounter = 0;
        }
       //dumpTileMap(0xF000, 20, 15);
        dumpRegisters();
        // Tiny sleep to prevent 100% CPU usage
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}
void ZX16_Simulator::dumpTileMap(uint16_t start, uint16_t width, uint16_t height) const {
    std::cout << "\n[TileMap Dump from 0x" << std::hex << start << "]\n";
    for (uint16_t row = 0; row < height; ++row) {
        for (uint16_t col = 0; col < width; ++col) {
            uint16_t addr = start + row * width + col;
            std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)memory[addr] << " ";
        }
        std::cout << "\n";
    }
    std::cout << std::dec;
}
void ZX16_Simulator::playHitSound() {
    hitSound.setVolume(volume);
    hitSound.play();
}

void ZX16_Simulator::playLoseSound() {
    loseSound.setVolume(volume);
    loseSound.play();
}