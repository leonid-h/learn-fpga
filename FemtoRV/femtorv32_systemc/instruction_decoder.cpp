/**
 * @file instruction_decoder.cpp
 * @brief RISC-V Instruction Decoder SystemC Module Implementation
 */

#include "instruction_decoder.h"

void InstructionDecoder::decode_instruction() {
    if (!decode_enable.read()) {
        return;
    }
    
    if (is_compressed.read()) {
        decode_compressed_instruction();
    } else {
        decode_standard_instruction();
    }
}

void InstructionDecoder::decode_compressed_instruction() {
    // For now, just pass through to standard decoder
    // In a full implementation, this would decompress the instruction
    decode_standard_instruction();
}

void InstructionDecoder::decode_standard_instruction() {
    sc_uint<32> instr = instruction.read();
    
    // Extract basic fields
    rd.write(instr.range(11, 7));
    rs1.write(instr.range(19, 15));
    rs2.write(instr.range(24, 20));
    funct3.write(instr.range(14, 12));
    funct7.write(instr.range(31, 25));
    opcode.write(instr.range(6, 0));
    
    // Extract immediate values
    i_imm.write(extract_i_imm(instr));
    s_imm.write(extract_s_imm(instr));
    b_imm.write(extract_b_imm(instr));
    u_imm.write(extract_u_imm(instr));
    j_imm.write(extract_j_imm(instr));
    
    // Classify instruction
    sc_uint<7> op = instr.range(6, 0);
    sc_uint<3> func3 = instr.range(14, 12);
    sc_uint<7> func7 = instr.range(31, 25);
    
    // Set instruction type flags
    is_load.write(op == 0x03);
    is_store.write(op == 0x23);
    is_alu_reg.write(op == 0x33);
    is_alu_imm.write(op == 0x13);
    is_branch.write(op == 0x63);
    is_jal.write(op == 0x6F);
    is_jalr.write(op == 0x67);
    is_lui.write(op == 0x37);
    is_auipc.write(op == 0x17);
    is_system.write(op == 0x73);
    
    // Determine instruction type
    if (op == 0x03) {
        instruction_type.write(InstructionType::LOAD);
        instruction_format.write(InstructionFormat::I_TYPE);
        memory_access_type.write(decode_memory_access_type(func3));
    } else if (op == 0x23) {
        instruction_type.write(InstructionType::STORE);
        instruction_format.write(InstructionFormat::S_TYPE);
        memory_access_type.write(decode_memory_access_type(func3));
    } else if (op == 0x33) {
        instruction_type.write(InstructionType::ALU_REG);
        instruction_format.write(InstructionFormat::R_TYPE);
        alu_operation.write(decode_alu_operation(func7, func3, false));
    } else if (op == 0x13) {
        instruction_type.write(InstructionType::ALU_IMM);
        instruction_format.write(InstructionFormat::I_TYPE);
        alu_operation.write(decode_alu_operation(func7, func3, true));
    } else if (op == 0x63) {
        instruction_type.write(InstructionType::BRANCH);
        instruction_format.write(InstructionFormat::B_TYPE);
        branch_condition.write(decode_branch_condition(func3));
    } else if (op == 0x6F) {
        instruction_type.write(InstructionType::JAL);
        instruction_format.write(InstructionFormat::J_TYPE);
    } else if (op == 0x67) {
        instruction_type.write(InstructionType::JALR);
        instruction_format.write(InstructionFormat::I_TYPE);
    } else if (op == 0x37) {
        instruction_type.write(InstructionType::LUI);
        instruction_format.write(InstructionFormat::U_TYPE);
    } else if (op == 0x17) {
        instruction_type.write(InstructionType::AUIPC);
        instruction_format.write(InstructionFormat::U_TYPE);
    } else if (op == 0x73) {
        instruction_type.write(InstructionType::SYSTEM);
        instruction_format.write(InstructionFormat::I_TYPE);
    } else {
        instruction_type.write(InstructionType::LOAD); // Default
        instruction_format.write(InstructionFormat::I_TYPE);
    }
}

sc_uint<32> InstructionDecoder::extract_i_imm(sc_uint<32> instr) {
    return sc_int<32>(sc_int<12>(instr.range(31, 20)));
}

