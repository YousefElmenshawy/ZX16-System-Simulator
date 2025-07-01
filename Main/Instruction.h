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
    L_TYPE,
    INVALID
};

class Instruction {
private:
    uint16_t Complete_Instruction;               // 16-bit binary instruction
    InstructionType type;       // Decoded type
    std::string Assembly_Code;       // Human-readable form (e.g., "add r1, r2, r3")

    // Decoded fields
    uint8_t opcode;
    uint8_t rd,rs1,  rs2, func3,func4, flag;
    int16_t imm;

    uint16_t svc; //For SYS
    uint16_t PC;

public:
    Instruction(uint16_t rawInstruction);

    void generateAssemblyString();
    InstructionType getType() const;
    std::string AssemblyCode() const;//Human-readable form (e.g., "add r1, r2, r3")
    uint16_t get_CompleteInstruction() const;  // 16-bit binary instruction

    // Access decoded fields
    void readPC(uint16_t pc) ;
    uint8_t getOpcode() const;
    uint8_t getRd() const;
    uint8_t getRs2() const;
    uint8_t getRs1() const;
    uint8_t getflag() const;
    int16_t getImmediate() const;
    uint8_t getFunc3() const;
    uint8_t getFunc4() const;
    uint16_t getSvc() const; // For SYS instructions
    void decode();  // Fills in fields like opcode, rd, etc.
};



#endif //INSTRUCTION_H
