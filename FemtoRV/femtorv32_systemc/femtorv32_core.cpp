/**
 * @file femtorv32_core.cpp
 * @brief FemtoRV32 Core SystemC Module Implementation
 */

#include "femtorv32_core.h"

FemtoRV32Core::FemtoRV32Core(sc_module_name name) : sc_module(name) {
    // Initialize component instances
    reg_file = new RegisterFile("register_file");
    alu = new ALU("alu");
    instr_decoder = new InstructionDecoder("instruction_decoder");
    comp_decoder = new CompressedInstructionDecoder("compressed_decoder");
    csr_regs = new CSRRegisters("csr_registers");
    
    // Connect register file
    reg_file->clk(clk);
    reg_file->reset(reset);
    reg_file->rs1_addr(rs1_addr);
    reg_file->rs2_addr(rs2_addr);
    reg_file->rs1_data(rs1_data);
    reg_file->rs2_data(rs2_data);
    reg_file->write_enable(reg_write_enable);
    reg_file->write_addr(rd_addr);
    reg_file->write_data(rd_data);
    
    // Connect ALU
    alu->clk(clk);
    alu->reset(reset);
    alu->operand1(alu_operand1);
    alu->operand2(alu_operand2);
    alu->operation(alu_operation);
    alu->start(alu_start);
    alu->result(alu_result);
    alu->busy(alu_busy);
    alu->valid(alu_valid);
    
    // Connect instruction decoder
    instr_decoder->instruction(decoded_instruction);
    instr_decoder->is_compressed(is_compressed);
    instr_decoder->decode_enable(true);
    instr_decoder->instruction_type(instruction_type);
    instr_decoder->alu_operation(decoded_alu_op);
    
    // Connect CSR registers
    csr_regs->clk(clk);
    csr_regs->reset(reset);
    csr_regs->csr_read_enable(csr_read_enable);
    csr_regs->csr_write_enable(csr_write_enable);
    csr_regs->csr_address(csr_address);
    csr_regs->csr_write_data(csr_write_data);
    csr_regs->csr_read_data(csr_read_data);
    csr_regs->interrupt_request(interrupt_request);
    csr_regs->pc_in(pc);
    csr_regs->pc_save(false);
    csr_regs->pc_restore(false);
    
    // Initialize configuration
    reset_address = ProcessorConfig::RESET_ADDR;
    address_width = ProcessorConfig::ADDR_WIDTH;
}

void FemtoRV32Core::processor_control() {
    if (reset.read()) {
        reset_processor();
        return;
    }
    
    cycles++;
    
    // Update debug outputs
    pc_debug.write(pc);
    instruction_debug.write(instruction);
    state_debug.write(state);
    
    // State machine
    switch (state) {
        case ProcessorState::FETCH_INSTR:
            fetch_instruction();
            break;
            
        case ProcessorState::WAIT_INSTR:
            if (!mem_read_busy.read()) {
                instruction = mem_read_data.read();
                decoded_instruction.write(instruction);
                state = ProcessorState::EXECUTE;
            }
            break;
            
        case ProcessorState::EXECUTE:
            execute_instruction();
            break;
            
        case ProcessorState::WAIT_ALU_OR_MEM:
            if (!alu_busy.read() && !mem_read_busy.read() && !mem_write_busy.read()) {
                state = ProcessorState::FETCH_INSTR;
            }
            break;
            
        case ProcessorState::WAIT_ALU_OR_MEM_SKIP:
            if (!alu_busy.read() && !mem_read_busy.read() && !mem_write_busy.read()) {
                state = ProcessorState::WAIT_INSTR;
            }
            break;
    }
}

void FemtoRV32Core::instruction_fetch() {
    if (state == ProcessorState::FETCH_INSTR) {
        mem_address.write(pc);
        mem_read_strobe.write(true);
        state = ProcessorState::WAIT_INSTR;
    } else {
        mem_read_strobe.write(false);
    }
}