sc_uint<32> InstructionDecoder::extract_s_imm(sc_uint<32> instr) {
    return sc_int<32>(sc_int<12>(instr.range(31, 25) << 5 | instr.range(11, 7)));
}

sc_uint<32> InstructionDecoder::extract_b_imm(sc_uint<32> instr) {
    sc_uint<13> b_imm = (instr[31] << 12) | (instr[7] << 11) | 
                       (instr.range(30, 25) << 5) | (instr.range(11, 8) << 1);
    return sc_int<32>(sc_int<13>(b_imm));
}

sc_uint<32> InstructionDecoder::extract_u_imm(sc_uint<32> instr) {
    return instr.range(31, 12) << 12;
}

sc_uint<32> InstructionDecoder::extract_j_imm(sc_uint<32> instr) {
    sc_uint<21> j_imm = (instr[31] << 20) | (instr.range(19, 12) << 12) |
                       (instr[20] << 11) | (instr.range(30, 21) << 1);
    return sc_int<32>(sc_int<21>(j_imm));
}

ALUOperation InstructionDecoder::decode_alu_operation(sc_uint<7> funct7, sc_uint<3> funct3, bool is_imm) {
    if (funct7 == 0x00) {
        switch (funct3) {
            case 0x0: return is_imm ? ALUOperation::ADD : ALUOperation::ADD;
            case 0x1: return ALUOperation::SLL;
            case 0x2: return ALUOperation::SLT;
            case 0x3: return ALUOperation::SLTU;
            case 0x4: return ALUOperation::XOR;
            case 0x5: return ALUOperation::SRL;
            case 0x6: return ALUOperation::OR;
            case 0x7: return ALUOperation::AND;
        }
    } else if (funct7 == 0x20) {
        switch (funct3) {
            case 0x0: return ALUOperation::SUB;
            case 0x5: return ALUOperation::SRA;
        }
    } else if (funct7 == 0x01) {
        switch (funct3) {
            case 0x0: return ALUOperation::MUL;
            case 0x1: return ALUOperation::MULH;
            case 0x2: return ALUOperation::MULHSU;
            case 0x3: return ALUOperation::MULHU;
            case 0x4: return ALUOperation::DIV;
            case 0x5: return ALUOperation::DIVU;
            case 0x6: return ALUOperation::REM;
            case 0x7: return ALUOperation::REMU;
        }
    }
    return ALUOperation::ADD; // Default
}

BranchCondition InstructionDecoder::decode_branch_condition(sc_uint<3> funct3) {
    switch (funct3) {
        case 0x0: return BranchCondition::BEQ;
        case 0x1: return BranchCondition::BNE;
        case 0x4: return BranchCondition::BLT;
        case 0x5: return BranchCondition::BGE;
        case 0x6: return BranchCondition::BLTU;
        case 0x7: return BranchCondition::BGEU;
        default: return BranchCondition::BEQ;
    }
}

MemoryAccessType InstructionDecoder::decode_memory_access_type(sc_uint<3> funct3) {
    switch (funct3) {
        case 0x0: return MemoryAccessType::BYTE;
        case 0x1: return MemoryAccessType::HALFWORD;
        case 0x2: return MemoryAccessType::WORD;
        default: return MemoryAccessType::WORD;
    }
}

