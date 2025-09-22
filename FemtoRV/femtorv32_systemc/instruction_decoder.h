/**
 * @file instruction_decoder.h
 * @brief RISC-V Instruction Decoder SystemC Module
 * 
 * Implements instruction decoding for RV32IMC instruction set with:
 * - Instruction format detection
 * - Immediate value extraction
 * - Operation type classification
 * - Compressed instruction support
 */

#ifndef INSTRUCTION_DECODER_H
#define INSTRUCTION_DECODER_H

#include "femtorv32_systemc.h"

/**
 * @brief Instruction Decoder Module
 * 
 * Decodes RISC-V instructions and extracts all necessary fields
 * for execution by the processor.
 */
class InstructionDecoder : public sc_module {
public:
    // Ports
    sc_in<sc_uint<32>> instruction;
    sc_in<bool> is_compressed;
    sc_in<bool> decode_enable;
    
    // Decoded fields
    sc_out<sc_uint<5>> rd;
    sc_out<sc_uint<5>> rs1;
    sc_out<sc_uint<5>> rs2;
    sc_out<sc_uint<3>> funct3;
    sc_out<sc_uint<7>> funct7;
    sc_out<sc_uint<7>> opcode;
    
    // Immediate values
    sc_out<sc_uint<32>> i_imm;
    sc_out<sc_uint<32>> s_imm;
    sc_out<sc_uint<32>> b_imm;
    sc_out<sc_uint<32>> u_imm;
    sc_out<sc_uint<32>> j_imm;
    
    // Instruction classification
    sc_out<InstructionType> instruction_type;
    sc_out<InstructionFormat> instruction_format;
    sc_out<ALUOperation> alu_operation;
    sc_out<BranchCondition> branch_condition;
    sc_out<MemoryAccessType> memory_access_type;
    
    // Control signals
    sc_out<bool> is_load;
    sc_out<bool> is_store;
    sc_out<bool> is_alu_reg;
    sc_out<bool> is_alu_imm;
    sc_out<bool> is_branch;
    sc_out<bool> is_jal;
    sc_out<bool> is_jalr;
    sc_out<bool> is_lui;
    sc_out<bool> is_auipc;
    sc_out<bool> is_system;
    
    // Constructor
    SC_CTOR(InstructionDecoder) {
        SC_METHOD(decode_instruction);
        sensitive << instruction << is_compressed << decode_enable;
    }
    
private:
    // Process methods
    void decode_instruction();
    void decode_compressed_instruction();
    void decode_standard_instruction();
    
    // Helper methods
    sc_uint<32> extract_i_imm(sc_uint<32> instr);
    sc_uint<32> extract_s_imm(sc_uint<32> instr);
    sc_uint<32> extract_b_imm(sc_uint<32> instr);
    sc_uint<32> extract_u_imm(sc_uint<32> instr);
    sc_uint<32> extract_j_imm(sc_uint<32> instr);
    
    ALUOperation decode_alu_operation(sc_uint<7> funct7, sc_uint<3> funct3, bool is_imm);
    BranchCondition decode_branch_condition(sc_uint<3> funct3);
    MemoryAccessType decode_memory_access_type(sc_uint<3> funct3);
};

/**
 * @brief Compressed Instruction Decoder Module
 * 
 * Decodes RISC-V compressed (16-bit) instructions and expands
 * them to 32-bit standard instructions.
 */
class CompressedInstructionDecoder : public sc_module {
public:
    // Ports
    sc_in<sc_uint<16>> compressed_instruction;
    sc_out<sc_uint<32>> decompressed_instruction;
    sc_out<bool> is_compressed;
    sc_out<CompressedInstructionType> compressed_type;
    
    // Constructor
    SC_CTOR(CompressedInstructionDecoder) {
        SC_METHOD(decompress_instruction);
        sensitive << compressed_instruction;
    }
    
private:
    // Process methods
    void decompress_instruction();
    
    // Helper methods
    sc_uint<5> decode_compressed_register(sc_uint<3> reg_field, bool is_high);
    sc_uint<32> decompress_addi4spn(sc_uint<16> instr);
    sc_uint<32> decompress_lw(sc_uint<16> instr);
    sc_uint<32> decompress_sw(sc_uint<16> instr);
    sc_uint<32> decompress_addi(sc_uint<16> instr);
    sc_uint<32> decompress_jal(sc_uint<16> instr);
    sc_uint<32> decompress_li(sc_uint<16> instr);
    sc_uint<32> decompress_addi16sp(sc_uint<16> instr);
    sc_uint<32> decompress_lui(sc_uint<16> instr);
    sc_uint<32> decompress_srli(sc_uint<16> instr);
    sc_uint<32> decompress_srai(sc_uint<16> instr);
    sc_uint<32> decompress_andi(sc_uint<16> instr);
    sc_uint<32> decompress_sub(sc_uint<16> instr);
    sc_uint<32> decompress_xor(sc_uint<16> instr);
    sc_uint<32> decompress_or(sc_uint<16> instr);
    sc_uint<32> decompress_and(sc_uint<16> instr);
    sc_uint<32> decompress_j(sc_uint<16> instr);
    sc_uint<32> decompress_beqz(sc_uint<16> instr);
    sc_uint<32> decompress_bnez(sc_uint<16> instr);
    sc_uint<32> decompress_slli(sc_uint<16> instr);
    sc_uint<32> decompress_lwsp(sc_uint<16> instr);
    sc_uint<32> decompress_jr(sc_uint<16> instr);
    sc_uint<32> decompress_mv(sc_uint<16> instr);
    sc_uint<32> decompress_jalr(sc_uint<16> instr);
    sc_uint<32> decompress_add(sc_uint<16> instr);
    sc_uint<32> decompress_swsp(sc_uint<16> instr);
};

#endif // INSTRUCTION_DECODER_H
