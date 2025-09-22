#include <systemc.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include "../femtorv32_quark.h"

struct SimpleTestResult {
    std::string name;
    bool passed;
    std::string error_message;
};

struct InstructionValidation {
    std::string instruction_name;
    int register_id;
    uint32_t expected_value;
    std::string description;
};

struct SimpleTestProgram {
    std::string name;
    std::string description;
    std::vector<uint32_t> instructions;
    std::vector<InstructionValidation> validations;
    uint32_t expected_result;
    int expected_register;
    int max_cycles;
    bool validate_during_execution;
};

class SimpleTestHarness : public sc_module {
public:
    sc_clock* clk;
    sc_signal<bool>* reset;
    sc_signal<sc_uint<32> >* mem_addr;
    sc_signal<sc_uint<32> >* mem_wdata;
    sc_signal<sc_uint<4> >* mem_wmask;
    sc_signal<bool>* mem_rstrb;
    sc_signal<bool>* mem_wbusy;
    sc_signal<bool>* mem_rbusy;
    sc_signal<sc_uint<32> >* mem_rdata;
    
    FemtoRV32_Quark* cpu;
    std::vector<uint32_t>* memory;
    
    // Reset counter logic (like in femtosoc.v)
    uint32_t reset_cnt;
    bool internal_reset;
    
    SC_CTOR(SimpleTestHarness) : reset_cnt(0), internal_reset(false) {
        setup_signals();
        setup_cpu();
        setup_memory();
        setup_processes();
    }
    
    ~SimpleTestHarness() {
        cleanup_signals();
    }
    
    void setup_signals() {
        clk = new sc_clock("clk", 10, SC_NS);
        reset = new sc_signal<bool>("reset");
        mem_addr = new sc_signal<sc_uint<32> >("mem_addr");
        mem_wdata = new sc_signal<sc_uint<32> >("mem_wdata");
        mem_wmask = new sc_signal<sc_uint<4> >("mem_wmask");
        mem_rstrb = new sc_signal<bool>("mem_rstrb");
        mem_wbusy = new sc_signal<bool>("mem_wbusy");
        mem_rbusy = new sc_signal<bool>("mem_rbusy");
        mem_rdata = new sc_signal<sc_uint<32> >("mem_rdata");
    }
    
    void setup_cpu() {
        cpu = new FemtoRV32_Quark("cpu");
        cpu->clk(*clk);
        cpu->reset(*reset);
        cpu->mem_addr(*mem_addr);
        cpu->mem_wdata(*mem_wdata);
        cpu->mem_wmask(*mem_wmask);
        cpu->mem_rstrb(*mem_rstrb);
        cpu->mem_wbusy(*mem_wbusy);
        cpu->mem_rbusy(*mem_rbusy);
        cpu->mem_rdata(*mem_rdata);
    }
    
    void setup_memory() {
        memory = new std::vector<uint32_t>(1024, 0);
    }
    
    void setup_processes() {
        SC_METHOD(memory_process);
        sensitive << *mem_rstrb << *mem_addr << *mem_wmask << *mem_wdata;
        SC_METHOD(reset_counter_process);
        sensitive << *clk;
    }
    
    void memory_process() {
        if (mem_rstrb->read()) {
            uint32_t addr = mem_addr->read();
            uint32_t word_addr = addr / 4;
            std::cout << "  📖 Memory read: addr=0x" << std::hex << std::setfill('0') << std::setw(8) << addr
                      << " (word=" << std::dec << word_addr << "), data=0x"
                      << std::hex << std::setfill('0') << std::setw(8) << (*memory)[word_addr] << std::dec << std::endl;
            
            if (word_addr < memory->size()) { // Convert byte address to word address
                mem_rdata->write((*memory)[word_addr]);
            } else {
                mem_rdata->write(0); // Default for out-of-bounds
            }
            mem_rbusy->write(false);
        }
        
        // Handle write operations
        if (mem_wmask->read() != 0) {
            uint32_t addr = mem_addr->read();
            uint32_t data = mem_wdata->read();
            uint32_t mask = mem_wmask->read();
            
            if (addr / 4 < memory->size()) {
                uint32_t current = (*memory)[addr / 4];
                uint32_t new_data = current;
                
                // Apply write mask
                if (mask & 0x1) new_data = (new_data & 0xFFFFFF00) | (data & 0x000000FF);
                if (mask & 0x2) new_data = (new_data & 0xFFFF00FF) | (data & 0x0000FF00);
                if (mask & 0x4) new_data = (new_data & 0xFF00FFFF) | (data & 0x00FF0000);
                if (mask & 0x8) new_data = (new_data & 0x00FFFFFF) | (data & 0xFF000000);
                
                (*memory)[addr / 4] = new_data;
            }
            mem_wbusy->write(false);
        }
    }
    