void FemtoRV32Core::instruction_execute() {
    if (state != ProcessorState::EXECUTE) return;
    
    // Extract instruction fields
    sc_uint<32> instr = instruction;
    sc_uint<5> rd = instr.range(11, 7);
    sc_uint<5> rs1 = instr.range(19, 15);
    sc_uint<5> rs2 = instr.range(24, 20);
    sc_uint<3> funct3 = instr.range(14, 12);
    sc_uint<7> funct7 = instr.range(31, 25);
    sc_uint<7> opcode = instr.range(6, 0);
    
    // Set register addresses
    rs1_addr.write(rs1);
    rs2_addr.write(rs2);
    rd_addr.write(rd);
    
    // Decode instruction type
    bool is_load = (opcode == 0x03);
    bool is_store = (opcode == 0x23);
    bool is_alu_reg = (opcode == 0x33);
    bool is_alu_imm = (opcode == 0x13);
    bool is_branch = (opcode == 0x63);
    bool is_jal = (opcode == 0x6F);
    bool is_jalr = (opcode == 0x67);
    bool is_lui = (opcode == 0x37);
    bool is_auipc = (opcode == 0x17);
    bool is_system = (opcode == 0x73);
    
    // Execute instruction
    if (is_load || is_store) {
        handle_memory_access();
    } else if (is_alu_reg || is_alu_imm) {
        // Set ALU operands
        alu_operand1.write(rs1_data.read());
        if (is_alu_reg) {
            alu_operand2.write(rs2_data.read());
        } else {
            // Extract immediate value
            sc_uint<32> imm = 0;
            if (is_alu_imm) {
                imm = sc_int<32>(sc_int<12>(instr.range(31, 20)));
            }
            alu_operand2.write(imm);
        }
        
        // Set ALU operation
        alu_operation.write(decoded_alu_op.read());
        alu_start.write(true);
        
        // Write result to register
        if (rd != 0) {
            reg_write_enable.write(true);
            rd_data.write(alu_result.read());
        }
        
        state = ProcessorState::WAIT_ALU_OR_MEM;
    } else if (is_branch) {
        handle_branch();
    } else if (is_jal || is_jalr) {
        handle_jump();
    } else if (is_lui) {
        // Load upper immediate
        sc_uint<32> imm = instr.range(31, 12) << 12;
        if (rd != 0) {
            reg_write_enable.write(true);
            rd_data.write(imm);
        }
        state = ProcessorState::FETCH_INSTR;
    } else if (is_auipc) {
        // Add upper immediate to PC
        sc_uint<32> imm = instr.range(31, 12) << 12;
        sc_uint<32> result = pc + imm;
        if (rd != 0) {
            reg_write_enable.write(true);
            rd_data.write(result);
        }
        state = ProcessorState::FETCH_INSTR;
    } else if (is_system) {
        handle_system_instruction();
    }
}

void FemtoRV32Core::memory_operations() {
    // Handle memory operations
    if (state == ProcessorState::EXECUTE) {
        sc_uint<32> instr = instruction;
        sc_uint<7> opcode = instr.range(6, 0);
        
        if (opcode == 0x03) { // Load
            sc_uint<32> base_addr = rs1_data.read();
            sc_uint<32> offset = sc_int<32>(sc_int<12>(instr.range(31, 20)));
            sc_uint<32> addr = base_addr + offset;
            
            mem_address.write(addr);
            mem_read_strobe.write(true);
            state = ProcessorState::WAIT_ALU_OR_MEM;
        } else if (opcode == 0x23) { // Store
            sc_uint<32> base_addr = rs1_data.read();
            sc_uint<32> offset = (sc_int<32>(sc_int<12>(instr.range(31, 25) << 5 | instr.range(11, 7))));
            sc_uint<32> addr = base_addr + offset;
            
            mem_address.write(addr);
            mem_write_data.write(rs2_data.read());
            mem_write_mask.write(0xF); // Full word write
            mem_write_strobe.write(true);
            state = ProcessorState::WAIT_ALU_OR_MEM;
        }
    } else {
        mem_read_strobe.write(false);
        mem_write_strobe.write(false);
    }
}

void FemtoRV32Core::reset_processor() {
    pc = reset_address;
    instruction = 0;
    state = ProcessorState::FETCH_INSTR;
    cycles = 0;
    
    // Reset all outputs
    mem_address.write(0);
    mem_write_data.write(0);
    mem_write_mask.write(0);
    mem_read_strobe.write(false);
    mem_write_strobe.write(false);
    reg_write_enable.write(false);
    alu_start.write(false);
    csr_read_enable.write(false);
    csr_write_enable.write(false);
}