// Compressed Instruction Decoder Implementation
void CompressedInstructionDecoder::decompress_instruction() {
    sc_uint<16> comp_instr = compressed_instruction.read();
    
    // Check if it's a compressed instruction (bits 1:0 != 11)
    if (comp_instr.range(1, 0) != 0x3) {
        is_compressed.write(true);
        
        // Decode compressed instruction type
        sc_uint<2> op = comp_instr.range(1, 0);
        sc_uint<3> funct3 = comp_instr.range(15, 13);
        
        if (op == 0x0) {
            if (funct3 == 0x0) {
                compressed_type.write(CompressedInstructionType::C_ADDI4SPN);
                decompressed_instruction.write(decompress_addi4spn(comp_instr));
            } else if (funct3 == 0x2) {
                compressed_type.write(CompressedInstructionType::C_LW);
                decompressed_instruction.write(decompress_lw(comp_instr));
            } else if (funct3 == 0x6) {
                compressed_type.write(CompressedInstructionType::C_SW);
                decompressed_instruction.write(decompress_sw(comp_instr));
            }
        } else if (op == 0x1) {
            if (funct3 == 0x0) {
                compressed_type.write(CompressedInstructionType::C_ADDI);
                decompressed_instruction.write(decompress_addi(comp_instr));
            } else if (funct3 == 0x1) {
                compressed_type.write(CompressedInstructionType::C_JAL);
                decompressed_instruction.write(decompress_jal(comp_instr));
            } else if (funct3 == 0x2) {
                compressed_type.write(CompressedInstructionType::C_LI);
                decompressed_instruction.write(decompress_li(comp_instr));
            } else if (funct3 == 0x3) {
                if (comp_instr.range(11, 7) == 0x2) {
                    compressed_type.write(CompressedInstructionType::C_ADDI16SP);
                    decompressed_instruction.write(decompress_addi16sp(comp_instr));
                } else {
                    compressed_type.write(CompressedInstructionType::C_LUI);
                    decompressed_instruction.write(decompress_lui(comp_instr));
                }
            }
        } else if (op == 0x2) {
            if (funct3 == 0x0) {
                compressed_type.write(CompressedInstructionType::C_SLLI);
                decompressed_instruction.write(decompress_slli(comp_instr));
            } else if (funct3 == 0x2) {
                compressed_type.write(CompressedInstructionType::C_LWSP);
                decompressed_instruction.write(decompress_lwsp(comp_instr));
            } else if (funct3 == 0x4) {
                if (comp_instr.range(11, 7) == 0x0) {
                    compressed_type.write(CompressedInstructionType::C_JR);
                    decompressed_instruction.write(decompress_jr(comp_instr));
                } else {
                    compressed_type.write(CompressedInstructionType::C_MV);
                    decompressed_instruction.write(decompress_mv(comp_instr));
                }
            } else if (funct3 == 0x6) {
                if (comp_instr.range(11, 7) == 0x0) {
                    compressed_type.write(CompressedInstructionType::C_JALR);
                    decompressed_instruction.write(decompress_jalr(comp_instr));
                } else {
                    compressed_type.write(CompressedInstructionType::C_ADD);
                    decompressed_instruction.write(decompress_add(comp_instr));
                }
            } else if (funct3 == 0x6) {
                compressed_type.write(CompressedInstructionType::C_SWSP);
                decompressed_instruction.write(decompress_swsp(comp_instr));
            }
        }
    } else {
        // Not a compressed instruction
        is_compressed.write(false);
        compressed_type.write(CompressedInstructionType::C_UNKNOWN);
        decompressed_instruction.write(comp_instr.range(15, 0) << 16 | comp_instr.range(15, 0));
    }
}

// Compressed instruction decompression methods
sc_uint<5> CompressedInstructionDecoder::decode_compressed_register(sc_uint<3> reg_field, bool is_high) {
    return (is_high ? 0x8 : 0x0) | reg_field;
}

sc_uint<32> CompressedInstructionDecoder::decompress_addi4spn(sc_uint<16> instr) {
    sc_uint<10> imm = instr.range(12, 5) << 2;
    sc_uint<5> rd = decode_compressed_register(instr.range(4, 2), false);
    return (imm << 20) | (0x2 << 15) | (0x0 << 12) | (rd << 7) | 0x13;
}

sc_uint<32> CompressedInstructionDecoder::decompress_lw(sc_uint<16> instr) {
    sc_uint<7> imm = (instr.range(12, 10) << 3) | (instr.range(6, 5) << 1);
    sc_uint<5> rs1 = decode_compressed_register(instr.range(9, 7), false);
    sc_uint<5> rd = decode_compressed_register(instr.range(4, 2), false);
    return (imm << 20) | (rs1 << 15) | (0x2 << 12) | (rd << 7) | 0x03;
}

sc_uint<32> CompressedInstructionDecoder::decompress_sw(sc_uint<16> instr) {
    sc_uint<7> imm = (instr.range(12, 10) << 3) | (instr.range(6, 5) << 1);
    sc_uint<5> rs1 = decode_compressed_register(instr.range(9, 7), false);
    sc_uint<5> rs2 = decode_compressed_register(instr.range(4, 2), false);
    return (imm.range(6, 5) << 25) | (rs2 << 20) | (rs1 << 15) | (0x2 << 12) | (imm.range(4, 0) << 7) | 0x23;
}