    void reset_counter_process() {
        // Reset counter logic (like in femtosoc.v)
        // wire reset = &reset_cnt;  // reset is 1 when ALL bits are 1
        // reset_cnt <= reset_cnt + !reset;
        
        if (!internal_reset) {
            reset_cnt = reset_cnt + 1;
        }
        
        // Generate internal reset signal
        // reset is 1 (active low reset = in reset) when counter hasn't reached max
        // reset is 0 (out of reset) when counter reaches max (all bits 1)
        internal_reset = (reset_cnt >= 1000); // Reset for 1000 cycles to avoid reset loop during test
        reset->write(!internal_reset); // Active low reset: 1 = in reset, 0 = out of reset
    }
    
    void load_program(const std::vector<uint32_t>& instructions) {
        // Clear memory
        std::fill(memory->begin(), memory->end(), 0);
        
        // Load instructions into memory starting at address 0
        std::cout << "📝 Loading program into memory:" << std::endl;
        for (size_t i = 0; i < instructions.size() && i < memory->size(); i++) {
            (*memory)[i] = instructions[i];
            std::cout << "  Address 0x" << std::hex << (i * 4) << ": 0x" 
                      << std::setfill('0') << std::setw(8) << instructions[i] << std::dec << std::endl;
        }
        
        // Initialize reset counter to start reset sequence
        reset_cnt = 0;
        internal_reset = false;
    }
    
    void run_simulation(int max_cycles) {
        std::cout << "🔄 Starting simulation for " << max_cycles << " cycles..." << std::endl;
        
        // Let the reset counter process handle the reset sequence
        // This matches the Verilog behavior where reset is controlled by the system
        
        // Run simulation in smaller chunks to allow debugging
        for (int i = 0; i < max_cycles; i += 1) {
            sc_start(1, SC_NS); // Run for just 1 NS (single delta cycle)
            
                   // Print intermediate state every 10 cycles
                   if (i % 10 == 0) {
                       uint32_t pc_val = cpu->PC.to_uint();
                       uint32_t x1_val = cpu->registerFile[1].to_uint();
                       uint32_t x2_val = cpu->registerFile[2].to_uint();
                       uint32_t x3_val = cpu->registerFile[3].to_uint();
                       std::cout << "  Cycle " << i << ": PC=0x" << std::hex << std::setfill('0') << std::setw(8) << pc_val
                                 << ", x1=0x" << std::setfill('0') << std::setw(8) << x1_val
                                 << ", x2=0x" << std::setfill('0') << std::setw(8) << x2_val
                                 << ", x3=0x" << std::setfill('0') << std::setw(8) << x3_val << std::dec << std::endl;
                   }
                   
                   // Debug: Print PC changes and state
                   if (i > 0 && i % 10 == 0) {
                       uint32_t pc_val = cpu->PC.to_uint();
                       std::cout << "  🔍 PC changed to: 0x" << std::hex << std::setfill('0') << std::setw(8) << pc_val << std::dec << std::endl;
                       std::cout << "  🔍 State: " << cpu->state << std::endl;
                       std::cout << "  🔍 Instruction: 0x" << std::hex << std::setfill('0') << std::setw(8) << cpu->instr.to_uint() << std::dec << std::endl;
                       std::cout << "  🔍 isALU: " << cpu->isALU << ", isLoad: " << cpu->isLoad << ", isStore: " << cpu->isStore << std::endl;
                       std::cout << "  🔍 needToWait: " << cpu->needToWait << std::endl;
                       std::cout << "  🔍 jumpToPCplusImm: " << cpu->jumpToPCplusImm << std::endl;
                   }
        }
        
        std::cout << "✅ Simulation completed" << std::endl;
    }
    
