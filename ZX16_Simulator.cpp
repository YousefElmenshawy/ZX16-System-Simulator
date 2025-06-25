//
// Created by youfi on 6/22/2025.
//

#include "ZX16_Simulator.h"
#include <iostream>
#include<bitset>
#include <iomanip>  // for setw, setfill

using namespace std;


static const std::string regs[8] = {
    "t0", "ra", "sp", "s0", "s1", "t1", "a0", "a1"
};
ZX16_Simulator::ZX16_Simulator() {
    // Initialize members here
}


void ZX16_Simulator::loadBinaryFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    uint16_t address = 0; // load instructions starting at 0x0000
    uint16_t value;

    while (file.read(reinterpret_cast<char*>(&value), sizeof(value))) {

        Instruction inst(value);
        program.push_back(inst);
        // Store in little-endian order into memory
        memory[address]     = value & 0xFF;       // lower byte
        memory[address + 1] = (value >> 8) & 0xFF; // upper byte
        address += 2;
    }
    programEnd= address;

    std::cout << "Program loaded into memory.\n";

}

bool ZX16_Simulator::executeInstruction(const Instruction& inst) {
    uint8_t rd = inst.getRd();
    uint8_t rs2 = inst.getRs2();
    uint8_t func4 = inst.getFunc4();
    uint8_t func3 = inst.getFunc3();
    uint8_t flag = inst.getflag();
    int16_t imm = inst.getImmediate();
    uint16_t svc = inst.getSvc();

    switch (inst.getType()) {
        case InstructionType::R_TYPE: {
            if (func4 == 0b0000 && func3 == 0b000) { // ADD
                registers[rd] += registers[rs2];
                std::cout << "Executed: ADD " << regs[rd] << ", " << regs[rs2] << "\n";

            } else if (func4 == 0b0001 && func3 == 0b000) { // SUB
                registers[rd] -= registers[rs2];
                std::cout << "Executed: SUB " << regs[rd] << ", " << regs[rs2] << "\n";

            } else if (func4 == 0b0010 && func3 == 0b001) { // SLT
                registers[rd] = (registers[rd] < registers[rs2]) ? 1 : 0;
                std::cout << "Executed: SLT " << regs[rd] << ", " << regs[rs2] << "\n";

            } else if (func4 == 0b0011 && func3 == 0b010) { // SLTU
                registers[rd] = (static_cast<uint16_t>(registers[rd]) < static_cast<uint16_t>(registers[rs2])) ? 1 : 0;
                std::cout << "Executed: SLTU " << regs[rd] << ", " << regs[rs2] << "\n";

            } else if (func4 == 0b0100 && func3 == 0b011) { // SLL
                registers[rd] <<= (registers[rs2] & 0xF);
                std::cout << "Executed: SLL " << regs[rd] << ", " << regs[rs2] << "\n";

            } else if (func4 == 0b0101 && func3 == 0b011) { // SRL
                registers[rd] = static_cast<uint16_t>(registers[rd]) >> (registers[rs2] & 0xF);
                std::cout << "Executed: SRL " << regs[rd] << ", " << regs[rs2] << "\n";

            } else if (func4 == 0b0110 && func3 == 0b011) { // SRA
                registers[rd] >>= (registers[rs2] & 0xF);
                std::cout << "Executed: SRA " << regs[rd] << ", " << regs[rs2] << "\n";

            } else if (func4 == 0b0111 && func3 == 0b100) { // OR
                registers[rd] |= registers[rs2];
                std::cout << "Executed: OR " << regs[rd] << ", " << regs[rs2] << "\n";

            } else if (func4 == 0b1000 && func3 == 0b101) { // AND
                registers[rd] &= registers[rs2];
                std::cout << "Executed: AND " << regs[rd] << ", " << regs[rs2] << "\n";

            } else if (func4 == 0b1001 && func3 == 0b110) { // XOR
                registers[rd] ^= registers[rs2];
                std::cout << "Executed: XOR " << regs[rd] << ", " << regs[rs2] << "\n";

            } else if (func4 == 0b1010 && func3 == 0b111) { // MV
                registers[rd] = registers[rs2];
                std::cout << "Executed: MV " << regs[rd] << ", " << regs[rs2] << "\n";

            } else if (func4 == 0b1011 && func3 == 0b000) { // JR
                pc = registers[rd];
                std::cout << "Executed: JR " << regs[rd] << " → PC = " << pc << "\n";
                return true; // manually changed PC

            } else if (func4 == 0b1100 && func3 == 0b000) { // JALR
                registers[rd] = pc + 2;
                pc = registers[rs2];
                std::cout << "Executed: JALR " << regs[rd] << ", " << regs[rs2] << " → PC = " << pc << "\n";
                return true; // manually changed PC

            } else {
                std::cerr << "Unknown R-Type instruction with func4=" << std::bitset<4>(func4)
                          << " func3=" << std::bitset<3>(func3) << " at PC=" << pc << "\n";
            }
            return false; // pc not manually changed
        }

        case InstructionType::I_TYPE:
            // Implement I-type logic here
            return false;

        case InstructionType::S_TYPE:
            // Implement S-type logic here
            return false;

        case InstructionType::B_TYPE:
            // Implement B-type logic here
            return false;

        case InstructionType::J_TYPE:
            // Implement J-type logic here
            return false;

        case InstructionType::U_TYPE: {
            if (flag == 0) { // LUI
                registers[rd] = imm << 7;
                std::cout << "Executed: LUI " << regs[rd] << " ← " << (imm << 7) << "\n";
            } else if (flag == 1) { // AUIPC
                registers[rd] = pc + (imm << 7);
                std::cout << "Executed: AUIPC " << regs[rd] << " ← PC + " << (imm << 7) << "\n";
            } else {
                std::cerr << "Unknown U-Type instruction with flag=" << flag << "\n";
            }
            return false; // PC not manually changed
        }

        case InstructionType::SYS_Type: {
            std::cout << "ECALL executed. Halting the simulator." << std::endl;
            running = false; // Stop the simulator
            switch (svc) { // Service number in the instruction
                case 1:
                { // Read String
                    char* buffer = reinterpret_cast<char*>(&memory[registers[6]]);
                    int maxLength = registers[7];
                    std::cin.getline(buffer, maxLength);
                    registers[6] = std::cin.gcount(); // Set a0 to string length


                    // Ensure null termination
                    if (registers[6] < maxLength) {
                        buffer[registers[6]] = '\0';
                    } else {
                        buffer[maxLength - 1] = '\0';
                    }

                    std::cout << "ECALL done. Continuing the simulator." << std::endl;
                    running = true; // Cont the simulator
                    return false; // pc not manually changed

                }
                case 2: { // Read Integer
                    int value;
                    std::cin >> value;
                    registers[6] = value; // Set a0 to the read integer



                    std::cout << "ECALL done. Continuing the simulator." << std::endl;
                    running = true; // Cont the simulator
                    return false; // pc not manually changed




                }
                case 3: { // Print String
                    char* str = reinterpret_cast<char*>(&memory[registers[6]]);
                    while (*str != '\0') {
                        std::cout << *str;
                        ++str;
                    }

                    std::cout << "ECALL done. Continuing the simulator." << std::endl;
                    running = true; // Cont the simulator
                    return false; // pc not manually changed


                }
                case 4: { // Play Tone
                    int frequency = registers[6];
                    int duration = registers[7];
                    std::cout << "Playing tone: Frequency=" << frequency << " Hz, Duration=" << duration << " ms\n";

                    //ACTUAL IMPLEMENTATION LATER



                    std::cout << "ECALL done. Continuing the simulator." << std::endl;
                    running = true; // Cont the simulator
                    return false; // pc not manually changed


                }
                case 5: { // Set Audio Volume
                    int value = registers[6];
                    if (value < 0 || value > 255) {
                        std::cerr << "Error: Volume must be between 0 and 255. Received: " << value << "\n";
                    } else {
                        volume = value;
                        std::cout << "Setting audio volume to " << volume << "\n";
                    }



                    std::cout << "ECALL done. Continuing the simulator." << std::endl;
                    running = true; // Cont the simulator
                    return false; // pc not manually changed


                }
                case 6: { // Stop Audio Playback
                    std::cout << "Stopping audio playback\n";


                    // Implementation later

                    std::cout << "ECALL done. Continuing the simulator." << std::endl;
                    running = true; // Cont the simulator
                    return false; // pc not manually changed


                }
                case 7: { // Read Keyboard
                    char key;
                    if (std::cin.peek() != EOF) {
                        key = std::cin.get();
                        registers[6] = key; // Set a0 to the key code
                        registers[7] = 1;   // Set a1 to 1 (key pressed)
                    } else {
                        registers[7] = 0;   // Set a1 to 0 (nothing pressed)
                    }


                    std::cout << "ECALL done. Continuing the simulator." << std::endl;
                    running = true; // Cont the simulator
                    return false; // pc not manually changed


                }
                case 8: { // Registers Dump
                    dumpRegisters();



                    std::cout << "ECALL done. Continuing the simulator." << std::endl;
                    running = true; // Cont the simulator
                    return false; // pc not manually changed

                }
                case 9: { // Memory Dump
                    uint16_t address = registers[6];
                    uint16_t size = registers[7];
                    dumpMemory(address, size);


                    std::cout << "ECALL done. Continuing the simulator." << std::endl;
                    running = true; // Cont the simulator
                    return false; // pc not manually changed


                }
                case 10: { // Program Exit
                    std::cout << "Program exiting...\n";
                    running = false;
                    exit(0); // Exit the simulator gracefully



                }
                default:
                    std::cerr << "Unknown service number: " << registers[6] << "\n";
                    break;
            }
        }
    }
}



void ZX16_Simulator::run() {
    pc = 0;
    running = true;

    while (running && pc < programEnd) {
        uint16_t BinaryInstruction = memory[pc] | (memory[pc + 1] << 8);
        Instruction inst(BinaryInstruction);

        bool jumped = executeInstruction(inst);

        if (!jumped) pc += 2;
    }
}



void ZX16_Simulator::dumpRegisters() const {


    for (int i = 0; i < NUM_REGISTERS; ++i) {
        std::cout << regs[i] << " = " << registers[i] << "\n";
    }
}
void ZX16_Simulator::printDisassembledProgram() const {
    cout << "Disassembled Program:\n";

    for (size_t i = 0; i < program.size(); ++i) {
        uint16_t address = i * 2;  // each instruction is 2 bytes
        cout << "[" << std::hex << std::setw(4) << std::setfill('0') << address << "]  "
                  << program[i].AssemblyCode() << "\n";
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