sc_uint<32> CompressedInstructionDecoder::decompress_addi(sc_uint<16> instr) {
    sc_uint<6> imm = sc_int<6>(sc_int<6>(instr.range(12, 2)));
    sc_uint<5> rd = instr.range(11, 7);
    return (imm << 20) | (rd << 15) | (0x0 << 12) | (rd << 7) | 0x13;
}

sc_uint<32> CompressedInstructionDecoder::decompress_jal(sc_uint<16> instr) {
    sc_uint<12> imm = (instr[12] << 11) | (instr.range(8, 1) << 1);
    return (sc_int<21>(sc_int<12>(imm)) << 20) | (0x1 << 7) | 0x6F;
}

sc_uint<32> CompressedInstructionDecoder::decompress_li(sc_uint<16> instr) {
    sc_uint<6> imm = sc_int<6>(sc_int<6>(instr.range(12, 2)));
    sc_uint<5> rd = instr.range(11, 7);
    return (imm << 20) | (0x0 << 15) | (0x0 << 12) | (rd << 7) | 0x13;
}

sc_uint<32> CompressedInstructionDecoder::decompress_addi16sp(sc_uint<16> instr) {
    sc_uint<6> imm = sc_int<6>(sc_int<6>(instr.range(12, 2)));
    return (imm << 20) | (0x2 << 15) | (0x0 << 12) | (0x2 << 7) | 0x13;
}

sc_uint<32> CompressedInstructionDecoder::decompress_lui(sc_uint<16> instr) {
    sc_uint<6> imm = sc_int<6>(sc_int<6>(instr.range(12, 2)));
    sc_uint<5> rd = instr.range(11, 7);
    return (imm << 20) | (rd << 7) | 0x37;
}

sc_uint<32> CompressedInstructionDecoder::decompress_srli(sc_uint<16> instr) {
    sc_uint<5> shamt = instr.range(12, 7);
    sc_uint<5> rd = decode_compressed_register(instr.range(9, 7), false);
    return (0x0 << 25) | (shamt << 20) | (rd << 15) | (0x5 << 12) | (rd << 7) | 0x13;
}

sc_uint<32> CompressedInstructionDecoder::decompress_srai(sc_uint<16> instr) {
    sc_uint<5> shamt = instr.range(12, 7);
    sc_uint<5> rd = decode_compressed_register(instr.range(9, 7), false);
    return (0x20 << 25) | (shamt << 20) | (rd << 15) | (0x5 << 12) | (rd << 7) | 0x13;
}

sc_uint<32> CompressedInstructionDecoder::decompress_andi(sc_uint<16> instr) {
    sc_uint<6> imm = sc_int<6>(sc_int<6>(instr.range(12, 2)));
    sc_uint<5> rd = decode_compressed_register(instr.range(9, 7), false);
    return (imm << 20) | (rd << 15) | (0x7 << 12) | (rd << 7) | 0x13;
}

sc_uint<32> CompressedInstructionDecoder::decompress_sub(sc_uint<16> instr) {
    sc_uint<5> rs2 = decode_compressed_register(instr.range(4, 2), false);
    sc_uint<5> rd = decode_compressed_register(instr.range(9, 7), false);
    return (0x20 << 25) | (rs2 << 20) | (rd << 15) | (0x0 << 12) | (rd << 7) | 0x33;
}

sc_uint<32> CompressedInstructionDecoder::decompress_xor(sc_uint<16> instr) {
    sc_uint<5> rs2 = decode_compressed_register(instr.range(4, 2), false);
    sc_uint<5> rd = decode_compressed_register(instr.range(9, 7), false);
    return (0x0 << 25) | (rs2 << 20) | (rd << 15) | (0x4 << 12) | (rd << 7) | 0x33;
}

sc_uint<32> CompressedInstructionDecoder::decompress_or(sc_uint<16> instr) {
    sc_uint<5> rs2 = decode_compressed_register(instr.range(4, 2), false);
    sc_uint<5> rd = decode_compressed_register(instr.range(9, 7), false);
    return (0x0 << 25) | (rs2 << 20) | (rd << 15) | (0x6 << 12) | (rd << 7) | 0x33;
}