    void run_simulation_with_validation(int max_cycles, const std::vector<InstructionValidation>& validations) {
        std::cout << "🔄 Starting simulation with real-time validation for " << max_cycles << " cycles..." << std::endl;
        
        int last_pc = -1;
        size_t instruction_index = 0;
        int passed_validations = 0;
        bool waiting_for_alu_completion = false;
        int alu_wait_cycles = 0;
        
        for (int i = 0; i < max_cycles; i++) {
            sc_start(1, SC_NS);
            
            // Check if PC changed (new instruction executed)
            int current_pc = cpu->PC.to_uint();
            
            // Check if we're waiting for ALU completion
            if (waiting_for_alu_completion) {
                alu_wait_cycles++;
                bool alu_busy = (cpu->aluShamt.to_uint() != 0);
                
                // Check if this is a multi-cycle operation (register-based shifts)
                bool is_multi_cycle = false;
                if (instruction_index < validations.size()) {
                    std::string instr_name = validations[instruction_index].instruction_name;
                    is_multi_cycle = (instr_name == "SLL" || instr_name == "SRL" || instr_name == "SRA");
                }
                
                bool should_validate = false;
                if (is_multi_cycle) {
                    // Multi-cycle operation: wait until ALU is not busy and one more cycle for writeback
                    should_validate = (!alu_busy && alu_wait_cycles > 4);
                } else {
                    // Single-cycle operation: wait four cycles for writeback
                    should_validate = (alu_wait_cycles >= 4);
                }
                
                if (should_validate || alu_wait_cycles > 15) {
                    // Operation completed, now validate
                    if (instruction_index < validations.size()) {
                        std::cout << "🔍 Validating instruction at PC=0x" << std::hex << current_pc << std::dec << " (after completion, aluBusy=" << alu_busy << "):" << std::endl;
                        if (validate_instruction_during_execution(validations[instruction_index], instruction_index)) {
                            passed_validations++;
                        }
                        instruction_index++;
                    }
                    waiting_for_alu_completion = false;
                    alu_wait_cycles = 0;
                }
            }
            
            if (current_pc != last_pc && current_pc > 0) {
                // New instruction executed
                if (!waiting_for_alu_completion && instruction_index < validations.size()) {
                    // Check if this is a multi-cycle operation (register-based shift operations)
                    std::string instr_name = validations[instruction_index].instruction_name;
                    if (instr_name == "SLL" || instr_name == "SRL" || instr_name == "SRA") {
                        // This is a register-based shift operation, wait for ALU completion
                        waiting_for_alu_completion = true;
                        alu_wait_cycles = 0;
                        std::cout << "⏳ Waiting for " << instr_name << " to complete..." << std::endl;
                    } else {
                        // Single-cycle operation (including immediate shifts), wait one cycle for writeback
                        waiting_for_alu_completion = true;
                        alu_wait_cycles = 0;
                        std::cout << "⏳ Waiting for " << instr_name << " writeback to complete..." << std::endl;
                    }
                }
                last_pc = current_pc;
            }
            
            // Print state every 10 cycles
            if (i % 10 == 0) {
                uint32_t pc_val = cpu->PC.to_uint();
                uint32_t x1_val = cpu->registerFile[1].to_uint();
                uint32_t x2_val = cpu->registerFile[2].to_uint();
                uint32_t x3_val = cpu->registerFile[3].to_uint();
                std::cout << "  Cycle " << i << ": PC=0x" << std::hex << std::setfill('0') << std::setw(8) << pc_val
                          << ", x1=0x" << std::setfill('0') << std::setw(8) << x1_val
                          << ", x2=0x" << std::setfill('0') << std::setw(8) << x2_val
                          << ", x3=0x" << std::setfill('0') << std::setw(8) << x3_val << std::dec << std::endl;
            }
        }
        
        std::cout << "📊 Real-time Validation Summary: " << passed_validations << "/" << validations.size() << " instructions passed" << std::endl;
        std::cout << "✅ Simulation completed" << std::endl;
    }
    
