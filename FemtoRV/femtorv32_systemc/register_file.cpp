/**
 * @file register_file.cpp
 * @brief RISC-V Register File SystemC Module Implementation
 */

#include "register_file.h"

void RegisterFile::read_ports() {
    // Read rs1 data
    if (rs1_addr.read() == 0) {
        rs1_data.write(0); // x0 is hardwired to zero
    } else {
        rs1_data.write(registers[rs1_addr.read()]);
    }
    
    // Read rs2 data
    if (rs2_addr.read() == 0) {
        rs2_data.write(0); // x0 is hardwired to zero
    } else {
        rs2_data.write(registers[rs2_addr.read()]);
    }
}

void RegisterFile::write_port() {
    if (reset.read()) {
        // Reset all registers to zero
        for (int i = 0; i < ProcessorConfig::REG_COUNT; i++) {
            registers[i] = 0;
        }
    } else if (write_enable.read()) {
        // Write to register (x0 is read-only)
        sc_uint<5> addr = write_addr.read();
        if (addr != 0) {
            registers[addr] = write_data.read();
        }
    }
}
