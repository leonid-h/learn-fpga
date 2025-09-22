/**
 * @file csr_registers.cpp
 * @brief RISC-V CSR Registers SystemC Module Implementation
 */

#include "csr_registers.h"

void CSRRegisters::csr_access() {
    if (csr_read_enable.read()) {
        csr_read_data.write(read_csr(csr_address.read()));
    } else {
        csr_read_data.write(0);
    }
    
    if (csr_write_enable.read() && is_csr_writable(csr_address.read())) {
        write_csr(csr_address.read(), csr_write_data.read());
    }
}

void CSRRegisters::cycle_counter() {
    if (reset.read()) {
        cycles = 0;
    } else {
        cycles++;
    }
    cycle_count.write(cycles);
}

void CSRRegisters::interrupt_control() {
    if (reset.read()) {
        interrupt_enable.write(false);
        interrupt_pending.write(false);
    } else {
        // Update interrupt enable from mstatus
        interrupt_enable.write(mstatus[3]); // MIE bit
        
        // Update interrupt pending
        interrupt_pending.write(interrupt_request.read() && interrupt_enable.read() && !mcause[0]);
    }
}

void CSRRegisters::pc_management() {
    if (reset.read()) {
        pc_out.write(0);
    } else {
        if (pc_save.read()) {
            mepc = pc_in.read();
        }
        if (pc_restore.read()) {
            pc_out.write(mepc);
        }
    }
}

sc_uint<32> CSRRegisters::read_csr(sc_uint<12> address) {
    switch (address) {
        case CSR::MSTATUS:
            return (mstatus << 3) | 0x0; // MIE bit in bit 3
        case CSR::MTVEC:
            return mtvec;
        case CSR::MEPC:
            return mepc;
        case CSR::MCAUSE:
            return (mcause << 31) | 0x0; // Interrupt bit in MSB
        case CSR::CYCLES:
            return cycles.range(31, 0);
        case CSR::CYCLESH:
            return cycles.range(63, 32);
        default:
            return 0;
    }
}

void CSRRegisters::write_csr(sc_uint<12> address, sc_uint<32> data) {
    switch (address) {
        case CSR::MSTATUS:
            mstatus = data[3]; // MIE bit
            break;
        case CSR::MTVEC:
            mtvec = data;
            break;
        case CSR::MEPC:
            mepc = data;
            break;
        case CSR::MCAUSE:
            mcause = data[31]; // Interrupt bit
            break;
        // CYCLES and CYCLESH are read-only
        default:
            break;
    }
}

bool CSRRegisters::is_csr_readable(sc_uint<12> address) {
    switch (address) {
        case CSR::MSTATUS:
        case CSR::MTVEC:
        case CSR::MEPC:
        case CSR::MCAUSE:
        case CSR::CYCLES:
        case CSR::CYCLESH:
            return true;
        default:
            return false;
    }
}

bool CSRRegisters::is_csr_writable(sc_uint<12> address) {
    switch (address) {
        case CSR::MSTATUS:
        case CSR::MTVEC:
        case CSR::MEPC:
        case CSR::MCAUSE:
            return true;
        case CSR::CYCLES:
        case CSR::CYCLESH:
            return false; // Read-only
        default:
            return false;
    }
}