    uint32_t get_register_value(int reg_num) const {
        return cpu->registerFile[reg_num].to_uint();
    }
    
    bool validate_instruction_during_execution(const InstructionValidation& validation, int instruction_index) {
        uint32_t actual_value = get_register_value(validation.register_id);
        bool passed = (actual_value == validation.expected_value);
        
        std::cout << "  " << (passed ? "✅" : "❌") << " [" << instruction_index << "] " << validation.instruction_name 
                  << ": " << validation.description << std::endl;
        if (!passed) {
            std::cout << "    Expected: 0x" << std::hex << validation.expected_value 
                      << ", Got: 0x" << actual_value << std::dec << std::endl;
        }
        
        return passed;
    }
    
    void print_cpu_state() {
        std::cout << "=== CPU State ===" << std::endl;
        uint32_t pc_val = cpu->PC.to_uint();
        std::cout << "PC: 0x" << std::hex << std::setfill('0') << std::setw(8) << pc_val << std::endl;
        for (int i = 1; i <= 5; i++) {
            std::cout << "x" << i << ": 0x" << std::hex << std::setfill('0') << std::setw(8) << get_register_value(i) << std::endl;
        }
        std::cout << std::dec << std::endl;
    }
    
    void cleanup_signals() {
        delete clk;
        delete reset;
        delete mem_addr;
        delete mem_wdata;
        delete mem_wmask;
        delete mem_rstrb;
        delete mem_wbusy;
        delete mem_rbusy;
        delete mem_rdata;
        delete cpu;
        delete memory;
    }
};


