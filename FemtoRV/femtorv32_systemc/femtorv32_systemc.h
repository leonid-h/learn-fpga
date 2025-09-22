/**
 * @file femtorv32_systemc.h
 * @brief FemtoRV32 RISC-V Processor SystemC Implementation
 * 
 * This is a production-quality SystemC translation of the FemtoRV32 Verilog processor.
 * Implements RV32IMC instruction set with interrupt support and compressed instructions.
 * 
 * Features:
 * - RV32IMC instruction set (RISC-V 32-bit Integer, Multiply, Compressed)
 * - Interrupt support with CSR registers
 * - Compressed instruction decompression
 * - Barrel shifter and multiplication/division
 * - Memory-mapped I/O
 * - Configurable address width and reset address
 * 
 * @author SystemC Translation of FemtoRV32 by Bruno Levy, Matthias Koch
 * @version 1.0
 * @date 2024
 */

#ifndef FEMTORV32_SYSTEMC_H
#define FEMTORV32_SYSTEMC_H

#include <systemc>
#include <vector>
#include <cstdint>
#include <string>
#include <map>
#include <functional>

using namespace sc_core;
using namespace sc_dt;

// Forward declarations
class FemtoRV32Core;
class MemoryInterface;
class ALU;
class RegisterFile;
class InstructionDecoder;
class CompressedInstructionDecoder;
class CSRRegisters;
class InterruptController;

/**
 * @brief Memory interface signals
 */
struct MemorySignals {
    sc_signal<sc_uint<32>> address;
    sc_signal<sc_uint<32>> write_data;
    sc_signal<sc_uint<4>>  write_mask;
    sc_signal<sc_uint<32>> read_data;
    sc_signal<bool>        read_strobe;
    sc_signal<bool>        read_busy;
    sc_signal<bool>        write_busy;
};

/**
 * @brief Interrupt signals
 */
struct InterruptSignals {
    sc_signal<bool> interrupt_request;
    sc_signal<bool> interrupt_ack;
    sc_signal<sc_uint<4>> interrupt_cause;
};

/**
 * @brief Configuration parameters
 */
struct ProcessorConfig {
    static constexpr uint32_t RESET_ADDR = 0x00000000;
    static constexpr int ADDR_WIDTH = 24;
    static constexpr int REG_COUNT = 32;
    static constexpr int CSR_COUNT = 16;
    static constexpr int MEMORY_SIZE = 64 * 1024; // 64KB default
};

/**
 * @brief RISC-V instruction formats
 */
enum class InstructionFormat {
    R_TYPE,  // Register-Register
    I_TYPE,  // Immediate
    S_TYPE,  // Store
    B_TYPE,  // Branch
    U_TYPE,  // Upper Immediate
    J_TYPE   // Jump
};

/**
 * @brief RISC-V instruction types
 */
enum class InstructionType {
    // Load/Store
    LOAD,
    STORE,
    
    // ALU Operations
    ALU_REG,
    ALU_IMM,
    
    // Control Flow
    BRANCH,
    JAL,
    JALR,
    
    // System
    SYSTEM,
    
    // Upper Immediate
    LUI,
    AUIPC,
    
    // Compressed
    COMPRESSED
};

/**
 * @brief ALU operations
 */
enum class ALUOperation {
    ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
    MUL, MULH, MULHSU, MULHU, DIV, DIVU, REM, REMU
};

/**
 * @brief Branch conditions
 */
enum class BranchCondition {
    BEQ, BNE, BLT, BGE, BLTU, BGEU
};

/**
 * @brief Memory access types
 */
enum class MemoryAccessType {
    BYTE, HALFWORD, WORD
};

/**
 * @brief CSR register addresses
 */
namespace CSR {
    constexpr uint32_t MSTATUS = 0x300;
    constexpr uint32_t MTVEC   = 0x305;
    constexpr uint32_t MEPC    = 0x341;
    constexpr uint32_t MCAUSE  = 0x342;
    constexpr uint32_t CYCLES  = 0xC00;
    constexpr uint32_t CYCLESH = 0xC80;
}

/**
 * @brief Processor states
 */
enum class ProcessorState {
    FETCH_INSTR,
    WAIT_INSTR,
    EXECUTE,
    WAIT_ALU_OR_MEM,
    WAIT_ALU_OR_MEM_SKIP
};

/**
 * @brief Compressed instruction types
 */
enum class CompressedInstructionType {
    C_ADDI4SPN, C_LW, C_SW, C_ADDI, C_JAL, C_LI, C_ADDI16SP, C_LUI,
    C_SRLI, C_SRAI, C_ANDI, C_SUB, C_XOR, C_OR, C_AND, C_J, C_BEQZ,
    C_BNEZ, C_SLLI, C_LWSP, C_JR, C_MV, C_JALR, C_ADD, C_SWSP,
    C_ILLEGAL, C_UNKNOWN
};

#endif // FEMTORV32_SYSTEMC_H
