/**
 * @file testbench.h
 * @brief FemtoRV32 SystemC Testbench
 * 
 * Comprehensive testbench for the FemtoRV32 processor with:
 * - RISC-V instruction tests
 * - Memory access tests
 * - Interrupt tests
 * - Performance measurements
 * - Debug output
 */

#ifndef TESTBENCH_H
#define TESTBENCH_H

#include "femtorv32_core.h"
#include <vector>
#include <string>
#include <fstream>

/**
 * @brief Testbench Module
 * 
 * Provides comprehensive testing of the FemtoRV32 processor
 * including instruction execution, memory operations, and timing.
 */
class Testbench : public sc_module {
public:
    // Ports
    sc_out<bool> clk;
    sc_out<bool> reset;
    sc_out<bool> interrupt_request;
    
    // Memory interface
    sc_signal<sc_uint<32>> mem_address;
    sc_signal<sc_uint<32>> mem_write_data;
    sc_signal<sc_uint<4>> mem_write_mask;
    sc_signal<sc_uint<32>> mem_read_data;
    sc_signal<bool> mem_read_strobe;
    sc_signal<bool> mem_read_busy;
    sc_signal<bool> mem_write_strobe;
    sc_signal<bool> mem_write_busy;
    
    // Debug outputs
    sc_signal<sc_uint<32>> pc_debug;
    sc_signal<sc_uint<32>> instruction_debug;
    sc_signal<ProcessorState> state_debug;
    
    // Constructor
    SC_CTOR(Testbench) : clk("clk"), reset("reset"), interrupt_request("interrupt_request") {
        // Create processor instance
        processor = new FemtoRV32Core("processor");
        
        // Connect processor ports
        processor->clk(clk);
        processor->reset(reset);
        processor->interrupt_request(interrupt_request);
        processor->mem_address(mem_address);
        processor->mem_write_data(mem_write_data);
        processor->mem_write_mask(mem_write_mask);
        processor->mem_read_data(mem_read_data);
        processor->mem_read_strobe(mem_read_strobe);
        processor->mem_read_busy(mem_read_busy);
        processor->mem_write_strobe(mem_write_strobe);
        processor->mem_write_busy(mem_write_busy);
        processor->pc_debug(pc_debug);
        processor->instruction_debug(instruction_debug);
        processor->state_debug(state_debug);
        
        // Register processes
        SC_METHOD(clock_generator);
        sensitive << clk;
        
        SC_METHOD(test_control);
        sensitive << clk.pos();
        
        SC_METHOD(memory_simulator);
        sensitive << mem_read_strobe << mem_write_strobe << mem_address;
        
        // Initialize
        clk.write(false);
        reset.write(false);
        interrupt_request.write(false);
        mem_read_busy.write(false);
        mem_write_busy.write(false);
        
        test_phase = 0;
        test_passed = 0;
        test_failed = 0;
    }
    
    // Destructor
    ~Testbench() {
        delete processor;
    }
    
    // Test control
    void run_tests();
    void run_instruction_tests();
    void run_memory_tests();
    void run_interrupt_tests();
    void run_performance_tests();
    
    // Test utilities
    void load_test_program(const std::vector<sc_uint<32>>& program);
    void check_register(sc_uint<5> reg, sc_uint<32> expected_value);
    void check_memory(sc_uint<32> addr, sc_uint<32> expected_value);
    void print_test_result(const std::string& test_name, bool passed);
    
    // Memory simulator
    void memory_simulator();
    
private:
    // Processor instance
    FemtoRV32Core* processor;
    
    // Test state
    int test_phase;
    int test_passed;
    int test_failed;
    
    // Memory simulation
    std::vector<sc_uint<32>> memory;
    sc_uint<32> memory_size;
    
    // Process methods
    void clock_generator();
    void test_control();
    
    // Test programs
    std::vector<sc_uint<32>> create_add_test_program();
    std::vector<sc_uint<32>> create_branch_test_program();
    std::vector<sc_uint<32>> create_memory_test_program();
    std::vector<sc_uint<32>> create_interrupt_test_program();
    
    // Utility functions
    sc_uint<32> assemble_instruction(const std::string& mnemonic, 
                                   sc_uint<5> rd = 0, sc_uint<5> rs1 = 0, sc_uint<5> rs2 = 0,
                                   sc_uint<32> imm = 0);
    void print_processor_state();
    void print_memory_dump(sc_uint<32> start_addr, int words);
};

#endif // TESTBENCH_H