void FemtoRV32Core::fetch_instruction() {
    mem_address.write(pc);
    mem_read_strobe.write(true);
    state = ProcessorState::WAIT_INSTR;
}

void FemtoRV32Core::handle_branch() {
    sc_uint<32> instr = instruction;
    sc_uint<3> funct3 = instr.range(14, 12);
    sc_uint<32> rs1_val = rs1_data.read();
    sc_uint<32> rs2_val = rs2_data.read();
    
    bool branch_taken = false;
    
    switch (funct3) {
        case 0x0: // BEQ
            branch_taken = (rs1_val == rs2_val);
            break;
        case 0x1: // BNE
            branch_taken = (rs1_val != rs2_val);
            break;
        case 0x4: // BLT
            branch_taken = (sc_int<32>(rs1_val) < sc_int<32>(rs2_val));
            break;
        case 0x5: // BGE
            branch_taken = (sc_int<32>(rs1_val) >= sc_int<32>(rs2_val));
            break;
        case 0x6: // BLTU
            branch_taken = (rs1_val < rs2_val);
            break;
        case 0x7: // BGEU
            branch_taken = (rs1_val >= rs2_val);
            break;
    }
    
    if (branch_taken) {
        // Extract branch immediate
        sc_uint<13> b_imm = (instr[31] << 12) | (instr[7] << 11) | 
                           (instr.range(30, 25) << 5) | (instr.range(11, 8) << 1);
        sc_uint<32> branch_target = pc + sc_int<32>(sc_int<13>(b_imm));
        pc = branch_target;
    } else {
        pc = pc + 4;
    }
    
    state = ProcessorState::FETCH_INSTR;
}

void FemtoRV32Core::handle_jump() {
    sc_uint<32> instr = instruction;
    sc_uint<5> rd = instr.range(11, 7);
    sc_uint<7> opcode = instr.range(6, 0);
    
    // Save return address
    if (rd != 0) {
        reg_write_enable.write(true);
        rd_data.write(pc + 4);
    }
    
    if (opcode == 0x6F) { // JAL
        // Extract jump immediate
        sc_uint<21> j_imm = (instr[31] << 20) | (instr.range(19, 12) << 12) |
                           (instr[20] << 11) | (instr.range(30, 21) << 1);
        pc = pc + sc_int<32>(sc_int<21>(j_imm));
    } else if (opcode == 0x67) { // JALR
        sc_uint<32> base_addr = rs1_data.read();
        sc_uint<32> offset = sc_int<32>(sc_int<12>(instr.range(31, 20)));
        pc = (base_addr + offset) & ~1; // Clear LSB
    }
    
    state = ProcessorState::FETCH_INSTR;
}

void FemtoRV32Core::handle_system_instruction() {
    sc_uint<32> instr = instruction;
    sc_uint<12> csr_addr = instr.range(31, 20);
    sc_uint<5> rs1 = instr.range(19, 15);
    sc_uint<3> funct3 = instr.range(14, 12);
    
    if (funct3 == 0x0) { // MRET
        // Return from interrupt
        csr_regs->pc_restore.write(true);
        pc = csr_regs->pc_out.read();
        state = ProcessorState::FETCH_INSTR;
    } else {
        // CSR operations
        csr_address.write(csr_addr);
        csr_read_enable.write(true);
        
        if (funct3 != 0x0) {
            csr_write_enable.write(true);
            csr_write_data.write(rs1_data.read());
        }
        
        // Write result to register
        sc_uint<5> rd = instr.range(11, 7);
        if (rd != 0) {
            reg_write_enable.write(true);
            rd_data.write(csr_read_data.read());
        }
        
        state = ProcessorState::FETCH_INSTR;
    }
}

void FemtoRV32Core::handle_interrupt() {
    if (interrupt_request.read() && csr_regs->interrupt_enable.read()) {
        // Save current PC
        csr_regs->pc_save.write(true);
        csr_regs->pc_in.write(pc);
        
        // Jump to interrupt handler
        pc = csr_regs->pc_out.read();
        state = ProcessorState::FETCH_INSTR;
    }
}
