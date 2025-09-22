/**
 * @file memory_interface.cpp
 * @brief Memory Interface SystemC Module Implementation
 */

#include "memory_interface.h"

void MemoryInterface::memory_access() {
    if (reset.read()) {
        read_data.write(0);
        read_busy.write(false);
        write_busy.write(false);
        return;
    }
    
    sc_uint<32> addr = address.read();
    
    if (read_strobe.read()) {
        if (is_io_address(addr)) {
            read_data.write(read_io_device(addr));
            read_busy.write(false);
        } else {
            read_data.write(read_memory_word(addr));
            read_busy.write(false);
        }
    } else if (write_strobe.read()) {
        if (is_io_address(addr)) {
            write_io_device(addr, write_data.read());
            write_busy.write(false);
        } else {
            write_memory_word(addr, write_data.read(), write_mask.read());
            write_busy.write(false);
        }
    } else {
        read_busy.write(false);
        write_busy.write(false);
    }
}

void MemoryInterface::timing_control() {
    if (reset.read()) {
        read_cycles = 0;
        write_cycles = 0;
        read_in_progress = false;
        write_in_progress = false;
        return;
    }
    
    // Simple timing model - in a real implementation, this would
    // model actual memory access delays
    if (read_strobe.read() && !read_in_progress) {
        read_in_progress = true;
        read_cycles = 1; // 1 cycle delay
    }
    
    if (write_strobe.read() && !write_in_progress) {
        write_in_progress = true;
        write_cycles = 1; // 1 cycle delay
    }
    
    if (read_in_progress) {
        read_cycles--;
        if (read_cycles == 0) {
            read_in_progress = false;
        }
    }
    
    if (write_in_progress) {
        write_cycles--;
        if (write_cycles == 0) {
            write_in_progress = false;
        }
    }
}

bool MemoryInterface::is_io_address(sc_uint<32> addr) {
    // Check if address is in I/O space (typically 0x40000000 and above)
    return (addr >= 0x40000000);
}

sc_uint<32> MemoryInterface::read_memory_word(sc_uint<32> addr) {
    if (addr >= memory_size) {
        return 0; // Out of bounds
    }
    
    sc_uint<32> word = 0;
    for (int i = 0; i < 4; i++) {
        word.range(7 + i*8, i*8) = memory[addr + i];
    }
    return word;
}

void MemoryInterface::write_memory_word(sc_uint<32> addr, sc_uint<32> data, sc_uint<4> mask) {
    if (addr >= memory_size) {
        return; // Out of bounds
    }
    
    for (int i = 0; i < 4; i++) {
        if (mask[i]) {
            memory[addr + i] = data.range(7 + i*8, i*8);
        }
    }
}

sc_uint<32> MemoryInterface::read_memory_byte(sc_uint<32> addr) {
    if (addr >= memory_size) {
        return 0;
    }
    return memory[addr];
}

sc_uint<32> MemoryInterface::read_memory_halfword(sc_uint<32> addr) {
    if (addr >= memory_size - 1) {
        return 0;
    }
    return (memory[addr + 1] << 8) | memory[addr];
}

void MemoryInterface::write_memory_byte(sc_uint<32> addr, sc_uint<8> data) {
    if (addr < memory_size) {
        memory[addr] = data;
    }
}

void MemoryInterface::write_memory_halfword(sc_uint<32> addr, sc_uint<16> data) {
    if (addr < memory_size - 1) {
        memory[addr] = data.range(7, 0);
        memory[addr + 1] = data.range(15, 8);
    }
}

sc_uint<32> MemoryInterface::read_io_device(sc_uint<32> addr) {
    // Simple I/O device mapping
    switch (addr) {
        case 0x40000000: // LED register
            return led_output.read();
        case 0x40000004: // Button register
            return button_input.read();
        case 0x40000008: // UART status
            return 0x1; // Always ready
        case 0x4000000C: // UART data
            return 0x0; // No data available
        default:
            return 0;
    }
}

void MemoryInterface::write_io_device(sc_uint<32> addr, sc_uint<32> data) {
    // Simple I/O device mapping
    switch (addr) {
        case 0x40000000: // LED register
            led_output.write(data.range(3, 0));
            break;
        case 0x40000008: // UART control
            // UART control register - not implemented
            break;
        case 0x4000000C: // UART data
            // UART data register - not implemented
            break;
        default:
            break;
    }
}

void MemoryInterface::load_program(const std::vector<sc_uint<32>>& program, sc_uint<32> start_address) {
    for (size_t i = 0; i < program.size() && (start_address + i*4) < memory_size; i++) {
        write_memory_word(start_address + i*4, program[i], 0xF);
    }
}
