//
// Created by youfi on 6/22/2025.
//

#include "Instruction.h"
#include<iostream>
#include <sstream>

using namespace std;
Instruction::Instruction(uint16_t value)
    : Complete_Instruction(value), opcode(0), rd(0),  rs2(0), imm(0), svc(0), type(InstructionType::INVALID) {
    decode();
}
void Instruction::decode() {
     opcode = Complete_Instruction & 0x7; // bits [2:0]

    switch (opcode) {
        case 0b000: { // R-Type
            type = InstructionType::R_TYPE;
            func4 = (Complete_Instruction >> 12) & 0xF;
            rs2 = (Complete_Instruction >> 9) & 0x7;
            rd = (Complete_Instruction >> 6) & 0x7;
            func3 = (Complete_Instruction >> 3) & 0x7;
            break;
        }
        case 0b001: { // I-Type
            type = InstructionType::I_TYPE;

            uint8_t rawImm = (Complete_Instruction >> 9) & 0x7F;  // bits [15:9]
            // Sign-extend 7-bit immediate to 16 bits
            if (rawImm & 0x40)  // if sign bit set
                imm = static_cast<int16_t>(rawImm | 0xFF80);
            else
                imm = rawImm;


            rd    = (Complete_Instruction >> 6) & 0x7;   // bits [8:6]
            func3 = (Complete_Instruction >> 3) & 0x7;   // bits [5:3]

            break;
        }

        case 0b010:  { // B-Type (e.g., branch)
            type = InstructionType::B_TYPE;
            // high 4 bits of 5-bit signed offset, imm[0] = 0--> [15:12]
            uint8_t rawImm = (Complete_Instruction >> 12) & 0xF;  // bits [15:12]
            // Sign-extend 4-bit immediate to 16 bits
            uint8_t shiftedImm = rawImm << 1; // shift left by 1 to set imm[0] = 0
            if (shiftedImm & 0x10)  // if sign bit (bit 4) is 1
                imm = shiftedImm | 0xFFF0;  // fill upper bits with 1s
            else    
                imm = shiftedImm;
            rs2 = (Complete_Instruction >> 9) & 0x7;  // bits [11:9]
            rs1 = (Complete_Instruction >> 6) & 0x7;  // bits [8:6]
            func3 = (Complete_Instruction >> 3) & 0x7;  // bits [5:3]
            break;
        }

        case 0b011: {  // S-Type (store)
            type = InstructionType::S_TYPE;
            // Extract the raw 4-bit immediate from bits [15:12]
            uint8_t rawImm = (Complete_Instruction >> 12) & 0xF;  // bits [15:12]
            // Sign-extend 4-bit immediate to 16 bits
            if (rawImm & 0x8)  // if sign bit (bit 3) is 1
                imm = rawImm | 0xFFF0;  // fill upper bits with 1s
            else
                imm = rawImm;
            rs2 = (Complete_Instruction >> 9) & 0x7; // bits [11:9] --> data register
            rs1 = (Complete_Instruction >> 6) & 0x7; // bits [8:6]  --> base register
            func3 = (Complete_Instruction >> 3) & 0x7; // bits [5:3]
            break;
        }

        case 0b100: {
            // L-Type (load)
            type = InstructionType::L_TYPE;
            // Extract the raw 4-bit immediate from bits [15:12]
            uint8_t rawImm = (Complete_Instruction >> 12) & 0xF;  // 4 bits

            // Sign-extend 4-bit immediate to 16 bits
            if (rawImm & 0x8)  // if sign bit (bit 3) is 1
                imm = rawImm | 0xFFF0;  // fill upper bits with 1s
            else
                imm = rawImm;

            rs2 = (Complete_Instruction >> 9) & 0x7;  //base register
            rd = (Complete_Instruction >> 6) & 0x7;
            func3 = (Complete_Instruction >> 3) & 0x7;
            break;
        }
        case 0b101: {
            // J-Type (jump)
            type = InstructionType::J_TYPE;
            // Extract individual parts of the immediate
            uint16_t imm_high = (Complete_Instruction >> 9) & 0x3F;  // bits [14:9] → imm[9:4]
            uint16_t imm_low  = (Complete_Instruction >> 3) & 0x7;   // bits [5:3]  → imm[3:1]
            // forming imm[9:1] and setting least bit to 0 by shifting
            uint16_t combined_imm = (imm_high << 4) | (imm_low << 1);  // shift low by 1 because imm[0] = 0

            // Sign-extend 10-bit immediate to 16 bits
            if (combined_imm & (1 << 9))  // if imm[9] is 1 → negative
                imm = combined_imm | 0xFC00;  // fill upper 6 bits with 1s
            else
                imm = combined_imm;

            // Extract rd (only used if flag == 1)
            rd = (Complete_Instruction >> 6) & 0x7;  // bits [8:6]

            // Extract link flag (bit 15)
            flag = (Complete_Instruction >> 15) &0x1;

            break;
        }case 0b110: {
            // U-Type
            type = InstructionType::U_TYPE;
            flag = (Complete_Instruction >> 15) & 0x1;
            uint16_t imm_high = (Complete_Instruction >> 9) & 0x3F; // [14:9] → imm[8:3]
            rd = (Complete_Instruction >> 6) & 0x7;                 // [8:6] → rd
            uint16_t imm_mid = (Complete_Instruction >> 3) & 0x7;   // [5:3] → imm[2:0]

            // Reconstruct the 9-bit immediate
            uint16_t imm9 = (imm_high << 3) | imm_mid;              // imm[8:0]

            // Sign-extend from bit 8 (MSB of 9-bit immediate)
            if (imm9 & 0x100)  // Test bit 8
                imm9 |= 0xFE00; // Sign extend to 16 bits

            imm = imm9;  // This is your final immediate value (e.g., 22)
            break;
        }
        case 0b111: {
            // SYS-Type (e.g., ecall)

            type = InstructionType::SYS_Type;
            svc = (Complete_Instruction >> 6) & 0x3FF; // 10 bits no sign extension

            break;
        }
        default:
            type = InstructionType::INVALID;
            break;
    }

    generateAssemblyString();
}

