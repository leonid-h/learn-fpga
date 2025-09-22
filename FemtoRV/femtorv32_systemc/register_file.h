/**
 * @file register_file.h
 * @brief RISC-V Register File SystemC Module
 * 
 * Implements the 32-register RISC-V register file with proper
 * SystemC modeling and timing.
 */

#ifndef REGISTER_FILE_H
#define REGISTER_FILE_H

#include "femtorv32_systemc.h"

/**
 * @brief RISC-V Register File Module
 * 
 * Implements the 32-register RISC-V register file with:
 * - 32 32-bit registers (x0-x31)
 * - x0 is hardwired to zero
 * - Dual-port read, single-port write
 * - Proper SystemC timing and sensitivity
 */
class RegisterFile : public sc_module {
public:
    // Ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    
    // Read ports
    sc_in<sc_uint<5>> rs1_addr;
    sc_in<sc_uint<5>> rs2_addr;
    sc_out<sc_uint<32>> rs1_data;
    sc_out<sc_uint<32>> rs2_data;
    
    // Write port
    sc_in<bool> write_enable;
    sc_in<sc_uint<5>> write_addr;
    sc_in<sc_uint<32>> write_data;
    
    // Constructor
    SC_CTOR(RegisterFile) {
        // Register processes
        SC_METHOD(read_ports);
        sensitive << rs1_addr << rs2_addr;
        
        SC_METHOD(write_port);
        sensitive << clk.pos();
        
        // Initialize registers
        for (int i = 0; i < ProcessorConfig::REG_COUNT; i++) {
            registers[i] = 0;
        }
    }
    
private:
    // Internal storage
    sc_uint<32> registers[ProcessorConfig::REG_COUNT];
    
    // Process methods
    void read_ports();
    void write_port();
};

#endif // REGISTER_FILE_H
