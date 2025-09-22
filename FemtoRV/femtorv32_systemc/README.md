# FemtoRV32 SystemC Implementation

A production-quality SystemC translation of the FemtoRV32 RISC-V processor, designed for Qualcomm interview preparation.

## Overview

This project implements a complete RISC-V processor in SystemC, based on the FemtoRV32 Verilog implementation. It features:

- **RV32IMC Instruction Set**: Complete RISC-V 32-bit Integer, Multiply, and Compressed instruction support
- **Interrupt Support**: Full interrupt handling with CSR registers
- **Compressed Instructions**: RISC-V compressed instruction set (16-bit instructions)
- **Memory-Mapped I/O**: Configurable memory interface with I/O devices
- **Production Quality**: Comprehensive error handling, documentation, and testing

## Architecture

The implementation consists of several SystemC modules:

- **FemtoRV32Core**: Main processor core integrating all components
- **RegisterFile**: 32-register RISC-V register file
- **ALU**: Arithmetic Logic Unit with multiplication/division support
- **InstructionDecoder**: Instruction decoding for RV32IMC
- **CompressedInstructionDecoder**: 16-bit compressed instruction decompression
- **CSRRegisters**: Control and Status Registers for interrupt handling
- **MemoryInterface**: Memory and I/O interface
- **Testbench**: Comprehensive test suite

## Building

### Prerequisites

- SystemC 2.3.3 or later
- GCC 4.8+ with C++11 support
- Make

### Installation

1. Install SystemC dependencies:
```bash
make install-deps
```

2. Build SystemC (if not already installed):
```bash
make build-systemc
```

3. Build the project:
```bash
make
```

### Running

Run the simulation:
```bash
make run
```

Run with Valgrind for memory checking:
```bash
make valgrind
```

Run the test suite:
```bash
make test
```

## Testing

The implementation includes a comprehensive test suite:

- **Instruction Tests**: Tests for all instruction types
- **Memory Tests**: Load/store operation testing
- **Interrupt Tests**: Interrupt handling verification
- **Performance Tests**: Timing and performance measurements

## Interview Preparation

This implementation demonstrates:

- **SystemC Expertise**: Advanced SystemC modeling techniques
- **RISC-V Knowledge**: Deep understanding of RISC-V architecture
- **Processor Design**: Complete processor implementation
- **Testing Skills**: Comprehensive test suite development
- **Code Quality**: Production-ready code standards