sc_uint<32> CompressedInstructionDecoder::decompress_and(sc_uint<16> instr) {
    sc_uint<5> rs2 = decode_compressed_register(instr.range(4, 2), false);
    sc_uint<5> rd = decode_compressed_register(instr.range(9, 7), false);
    return (0x0 << 25) | (rs2 << 20) | (rd << 15) | (0x7 << 12) | (rd << 7) | 0x33;
}

sc_uint<32> CompressedInstructionDecoder::decompress_j(sc_uint<16> instr) {
    sc_uint<12> imm = (instr[12] << 11) | (instr.range(8, 1) << 1);
    return (sc_int<21>(sc_int<12>(imm)) << 20) | (0x0 << 7) | 0x6F;
}

sc_uint<32> CompressedInstructionDecoder::decompress_beqz(sc_uint<16> instr) {
    sc_uint<9> imm = (instr[12] << 8) | (instr.range(6, 5) << 6) | (instr.range(4, 2) << 3) | (instr.range(11, 10) << 1);
    sc_uint<5> rs1 = decode_compressed_register(instr.range(9, 7), false);
    return (sc_int<13>(sc_int<9>(imm)) << 20) | (0x0 << 15) | (rs1 << 15) | (0x0 << 12) | (imm.range(4, 1) << 7) | (imm[8] << 7) | 0x63;
}

sc_uint<32> CompressedInstructionDecoder::decompress_bnez(sc_uint<16> instr) {
    sc_uint<9> imm = (instr[12] << 8) | (instr.range(6, 5) << 6) | (instr.range(4, 2) << 3) | (instr.range(11, 10) << 1);
    sc_uint<5> rs1 = decode_compressed_register(instr.range(9, 7), false);
    return (sc_int<13>(sc_int<9>(imm)) << 20) | (0x0 << 15) | (rs1 << 15) | (0x1 << 12) | (imm.range(4, 1) << 7) | (imm[8] << 7) | 0x63;
}

sc_uint<32> CompressedInstructionDecoder::decompress_slli(sc_uint<16> instr) {
    sc_uint<5> shamt = instr.range(12, 7);
    sc_uint<5> rd = instr.range(11, 7);
    return (0x0 << 25) | (shamt << 20) | (rd << 15) | (0x1 << 12) | (rd << 7) | 0x13;
}

sc_uint<32> CompressedInstructionDecoder::decompress_lwsp(sc_uint<16> instr) {
    sc_uint<8> imm = (instr.range(12, 9) << 4) | (instr.range(4, 2) << 1);
    sc_uint<5> rd = instr.range(11, 7);
    return (imm << 20) | (0x2 << 15) | (0x2 << 12) | (rd << 7) | 0x03;
}

sc_uint<32> CompressedInstructionDecoder::decompress_jr(sc_uint<16> instr) {
    sc_uint<5> rs1 = instr.range(11, 7);
    return (0x0 << 20) | (rs1 << 15) | (0x0 << 12) | (0x0 << 7) | 0x67;
}

sc_uint<32> CompressedInstructionDecoder::decompress_mv(sc_uint<16> instr) {
    sc_uint<5> rs2 = instr.range(6, 2);
    sc_uint<5> rd = instr.range(11, 7);
    return (0x0 << 25) | (rs2 << 20) | (0x0 << 15) | (0x0 << 12) | (rd << 7) | 0x33;
}

sc_uint<32> CompressedInstructionDecoder::decompress_jalr(sc_uint<16> instr) {
    sc_uint<5> rs1 = instr.range(11, 7);
    return (0x0 << 20) | (rs1 << 15) | (0x0 << 12) | (0x1 << 7) | 0x67;
}

sc_uint<32> CompressedInstructionDecoder::decompress_add(sc_uint<16> instr) {
    sc_uint<5> rs2 = instr.range(6, 2);
    sc_uint<5> rd = instr.range(11, 7);
    return (0x0 << 25) | (rs2 << 20) | (rd << 15) | (0x0 << 12) | (rd << 7) | 0x33;
}

sc_uint<32> CompressedInstructionDecoder::decompress_swsp(sc_uint<16> instr) {
    sc_uint<8> imm = (instr.range(12, 9) << 4) | (instr.range(4, 2) << 1);
    sc_uint<5> rs2 = instr.range(6, 2);
    return (imm.range(7, 4) << 25) | (rs2 << 20) | (0x2 << 15) | (0x2 << 12) | (imm.range(3, 0) << 7) | 0x23;
}
