# FemtoRV32 Quark SystemC Implementation

This directory contains a SystemC C++ translation of the FemtoRV32 Quark processor, which is the most elementary version of the FemtoRV32 RISC-V core.

## Overview

The FemtoRV32 Quark is a minimal RISC-V RV32I processor implementation that fits in a single Verilog file. This SystemC translation maintains the same functionality and structure while providing a C++ simulation environment.

## Features

- **Instruction Set**: RV32I + RDCYCLES (cycle counter)
- **Architecture**: Single-cycle with multi-cycle shifts
- **Memory Interface**: 32-bit word-aligned memory access
- **Register File**: 32 registers (x0-x31)
- **State Machine**: 4-state execution model

## Files

- `femtorv32_quark.h` - Main processor header file
- `femtorv32_quark.cpp` - Processor implementation
- `testbench.h` - Testbench header
- `testbench.cpp` - Testbench implementation with simple memory model
- `main.cpp` - Main simulation entry point
- `Makefile` - Build configuration
- `README.md` - This file

## Building

### Prerequisites

- SystemC library (included in `systemc-install/`)
- GCC with C++11 support
- Make

### Build Commands

```bash
# Build the simulation
make

# Build and run
make run

# Clean build artifacts
make clean

# Build with debug symbols
make debug
```

## Running the Simulation

The simulation includes a simple test program that:
1. Loads immediate value 5 into register x1
2. Loads immediate value 3 into register x2
3. Adds x1 + x2 and stores result in x3
4. Enters an infinite loop

```bash
./femtorv32_quark_sim
```

The simulation will:
- Run for 1000 cycles
- Print CPU state every 100 cycles
- Generate a VCD trace file (`femtorv32_quark.vcd`)

## Architecture Details

### State Machine

The processor uses a 4-state execution model:

1. **FETCH_INSTR**: Request instruction from memory
2. **WAIT_INSTR**: Wait for instruction to be available
3. **EXECUTE**: Execute the instruction
4. **WAIT_ALU_OR_MEM**: Wait for ALU shifts or memory operations

### ALU Operations

The ALU supports all RV32I operations:
- Arithmetic: ADD, SUB, ADDI
- Logical: AND, OR, XOR, ANDI, ORI, XORI
- Comparison: SLT, SLTU, SLTI, SLTIU
- Shifts: SLL, SRL, SRA, SLLI, SRLI, SRAI

### Memory Interface

- 32-bit word-aligned memory access
- Support for byte and halfword load/store operations
- Write mask support for partial word writes

### Register File

- 32 registers (x0-x31)
- x0 is hardwired to zero
- Register updates occur on write-back

## Key Differences from Verilog

1. **Process Model**: Uses SystemC SC_METHOD processes instead of always blocks
2. **Signal Types**: Uses SystemC signal types (sc_signal, sc_uint, etc.)
3. **Memory Model**: Includes a simple memory model in the testbench
4. **Debugging**: Enhanced debugging capabilities with state monitoring

## Customization

### Parameters

- `RESET_ADDR`: Reset address (default: 0x00000000)
- `ADDR_WIDTH`: Address bus width (default: 24)

### Macros

- `NRV_TWOLEVEL_SHIFTER`: Enable two-level shifter for faster shifts
- `NRV_COUNTER_WIDTH`: Reduce cycle counter width for space-constrained designs
- `NRV_IS_IO_ADDR`: Define custom I/O address space

## Testing

The included testbench provides:
- Simple memory model
- Basic test program
- State monitoring
- VCD trace generation

For more comprehensive testing, you can:
- Add more test programs to the memory
- Implement a more sophisticated memory model
- Add instruction-level debugging
- Create regression test suites

## Performance

The SystemC implementation is designed for functional verification and simulation. For performance-critical applications, consider:
- Using the original Verilog implementation
- Optimizing the SystemC code for faster simulation
- Using commercial SystemC simulators

## License

This SystemC translation maintains the same license as the original FemtoRV32 project.
