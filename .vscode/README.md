# Verilog Support for Cursor IDE

Simple configuration for Verilog syntax highlighting and FPGA build tasks.

## What's Included

- **File associations**: `.v` and `.vh` files use C syntax highlighting
- **Code snippets**: Common Verilog constructs
- **Build tasks**: FPGA build tasks for IceStick and ULX3S
- **File exclusions**: Hide build artifacts

## Usage

1. **Install Verilog extension**: `mshr-h.veriloghdl` (recommended)
2. **Build FPGA projects**: `Ctrl+Shift+P` → "Tasks: Run Task" → "FPGA: Build IceStick"
3. **Use snippets**: Type `module` + Tab for module template

## For Better Verilog Support

Install the Verilog extension:
```bash
cursor --install-extension mshr-h.veriloghdl
```
