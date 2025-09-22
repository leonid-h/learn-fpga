# FemtoRV32 SystemC Project Structure

## Directory Organization

All SystemC implementation files are organized in the `femtorv32_systemc/` directory:

```
femtorv32_systemc/
├── femtorv32_systemc.h      # Main header with definitions and types
├── register_file.h/.cpp     # RISC-V register file implementation
├── alu.h/.cpp              # ALU with multiplication/division support
├── instruction_decoder.h/.cpp # RV32IMC instruction decoding
├── csr_registers.h/.cpp    # Control and Status Registers
├── memory_interface.h/.cpp # Memory and I/O interface
├── femtorv32_core.h/.cpp   # Main processor core
├── testbench.h/.cpp        # Comprehensive testbench
├── main.cpp                # Main program entry point
├── Makefile                # Build system
├── README.md               # Project documentation
├── Doxyfile                # Documentation generation
└── PROJECT_STRUCTURE.md    # This file
```

## Quick Start

1. **Navigate to the SystemC directory:**
   ```bash
   cd femtorv32_systemc/
   ```

2. **Build the project:**
   ```bash
   make
   ```

3. **Run the simulation:**
   ```bash
   make run
   ```

4. **Run tests:**
   ```bash
   make test
   ```

5. **Generate documentation:**
   ```bash
   make docs
   ```

## Features

- **Complete RISC-V RV32IMC Implementation**
- **SystemC Best Practices**
- **Comprehensive Testing**
- **Production Quality Code**
- **Professional Documentation**

Perfect for Qualcomm interview preparation! 🚀