SimpleTestProgram create_comprehensive_test() {
    SimpleTestProgram test;
    test.name = "Comprehensive RV32I Test";
    test.description = "Complete RISC-V RV32I instruction set test with all major instruction types";
    test.expected_result = 0x718;  // Final result: AUIPC + ADDI combination
    test.expected_register = 5;  // Check x5 which contains the final result
    test.max_cycles = 1500;  // Increased for comprehensive test with more instructions
    test.validate_during_execution = true;
    
    // Comprehensive test program covering all major RV32I instruction types
    test.instructions = {
        // === SETUP ===
        0x00500093,  // addi x1, x0, 5        -> x1 = 5
        0x00300113,  // addi x2, x0, 3        -> x2 = 3
        0x00A00193,  // addi x3, x0, 10       -> x3 = 10
        
        // === ARITHMETIC OPERATIONS ===
        0x002081B3,  // add x3, x1, x2        -> x3 = 8 (5+3)
        0x402081B3,  // sub x3, x1, x2        -> x3 = 2 (5-3)
        0x0020F1B3,  // and x3, x1, x2        -> x3 = 1 (5&3)
        0x0020E1B3,  // or x3, x1, x2         -> x3 = 7 (5|3)
        0x0020C1B3,  // xor x3, x1, x2        -> x3 = 6 (5^3)
        
        // === IMMEDIATE ARITHMETIC ===
        0x00A0A193,  // slti x3, x1, 10       -> x3 = 1 (5<10)
        0x00A0B193,  // sltiu x3, x1, 10      -> x3 = 1 (5<10 unsigned)
        0x00A0C193,  // xori x3, x1, 10       -> x3 = 15 (5^10)
        0x00A0E193,  // ori x3, x1, 10        -> x3 = 15 (5|10)
        0x00A0F193,  // andi x3, x1, 10       -> x3 = 0 (5&10)
        
        // === MORE IMMEDIATE OPERATIONS ===
        0x00708093,  // addi x1, x1, 7        -> x1 = 12 (5+7)
        0xFFF10113,  // addi x2, x2, -1       -> x2 = 2 (3-1)
        0x00118193,  // addi x3, x3, 1        -> x3 = 11 (10+1)
        0x00C0A193,  // slti x3, x1, 12       -> x3 = 0 (12<12)
        0x00D0A193,  // slti x3, x1, 13       -> x3 = 1 (12<13)
        0x00B0B193,  // sltiu x3, x1, 11      -> x3 = 0 (12<11 unsigned)
        0x00D0B193,  // sltiu x3, x1, 13      -> x3 = 1 (12<13 unsigned)
        0x00F0C193,  // xori x3, x1, 15       -> x3 = 3 (12^15)
        0x00F0E193,  // ori x3, x1, 15        -> x3 = 15 (12|15)
        0x00F0F193,  // andi x3, x1, 15       -> x3 = 12 (12&15)
        
        // === SHIFT OPERATIONS ===
        0x00109193,  // slli x3, x1, 1        -> x3 = 10 (5<<1)
        0x0010D193,  // srli x3, x1, 1        -> x3 = 2 (5>>1)
        0x4010D193,  // srai x3, x1, 1        -> x3 = 2 (5>>1, arithmetic)
        0x002091B3,  // sll x3, x1, x2        -> x3 = 40 (5<<3)
        0x0020D1B3,  // srl x3, x1, x2        -> x3 = 0 (5>>3)
        0x4020D1B3,  // sra x3, x1, x2        -> x3 = 0 (5>>3, arithmetic)
        
        // === COMPARISON OPERATIONS ===
        0x0020A1B3,  // slt x3, x1, x2        -> x3 = 0 (5<3)
        0x0020B1B3,  // sltu x3, x1, x2       -> x3 = 0 (5<3 unsigned)
        
        // === MORE COMPARISON OPERATIONS ===
        0x0010A1B3,  // slt x3, x1, x1        -> x3 = 0 (5<5)
        0x0020A1B3,  // slt x3, x1, x2        -> x3 = 0 (5<3)
        0x0010B1B3,  // sltu x3, x1, x1       -> x3 = 0 (5<5 unsigned)
        0x0020B1B3,  // sltu x3, x1, x2       -> x3 = 0 (5<3 unsigned)
        0x00A0A193,  // slti x3, x1, 10       -> x3 = 1 (5<10)
        0x0030A193,  // slti x3, x1, 3        -> x3 = 0 (5<3)
        0x00A0B193,  // sltiu x3, x1, 10      -> x3 = 1 (5<10 unsigned)
        0x0030B193,  // sltiu x3, x1, 3       -> x3 = 0 (5<3 unsigned)
        
        // === UPPER IMMEDIATE OPERATIONS ===
        0x123450B7,  // lui x1, 0x12345       -> x1 = 0x12345000
        0x00008097,  // auipc x1, 0           -> x1 = PC + 0 (current PC)
        
        // === FINAL COMBINATION ===
        0x123452B7,  // lui x5, 0x12345       -> x5 = 0x12345000
        0x67808093,  // addi x1, x1, 0x678    -> x1 = 0x12345678
        0x00008293,  // addi x5, x1, 0        -> x5 = 0x12345678 (final result)
        
        // === HALT ===
        0x0000006F   // jal x0, 0             -> jump to PC+0 (infinite loop to halt)
    };
    
    // Individual instruction validations for comprehensive test
    test.validations = {
        // === SETUP ===
        {"ADDI", 1, 5, "addi x1, x0, 5 -> x1 = 5"},
        {"ADDI", 2, 3, "addi x2, x0, 3 -> x2 = 3"},
        {"ADDI", 3, 10, "addi x3, x0, 10 -> x3 = 10"},
        
        // === ARITHMETIC OPERATIONS ===
        {"ADD", 3, 8, "add x3, x1, x2 -> x3 = 8 (5+3)"},
        {"SUB", 3, 2, "sub x3, x1, x2 -> x3 = 2 (5-3)"},
        {"AND", 3, 1, "and x3, x1, x2 -> x3 = 1 (5&3)"},
        {"OR", 3, 7, "or x3, x1, x2 -> x3 = 7 (5|3)"},
        {"XOR", 3, 6, "xor x3, x1, x2 -> x3 = 6 (5^3)"},
        
        // === IMMEDIATE ARITHMETIC ===
        {"SLTI", 3, 1, "slti x3, x1, 10 -> x3 = 1 (5<10)"},
        {"SLTIU", 3, 1, "sltiu x3, x1, 10 -> x3 = 1 (5<10 unsigned)"},
        {"XORI", 3, 15, "xori x3, x1, 10 -> x3 = 15 (5^10)"},
        {"ORI", 3, 15, "ori x3, x1, 10 -> x3 = 15 (5|10)"},
        {"ANDI", 3, 0, "andi x3, x1, 10 -> x3 = 0 (5&10)"},
        
        // === MORE IMMEDIATE OPERATIONS ===
        {"ADDI", 1, 12, "addi x1, x1, 7 -> x1 = 12 (5+7)"},
        {"ADDI", 2, 2, "addi x2, x2, -1 -> x2 = 2 (3-1)"},
        {"ADDI", 3, 11, "addi x3, x3, 1 -> x3 = 11 (10+1)"},
        {"SLTI", 3, 0, "slti x3, x1, 12 -> x3 = 0 (12<12)"},
        {"SLTI", 3, 1, "slti x3, x1, 13 -> x3 = 1 (12<13)"},
        {"SLTIU", 3, 0, "sltiu x3, x1, 11 -> x3 = 0 (12<11 unsigned)"},
        {"SLTIU", 3, 1, "sltiu x3, x1, 13 -> x3 = 1 (12<13 unsigned)"},
        {"XORI", 3, 3, "xori x3, x1, 15 -> x3 = 3 (12^15)"},
        {"ORI", 3, 15, "ori x3, x1, 15 -> x3 = 15 (12|15)"},
        {"ANDI", 3, 12, "andi x3, x1, 15 -> x3 = 12 (12&15)"},
        
        // === SHIFT OPERATIONS ===
        {"SLLI", 3, 24, "slli x3, x1, 1 -> x3 = 24 (12<<1)"},
        {"SRLI", 3, 6, "srli x3, x1, 1 -> x3 = 6 (12>>1)"},
        {"SRAI", 3, 6, "srai x3, x1, 1 -> x3 = 6 (12>>1, arithmetic)"},
        {"SLL", 3, 48, "sll x3, x1, x2 -> x3 = 48 (12<<2)"},
        {"SRL", 3, 3, "srl x3, x1, x2 -> x3 = 3 (12>>2)"},
        {"SRA", 3, 3, "sra x3, x1, x2 -> x3 = 3 (12>>2, arithmetic)"},
        
        // === COMPARISON OPERATIONS ===
        {"SLT", 3, 0, "slt x3, x1, x2 -> x3 = 0 (12<2)"},
        {"SLTU", 3, 0, "sltu x3, x1, x2 -> x3 = 0 (12<2 unsigned)"},
        
        // === MORE COMPARISON OPERATIONS ===
        {"SLT", 3, 0, "slt x3, x1, x1 -> x3 = 0 (5<5)"},
        {"SLT", 3, 0, "slt x3, x1, x2 -> x3 = 0 (5<3)"},
        {"SLTU", 3, 0, "sltu x3, x1, x1 -> x3 = 0 (5<5 unsigned)"},
        {"SLTU", 3, 0, "sltu x3, x1, x2 -> x3 = 0 (5<3 unsigned)"},
        {"SLTI", 3, 0, "slti x3, x1, 10 -> x3 = 0 (12<10)"},
        {"SLTI", 3, 0, "slti x3, x1, 3 -> x3 = 0 (12<3)"},
        {"SLTIU", 3, 0, "sltiu x3, x1, 10 -> x3 = 0 (12<10 unsigned)"},
        {"SLTIU", 3, 0, "sltiu x3, x1, 3 -> x3 = 0 (12<3 unsigned)"},
        
        // === UPPER IMMEDIATE OPERATIONS ===
        {"LUI", 1, 0x12345000, "lui x1, 0x12345 -> x1 = 0x12345000"},
        {"AUIPC", 1, 0xa0, "auipc x1, 0 -> x1 = PC + 0 = 0xa0"},
        
        // === FINAL COMBINATION ===
        {"LUI", 5, 0x12345000, "lui x5, 0x12345 -> x5 = 0x12345000"},
        {"ADDI", 1, 0x718, "addi x1, x1, 0x678 -> x1 = 0xa0 + 0x678 = 0x718"},
        {"ADDI", 5, 0x718, "addi x5, x1, 0 -> x5 = 0x718 + 0 = 0x718 (final result)"},
        
        // === HALT ===
        {"JAL", 0, 0, "jal x0, 0 -> jump to PC+0 (infinite loop)"}
    };
    
    return test;
}


