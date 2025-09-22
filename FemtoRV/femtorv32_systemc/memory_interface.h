/**
 * @file memory_interface.h
 * @brief Memory Interface SystemC Module
 * 
 * Implements the memory interface for the RISC-V processor with:
 * - Memory-mapped I/O support
 * - Configurable memory size
 * - Proper timing and handshaking
 * - Load/store operations with byte/halfword/word support
 */

#ifndef MEMORY_INTERFACE_H
#define MEMORY_INTERFACE_H

#include "femtorv32_systemc.h"

/**
 * @brief Memory Interface Module
 * 
 * Handles all memory operations including:
 * - RAM access
 * - Memory-mapped I/O
 * - Load/store with proper alignment
 * - Timing and handshaking
 */
class MemoryInterface : public sc_module {
public:
    // Ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    
    // Memory bus
    sc_in<sc_uint<32>> address;
    sc_in<sc_uint<32>> write_data;
    sc_in<sc_uint<4>> write_mask;
    sc_out<sc_uint<32>> read_data;
    sc_in<bool> read_strobe;
    sc_out<bool> read_busy;
    sc_in<bool> write_strobe;
    sc_out<bool> write_busy;
    
    // I/O devices (simplified for this implementation)
    sc_out<sc_uint<4>> led_output;
    sc_in<sc_uint<4>> button_input;
    
    // Constructor
    SC_CTOR(MemoryInterface) : memory_size(ProcessorConfig::MEMORY_SIZE) {
        // Register processes
        SC_METHOD(memory_access);
        sensitive << address << write_data << write_mask << read_strobe << write_strobe;
        
        SC_METHOD(timing_control);
        sensitive << clk.pos();
        
        // Initialize memory
        memory = new sc_uint<8>[memory_size];
        for (int i = 0; i < memory_size; i++) {
            memory[i] = 0;
        }
        
        // Initialize outputs
        read_data.write(0);
        read_busy.write(false);
        write_busy.write(false);
        led_output.write(0);
    }
    
    // Destructor
    ~MemoryInterface() {
        delete[] memory;
    }
    
    // Memory configuration
    void set_memory_size(int size) {
        memory_size = size;
        delete[] memory;
        memory = new sc_uint<8>[memory_size];
        for (int i = 0; i < memory_size; i++) {
            memory[i] = 0;
        }
    }
    
    // Load program into memory
    void load_program(const std::vector<sc_uint<32>>& program, sc_uint<32> start_address = 0);
    
private:
    // Memory storage
    sc_uint<8>* memory;
    int memory_size;
    
    // Timing control
    int read_cycles;
    int write_cycles;
    bool read_in_progress;
    bool write_in_progress;
    
    // Process methods
    void memory_access();
    void timing_control();
    
    // Helper methods
    bool is_io_address(sc_uint<32> addr);
    sc_uint<32> read_memory_word(sc_uint<32> addr);
    void write_memory_word(sc_uint<32> addr, sc_uint<32> data, sc_uint<4> mask);
    sc_uint<32> read_memory_byte(sc_uint<32> addr);
    sc_uint<32> read_memory_halfword(sc_uint<32> addr);
    void write_memory_byte(sc_uint<32> addr, sc_uint<8> data);
    void write_memory_halfword(sc_uint<32> addr, sc_uint<16> data);
    
    // I/O device handling
    sc_uint<32> read_io_device(sc_uint<32> addr);
    void write_io_device(sc_uint<32> addr, sc_uint<32> data);
};

#endif // MEMORY_INTERFACE_H
