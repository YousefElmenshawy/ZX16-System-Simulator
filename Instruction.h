//
// Created by youfi on 6/22/2025.
//

#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <string>
#include <cstdint>

enum class InstructionType {
    R_TYPE,
    I_TYPE,
    S_TYPE,
    B_TYPE,
    U_TYPE,
    J_TYPE,
    SYS_Type,
    INVALID
};

class Instruction {
private:
    uint16_t Complete_Instruction;               // 16-bit binary instruction
    InstructionType type;       // Decoded type
    std::string Assembly_Code;       // Human-readable form (e.g., "add r1, r2, r3")

    // Decoded fields
    uint8_t opcode;
    uint8_t rd, rs1, rs2;
    int16_t imm;

public:
    Instruction(uint16_t rawInstruction);

    InstructionType getType() const;
    std::string AssemblyCode() const;//Human-readable form (e.g., "add r1, r2, r3")
    uint16_t getRaw() const;

    // Access decoded fields
    uint8_t getOpcode() const;
    uint8_t getRd() const;
    uint8_t getRs1() const;
    uint8_t getRs2() const;
    int16_t getImmediate() const;

    void decode();  // Fills in fields like opcode, rd, etc.
};



#endif //INSTRUCTION_H