bool validate_instruction(const SimpleTestHarness& harness, const InstructionValidation& validation) {
    uint32_t actual_value = harness.get_register_value(validation.register_id);
    bool passed = (actual_value == validation.expected_value);
    
    std::cout << "  " << (passed ? "✅" : "❌") << " " << validation.instruction_name 
              << ": " << validation.description << std::endl;
    if (!passed) {
        std::cout << "    Expected: 0x" << std::hex << validation.expected_value 
                  << ", Got: 0x" << actual_value << std::dec << std::endl;
    }
    
    return passed;
}

SimpleTestResult run_test(const SimpleTestProgram& test) {
    SimpleTestResult result;
    result.name = test.name;
    
    std::cout << "🔍 Running: " << test.name << std::endl;
    std::cout << "📝 Description: " << test.description << std::endl;
    
    // Create test harness
    SimpleTestHarness* harness = new SimpleTestHarness("harness");
    
    // Load program
    harness->load_program(test.instructions);
    
    std::cout << "🚀 Initial CPU State:" << std::endl;
    harness->print_cpu_state();
    
    // Run simulation with or without real-time validation
    if (test.validate_during_execution) {
        harness->run_simulation_with_validation(test.max_cycles, test.validations);
    } else {
        harness->run_simulation(test.max_cycles);
        
        // Validate individual instructions at the end (old method)
        std::cout << "🔍 Individual Instruction Validation:" << std::endl;
        int passed_validations = 0;
        
        for (const auto& validation : test.validations) {
            if (validate_instruction(*harness, validation)) {
                passed_validations++;
            }
        }
    }
    
    std::cout << "🏁 Final CPU State (after " << test.max_cycles << " cycles):" << std::endl;
    harness->print_cpu_state();
    
    // Check final result
    uint32_t actual_result = harness->get_register_value(test.expected_register);
    
    // Overall test result
    bool final_result_correct = (actual_result == test.expected_result);
    
    std::cout << "📊 Final Validation Summary:" << std::endl;
    std::cout << "  Final Result: " << (final_result_correct ? "✅ PASS" : "❌ FAIL") << std::endl;
    
    if (final_result_correct) {
        std::cout << "✅ PASS: Expected 0x" << std::hex << test.expected_result 
                  << ", got 0x" << actual_result << std::dec << std::endl;
    } else {
        std::cout << "❌ FAIL: Expected 0x" << std::hex << test.expected_result 
                  << ", got 0x" << actual_result << std::dec << std::endl;
    }
    
    result.passed = final_result_correct;
    if (!result.passed) {
        result.error_message = "Final result mismatch";
    }
    
    // Clean up harness
    delete harness;
    
    return result;
}

