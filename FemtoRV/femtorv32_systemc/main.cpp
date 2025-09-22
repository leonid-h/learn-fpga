/**
 * @file main.cpp
 * @brief FemtoRV32 SystemC Main Program
 * 
 * Main program that instantiates the testbench and runs the simulation.
 * This demonstrates the complete SystemC implementation of the FemtoRV32
 * RISC-V processor.
 */

#include "testbench.h"
#include <iostream>
#include <cstdlib>

int sc_main(int argc, char* argv[]) {
    std::cout << "FemtoRV32 SystemC Implementation" << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << "A production-quality SystemC translation of the FemtoRV32 RISC-V processor" << std::endl;
    std::cout << "Features: RV32IMC instruction set, interrupts, compressed instructions" << std::endl;
    std::cout << std::endl;
    
    // Create testbench
    Testbench testbench("testbench");
    
    // Set up tracing (optional)
    sc_trace_file* tf = sc_create_vcd_trace_file("femtorv32_trace");
    if (tf) {
        sc_trace(tf, testbench.clk, "clk");
        sc_trace(tf, testbench.reset, "reset");
        sc_trace(tf, testbench.pc_debug, "pc");
        sc_trace(tf, testbench.instruction_debug, "instruction");
        sc_trace(tf, testbench.state_debug, "state");
        sc_trace(tf, testbench.mem_address, "mem_addr");
        sc_trace(tf, testbench.mem_read_data, "mem_rdata");
        sc_trace(tf, testbench.mem_write_data, "mem_wdata");
        sc_trace(tf, testbench.mem_read_strobe, "mem_rstrb");
        sc_trace(tf, testbench.mem_write_strobe, "mem_wstrb");
    }
    
    // Initialize simulation
    std::cout << "Initializing simulation..." << std::endl;
    
    // Reset the system
    testbench.reset.write(true);
    sc_start(10, SC_NS);
    testbench.reset.write(false);
    
    std::cout << "Starting simulation..." << std::endl;
    
    // Run simulation
    try {
        sc_start(10000, SC_NS);
    } catch (const std::exception& e) {
        std::cerr << "Simulation error: " << e.what() << std::endl;
        return 1;
    }
    
    // Close trace file
    if (tf) {
        sc_close_vcd_trace_file(tf);
        std::cout << "Trace file 'femtorv32_trace.vcd' created" << std::endl;
    }
    
    std::cout << "Simulation completed successfully!" << std::endl;
    
    return 0;
}
