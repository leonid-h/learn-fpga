/**
 * @file femtorv32_core.h
 * @brief FemtoRV32 Core SystemC Module
 * 
 * Main processor core implementing the complete RISC-V processor
 * with all components integrated together.
 */

#ifndef FEMTORV32_CORE_H
#define FEMTORV32_CORE_H

#include "femtorv32_systemc.h"
#include "register_file.h"
#include "alu.h"
#include "instruction_decoder.h"
#include "csr_registers.h"
#include "memory_interface.h"

/**
 * @brief FemtoRV32 Core Module
 * 
 * Main processor core that integrates all components:
 * - Register file
 * - ALU
 * - Instruction decoder
 * - CSR registers
 * - Memory interface
 * - Control logic
 */
class FemtoRV32Core : public sc_module {
public:
    // Ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    
    // Memory interface
    sc_out<sc_uint<32>> mem_address;
    sc_out<sc_uint<32>> mem_write_data;
    sc_out<sc_uint<4>> mem_write_mask;
    sc_in<sc_uint<32>> mem_read_data;
    sc_out<bool> mem_read_strobe;
    sc_in<bool> mem_read_busy;
    sc_out<bool> mem_write_strobe;
    sc_in<bool> mem_write_busy;
    
    // Interrupt interface
    sc_in<bool> interrupt_request;
    
    // Debug outputs
    sc_out<sc_uint<32>> pc_debug;
    sc_out<sc_uint<32>> instruction_debug;
    sc_out<ProcessorState> state_debug;
    
    // Constructor
    SC_CTOR(FemtoRV32Core) {
        // Register processes
        SC_METHOD(processor_control);
        sensitive << clk.pos();
        
        SC_METHOD(instruction_fetch);
        sensitive << clk.pos();
        
        SC_METHOD(instruction_execute);
        sensitive << clk.pos();
        
        SC_METHOD(memory_operations);
        sensitive << clk.pos();
        
        // Initialize internal signals
        pc = ProcessorConfig::RESET_ADDR;
        instruction = 0;
        state = ProcessorState::FETCH_INSTR;
        cycles = 0;
        
        // Initialize outputs
        mem_address.write(0);
        mem_write_data.write(0);
        mem_write_mask.write(0);
        mem_read_strobe.write(false);
        mem_write_strobe.write(false);
        pc_debug.write(0);
        instruction_debug.write(0);
        state_debug.write(ProcessorState::FETCH_INSTR);
    }
    
    // Configuration
    void set_reset_address(sc_uint<32> addr) {
        reset_address = addr;
    }
    
    void set_address_width(int width) {
        address_width = width;
    }
    
private:
    // Internal state
    sc_uint<32> pc;
    sc_uint<32> instruction;
    ProcessorState state;
    sc_uint<64> cycles;
    sc_uint<32> reset_address;
    int address_width;
    
    // Component instances
    RegisterFile* reg_file;
    ALU* alu;
    InstructionDecoder* instr_decoder;
    CompressedInstructionDecoder* comp_decoder;
    CSRRegisters* csr_regs;
    
    // Internal signals
    sc_signal<sc_uint<5>> rs1_addr, rs2_addr, rd_addr;
    sc_signal<sc_uint<32>> rs1_data, rs2_data, rd_data;
    sc_signal<bool> reg_write_enable;
    
    sc_signal<sc_uint<32>> alu_operand1, alu_operand2;
    sc_signal<ALUOperation> alu_operation;
    sc_signal<bool> alu_start;
    sc_signal<sc_uint<32>> alu_result;
    sc_signal<bool> alu_busy, alu_valid;
    
    sc_signal<sc_uint<32>> decoded_instruction;
    sc_signal<bool> is_compressed;
    sc_signal<InstructionType> instruction_type;
    sc_signal<ALUOperation> decoded_alu_op;
    
    sc_signal<bool> csr_read_enable, csr_write_enable;
    sc_signal<sc_uint<12>> csr_address;
    sc_signal<sc_uint<32>> csr_write_data, csr_read_data;
    
    // Process methods
    void processor_control();
    void instruction_fetch();
    void instruction_execute();
    void memory_operations();
    
    // Helper methods
    void reset_processor();
    void fetch_instruction();
    void execute_instruction();
    void handle_memory_access();
    void handle_branch();
    void handle_jump();
    void handle_system_instruction();
    void handle_interrupt();
    
    // Instruction execution helpers
    sc_uint<32> get_immediate_value();
    sc_uint<32> compute_branch_target();
    sc_uint<32> compute_jump_target();
    bool evaluate_branch_condition();
    void update_pc();
    
    // Memory access helpers
    sc_uint<32> load_data(sc_uint<32> address, MemoryAccessType access_type);
    void store_data(sc_uint<32> address, sc_uint<32> data, MemoryAccessType access_type);
    sc_uint<4> get_store_mask(sc_uint<32> address, MemoryAccessType access_type);
};

#endif // FEMTORV32_CORE_H