int sc_main(int /* argc */, char* /* argv */[]) {
    std::cout << "FemtoRV32 Quark SystemC Assembly Instruction Test Suite" << std::endl;
    std::cout << "=====================================================" << std::endl;
    
    // Create test programs - comprehensive test with multiple instruction types
    std::vector<SimpleTestProgram> tests = {
        create_comprehensive_test()
    };
    
    // Run all tests
    std::vector<SimpleTestResult> results;
    int passed = 0;
    int failed = 0;
    
    for (const auto& test : tests) {
        SimpleTestResult result = run_test(test);
        results.push_back(result);
        
        if (result.passed) {
            passed++;
        } else {
            failed++;
        }
        
        std::cout << std::endl;
    }
    
    // Print summary
    std::cout << "=== Test Suite Summary ===" << std::endl;
    std::cout << "Total Tests: " << (passed + failed) << std::endl;
    std::cout << "Passed: " << passed << std::endl;
    std::cout << "Failed: " << failed << std::endl;
    std::cout << "Success Rate: " << std::fixed << std::setprecision(1) 
              << (100.0 * passed / (passed + failed)) << "%" << std::endl;
    
    if (failed > 0) {
        std::cout << std::endl << "❌ Some tests failed. Check the output above for details." << std::endl;
        return 1;
    } else {
        std::cout << std::endl << "✅ All tests passed!" << std::endl;
        return 0;
    }
}