void Instruction::generateAssemblyString() {

    static const std::string regs[8] = {
        "t0", "ra", "sp", "s0", "s1", "t1", "a0", "a1"
    };
    std::ostringstream oss;

    switch (type) {
        case InstructionType::R_TYPE:
            if (func4 == 0x0 && func3 == 0x0)
                oss << "add " << regs[rd] << ", " << regs[rs2];
            else if (func4 == 0x1 && func3 == 0x0)
                oss << "sub " << regs[rd] << ", " << regs[rs2];
            else if (func4 == 0x2 && func3 == 0x1)
                oss << "slt " << regs[rd] << ", " << regs[rs2];
            else if (func4 == 0x3 && func3 == 0x2)
                oss << "sltu " << regs[rd] << ", " << regs[rs2];
            else if (func4 == 0x4 && func3 == 0x3)
                oss << "sll " << regs[rd] << ", " << regs[rs2];
            else if (func4 == 0x5 && func3 == 0x3)
                oss << "srl " << regs[rd] << ", " << regs[rs2];
            else if (func4 == 0x6 && func3 == 0x3)
                oss << "sra " << regs[rd] << ", " << regs[rs2];
            else if (func4 == 0x7 && func3 == 0x4)
                oss << "or " << regs[rd] << ", " << regs[rs2];
            else if (func4 == 0x8 && func3 == 0x5)
                oss << "and " << regs[rd] << ", " << regs[rs2];
            else if (func4 == 0x9 && func3 == 0x6)
                oss << "xor " << regs[rd] << ", " << regs[rs2];
            else if (func4 == 0xA && func3 == 0x7)
                oss << "mv " << regs[rd] << ", " << regs[rs2];
            else if (func4 == 0xB && func3 == 0x0)
                oss << "jr " << regs[rd];
            else if (func4 == 0xC && func3 == 0x0)
                oss << "jalr " << regs[rd] << ", " << regs[rs2];
            else
                oss << "unknown_rtype";
        break;

        case InstructionType::I_TYPE:
            if (func3 == 0b000)
                oss << "addi " << regs[rd] << ", " << imm;
            else if (func3 == 0b001)
                oss << "slti " << regs[rd] << ", " << imm;
            else if (func3 == 0b010)
                oss << "sltui " << regs[rd] << ", " << imm;
            else if (func3 == 0b011) {
                uint8_t imm_Identifier = (imm >> 4) & 0b111; // imm[6:4]
                if (imm_Identifier == 0b001)
                    oss << "slli " << regs[rd] << ", " << (imm & 0xF);
                else if (imm_Identifier == 0b010)
                    oss << "srli " << regs[rd] << ", " << (imm & 0xF);
                else if (imm_Identifier == 0b100)
                    oss << "srai " << regs[rd] << ", " << (imm & 0xF);
                else
                    oss << "unknown_shift_imm";
            }
            else if (func3 == 0b100)
                oss << "ori " << regs[rd] << ", " << imm;
            else if (func3 == 0b101)
                oss << "andi " << regs[rd] << ", " << imm;
            else if (func3 == 0b110)
                oss << "xori " << regs[rd] << ", " << imm;
            else if (func3 == 0b111)
                oss << "li " << regs[rd] << ", " << imm;
            else
                oss << "unknown_itype";
        break;

        case InstructionType::B_TYPE:
            // TODO: Add logic for B-Type decoding
            if(func3 ==0b000)
                oss << "beq " << regs[rs1] << ", " << regs[rs2] << ", "<< imm;
            else if(func3 ==0b001)
                oss << "bne " << regs[rs1] << ", " << regs[rs2] << ", "<< imm;
            else if(func3 ==0b010)
                oss << "bz " << regs[rs1] << ", " << regs[rs2] << ", "<< imm;
            else if(func3 ==0b011)
                oss << "bnz " << regs[rs1] << ", " << regs[rs2] << ", "<< imm;
            else if(func3 ==0b100)
                oss << "blt " << regs[rs1] << ", " << regs[rs2] << ", "<< imm;
            else if(func3 ==0b101)
                oss << "bge " << regs[rs1] << ", " << regs[rs2] << ", "<< imm;
            else if(func3 ==0b110)
                oss << "bltu " << regs[rs1] << ", " << regs[rs2] << ", "<< imm;
            else if(func3 ==0b111)
                oss << "bgeu " << regs[rs1] << ", " << regs[rs2] << ", "<< imm;
            else
                oss << "unknown_btype";
           
                break;

        case InstructionType::S_TYPE:
            // TODO: Add logic for S-Type decoding
            if(func3 == 0b000)
                oss << "sb " << regs[rs2] << ", " << imm << "(" << regs[rs1] << ")";
            else if(func3 == 0b001) 
                oss << "sw " << regs[rs2] << ", " << imm << "(" << regs[rs1] << ")";
            else
                oss << "unknown_stype";
                break;

        case InstructionType::U_TYPE:
            if (flag == 0)
                oss << "LUI " << regs[rd] << ", " << imm;
            else if (flag == 1)
                oss << "AUIPC " << regs[rd] << ", " << imm;

                break;

        case InstructionType::J_TYPE:
            if (flag == 0)
                oss << "j " << imm;
            else
                oss << "jal " << regs[rd] << ", " << imm;
        break;

        case InstructionType::L_TYPE:
            if (func3 == 0b000)
                oss << "lb " << regs[rd] << ", " << imm << "(" << regs[rs2] << ")";
            else if (func3 == 0b001)
                oss << "lw " << regs[rd] << ", " << imm << "(" << regs[rs2] << ")";
            else if (func3 == 0b100)
                oss << "lbu " << regs[rd] << ", " << imm << "(" << regs[rs2] << ")";
            else
                oss << "unknown_ltype";
        break;

        case InstructionType::SYS_Type:
                oss << "ecall " <<  svc;
                break;

        case InstructionType::INVALID:
            default:
                oss << "invalid";
        break;
    }


    Assembly_Code = oss.str();

}

// Getters
InstructionType Instruction::getType() const {
    return type;
}

std::string Instruction::AssemblyCode() const {
    return Assembly_Code;
}

uint16_t Instruction::get_CompleteInstruction() const {
    return Complete_Instruction;
}

uint8_t Instruction::getOpcode() const {
    return opcode;
}

uint8_t Instruction::getRd() const {
    return rd;
}

uint8_t Instruction:: getRs1 () const {

    return rs1;
}
uint8_t Instruction::getRs2() const {
    return rs2;
}

int16_t Instruction::getImmediate() const {
    return imm;
}

uint8_t Instruction::getFunc3() const {
    return func3;
}

uint8_t Instruction::getFunc4() const {
    return func4;
}

uint16_t Instruction::getSvc() const {
    return svc;
}

uint8_t Instruction::getflag() const {
    return flag;
}