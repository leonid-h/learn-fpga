/**
 * @file csr_registers.h
 * @brief RISC-V Control and Status Registers SystemC Module
 * 
 * Implements the CSR (Control and Status Registers) for RISC-V
 * with support for machine-level registers and interrupt handling.
 */

#ifndef CSR_REGISTERS_H
#define CSR_REGISTERS_H

#include "femtorv32_systemc.h"

/**
 * @brief CSR Registers Module
 * 
 * Implements the Control and Status Registers including:
 * - mstatus: Machine status register
 * - mtvec: Machine trap vector base address
 * - mepc: Machine exception program counter
 * - mcause: Machine cause register
 * - cycles: Cycle counter
 * - cycleh: High part of cycle counter
 */
class CSRRegisters : public sc_module {
public:
    // Ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    
    // CSR access
    sc_in<bool> csr_read_enable;
    sc_in<bool> csr_write_enable;
    sc_in<sc_uint<12>> csr_address;
    sc_in<sc_uint<32>> csr_write_data;
    sc_out<sc_uint<32>> csr_read_data;
    
    // Interrupt control
    sc_in<bool> interrupt_request;
    sc_out<bool> interrupt_enable;
    sc_out<bool> interrupt_pending;
    
    // PC management
    sc_in<sc_uint<32>> pc_in;
    sc_out<sc_uint<32>> pc_out;
    sc_in<bool> pc_save;
    sc_in<bool> pc_restore;
    
    // Cycle counter
    sc_out<sc_uint<64>> cycle_count;
    
    // Constructor
    SC_CTOR(CSRRegisters) {
        // Register processes
        SC_METHOD(csr_access);
        sensitive << csr_read_enable << csr_write_enable << csr_address << csr_write_data;
        
        SC_METHOD(cycle_counter);
        sensitive << clk.pos();
        
        SC_METHOD(interrupt_control);
        sensitive << interrupt_request << clk.pos();
        
        SC_METHOD(pc_management);
        sensitive << pc_save << pc_restore << pc_in << clk.pos();
        
        // Initialize registers
        mstatus = 0;
        mtvec = 0;
        mepc = 0;
        mcause = 0;
        cycles = 0;
        interrupt_enable.write(false);
        interrupt_pending.write(false);
        pc_out.write(0);
        cycle_count.write(0);
    }
    
private:
    // CSR registers
    sc_uint<32> mstatus;  // Machine status register
    sc_uint<32> mtvec;    // Machine trap vector base address
    sc_uint<32> mepc;     // Machine exception program counter
    sc_uint<32> mcause;   // Machine cause register
    sc_uint<64> cycles;   // Cycle counter (64-bit)
    
    // Process methods
    void csr_access();
    void cycle_counter();
    void interrupt_control();
    void pc_management();
    
    // Helper methods
    sc_uint<32> read_csr(sc_uint<12> address);
    void write_csr(sc_uint<12> address, sc_uint<32> data);
    bool is_csr_readable(sc_uint<12> address);
    bool is_csr_writable(sc_uint<12> address);
};

#endif // CSR_REGISTERS_H
