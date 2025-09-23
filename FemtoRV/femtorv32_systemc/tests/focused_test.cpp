#include <systemc.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include "../femtorv32_quark.h"

struct InstructionValidation {
    std::string instruction_name;
    uint32_t register_id;
    uint32_t expected_value;
    std::string description;
};

struct SimpleTestProgram {
    std::string name;
    std::string description;
    std::vector<uint32_t> instructions;
    uint32_t expected_result;
    uint32_t expected_register;
    uint32_t max_cycles;
    bool validate_during_execution;
    std::vector<InstructionValidation> validations;
};

struct SimpleTestResult {
    std::string name;
    bool passed;
    std::string message;
    int commands_passed = 0;
    int commands_total = 0;
};

class SimpleTestHarness : public sc_module {
public:
    sc_clock clk;
    sc_signal<bool> reset;
    sc_signal<bool> mem_rstrb;
    sc_signal<sc_uint<32>> mem_addr;
    sc_signal<sc_uint<32>> mem_rdata;
    sc_signal<bool> mem_rbusy;
    sc_signal<bool> mem_wbusy;
    sc_signal<sc_uint<4>> mem_wmask;
    sc_signal<sc_uint<32>> mem_wdata;

    FemtoRV32_Quark* cpu;
    std::vector<uint32_t>* memory;
    uint32_t reset_cnt;

    SimpleTestHarness(sc_module_name name) : sc_module(name), clk("clk", 10, SC_NS) {
        cpu = new FemtoRV32_Quark("cpu");
        memory = new std::vector<uint32_t>(1024, 0);
        reset_cnt = 0;

        // Connect CPU to test harness
        cpu->clk(clk);
        cpu->reset(reset);
        cpu->mem_rstrb(mem_rstrb);
        cpu->mem_addr(mem_addr);
        cpu->mem_rdata(mem_rdata);
        cpu->mem_rbusy(mem_rbusy);
        cpu->mem_wbusy(mem_wbusy);
        cpu->mem_wmask(mem_wmask);
        cpu->mem_wdata(mem_wdata);

        SC_METHOD(memory_process);
        sensitive << mem_rstrb << mem_addr << mem_wmask << mem_wdata;

        SC_METHOD(reset_counter_process);
        sensitive << clk.posedge_event();
    }

    void memory_process() {
        if (mem_rstrb.read()) {
            uint32_t addr = mem_addr.read().to_uint();
            uint32_t word_addr = addr >> 2;
            if (word_addr < memory->size()) {
                uint32_t data = (*memory)[word_addr];
                mem_rdata.write(data);
                mem_rbusy.write(false);
            mem_wbusy.write(false);
                #ifdef DEBUG
                std::cout << "  📖 Memory read: addr=0x" << std::hex << addr << std::dec 
                          << " (word=" << word_addr << "), data=0x" << std::hex << data << std::dec << std::endl;
                #endif
            } else {
                mem_rdata.write(0);
                mem_rbusy.write(false);
            mem_wbusy.write(false);
            }
        } else {
            mem_rbusy.write(false);
            mem_wbusy.write(false);
        }

        if (mem_wmask.read().to_uint() != 0) {
            uint32_t addr = mem_addr.read().to_uint();
            uint32_t word_addr = addr >> 2;
            if (word_addr < memory->size()) {
                uint32_t data = mem_wdata.read().to_uint();
                uint32_t mask = mem_wmask.read().to_uint();
                
                uint32_t old_data = (*memory)[word_addr];
                uint32_t new_data = old_data;
                
                for (int i = 0; i < 4; i++) {
                    if (mask & (1 << i)) {
                        new_data = (new_data & ~(0xFF << (i * 8))) | ((data & (0xFF << (i * 8))));
                    }
                }
                
                (*memory)[word_addr] = new_data;
            }
        }
    }

    void reset_counter_process() {
        if (reset_cnt < 1000) {
            reset_cnt++;
            reset.write(reset_cnt < 900);  // Keep reset high for 900 cycles to allow more instructions
        }
        
        // Debug output to track reset signal
        if (reset_cnt % 1000 == 0 || reset_cnt < 910) {
            std::cout << "  🔄 RESET: cycle=" << reset_cnt << ", reset=" << reset.read() << std::endl;
        }
    }

    void load_program(const std::vector<uint32_t>& instructions) {
        for (size_t i = 0; i < instructions.size(); i++) {
            (*memory)[i] = instructions[i];
        }
    }

    uint32_t get_register_value(uint32_t reg_num) const {
        if (reg_num == 0) return 0;
        if (reg_num < 32) {
            return cpu->registerFile[reg_num].to_uint();
        }
        return 0;
    }

    void print_cpu_state() const {
        std::cout << "=== CPU State ===" << std::endl;
        std::cout << "PC: 0x" << std::hex << std::setw(8) << std::setfill('0') 
                  << cpu->PC.to_uint() << std::dec << std::endl;
        for (int i = 1; i <= 5; i++) {
            std::cout << "x" << i << ": 0x" << std::hex << std::setw(8) << std::setfill('0') 
                      << get_register_value(i) << std::dec << std::endl;
        }
        std::cout << std::endl;
    }

    bool validate_instruction_during_execution(const InstructionValidation& validation) const {
        uint32_t actual_value = get_register_value(validation.register_id);
        bool passed = (actual_value == validation.expected_value);
        
        std::cout << "  " << (passed ? "✅" : "❌") << " [" << validation.instruction_name 
                  << "] " << validation.description << std::endl;
        if (!passed) {
            std::cout << "    Expected: 0x" << std::hex << validation.expected_value 
                      << ", Got: 0x" << actual_value << std::dec << std::endl;
        }
        
        return passed;
    }

    SimpleTestResult run_simulation_with_validation(const SimpleTestProgram& test) {
        SimpleTestResult result;
        result.name = test.name;
        result.passed = true;
        
        // Track state transitions to detect instruction completion
        int last_state = -1;
        uint32_t last_pc = 0xFFFFFFFF;
        size_t instruction_index = 0;
        int passed_validations = 0;
        
        for (uint32_t cycle = 0; cycle < test.max_cycles; cycle += 10) {
            sc_start(1, SC_NS);
            
            uint32_t current_pc = cpu->PC.to_uint();
            int current_state = cpu->state;
            
            if (cycle % 100 == 0) {
                std::cout << "  Cycle " << cycle << ": PC=0x" << std::hex << std::setw(8) << std::setfill('0') 
                          << current_pc << std::dec << ", x1=0x" << std::hex << std::setw(8) << std::setfill('0') 
                          << get_register_value(1) << std::dec << ", x2=0x" << std::hex << std::setw(8) << std::setfill('0') 
                          << get_register_value(2) << std::dec << ", x3=0x" << std::hex << std::setw(8) << std::setfill('0') 
                          << get_register_value(3) << std::dec << std::endl;
            }
            
            // Detect instruction completion: state transition from EXECUTE or WAIT_ALU_OR_MEM to FETCH_INSTR
            // FETCH_INSTR = 0, WAIT_INSTR = 1, EXECUTE = 2, WAIT_ALU_OR_MEM = 3
            if (last_state != -1 && instruction_index < test.validations.size()) {
                bool instruction_completed = false;
                
                // Single-cycle instruction completion: EXECUTE -> FETCH_INSTR
                if (last_state == 2 && current_state == 0) {
                    instruction_completed = true;
                }
                // Multi-cycle instruction completion: WAIT_ALU_OR_MEM -> FETCH_INSTR
                else if (last_state == 3 && current_state == 0) {
                    instruction_completed = true;
                }
                
                if (instruction_completed) {
                    const auto& validation = test.validations[instruction_index];
                    std::cout << "🔍 Validating " << validation.instruction_name << " at PC=0x" << std::hex << last_pc << std::dec << std::endl;
                    bool validation_passed = validate_instruction_during_execution(validation);
                    if (validation_passed) passed_validations++;
                    instruction_index++;
                }
            }
            
            last_state = current_state;
            last_pc = current_pc;
        }
        
        std::cout << "📊 Real-time Validation Summary: " << passed_validations << "/" << test.validations.size() << " instructions passed" << std::endl;
        
        result.passed = (static_cast<size_t>(passed_validations) == test.validations.size());
        result.message = "Real-time validation: " + std::to_string(passed_validations) + "/" + std::to_string(test.validations.size()) + " passed";
        result.commands_passed = passed_validations;
        result.commands_total = test.validations.size();
        
        return result;
    }

    ~SimpleTestHarness() {
        delete cpu;
        delete memory;
    }
};

SimpleTestProgram create_focused_validation_test() {
    SimpleTestProgram test;
    test.name = "Focused Validation Test";
    test.description = "Comprehensive test with various RISC-V instruction types for real-time validation";
    test.max_cycles = 15000;  // Maximum cycles to allow all instructions to complete
    test.validate_during_execution = true;
    
    // Comprehensive test program with various instruction types
    test.instructions = {
        // === SETUP ===
        0x00500093,  // addi x1, x0, 5        -> x1 = 5
        0x00300113,  // addi x2, x0, 3        -> x2 = 3
        0x00A00193,  // addi x3, x0, 10       -> x3 = 10
        0x00F00213,  // addi x4, x0, 15       -> x4 = 15
        
        // === IMMEDIATE OPERATIONS ===
        0x00708093,  // addi x1, x1, 7        -> x1 = 12 (5+7)
        0xFFF10113,  // addi x2, x2, -1       -> x2 = 2 (3-1)
        0x00118193,  // addi x3, x3, 1        -> x3 = 11 (10+1)
        
        // === LOGICAL IMMEDIATE OPERATIONS ===
        0x00C0F213,  // andi x4, x1, 12       -> x4 = 12 (12 & 12)
        0x00C0E213,  // ori x4, x1, 12        -> x4 = 12 (12 | 12)
        0x00C0C213,  // xori x4, x1, 12       -> x4 = 0 (12 ^ 12)
        
        // === SHIFT OPERATIONS ===
        0x00109193,  // slli x3, x1, 1        -> x3 = 24 (12<<1)
        0x0010D193,  // srli x3, x1, 1        -> x3 = 6 (12>>1)
        0x002091B3,  // sll x3, x1, x2        -> x3 = 48 (12<<2)
        0x0020D1B3,  // srl x3, x1, x2        -> x3 = 3 (12>>2)
        0x4020D1B3,  // sra x3, x1, x2        -> x3 = 3 (12>>2, arithmetic)
        
        // === REGISTER-REGISTER OPERATIONS ===
        0x002081B3,  // add x3, x1, x2        -> x3 = 14 (12+2)
        0x402081B3,  // sub x3, x1, x2        -> x3 = 10 (12-2)
        0x0020A1B3,  // slt x3, x1, x2        -> x3 = 0 (12 < 2 = false)
        0x0020B1B3,  // sltu x3, x1, x2       -> x3 = 0 (12 < 2 = false)
        0x0020C1B3,  // xor x3, x1, x2        -> x3 = 14 (12 ^ 2)
        0x0020E1B3,  // or x3, x1, x2         -> x3 = 14 (12 | 2)
        0x0020F1B3,  // and x3, x1, x2        -> x3 = 0 (12 & 2)
        
        // === COMPARISON OPERATIONS ===
        0x0020A193,  // slti x3, x1, 2        -> x3 = 0 (12 < 2 = false)
        0x00D0A193,  // slti x3, x1, 13       -> x3 = 1 (12 < 13 = true)
        0x0020B193,  // sltiu x3, x1, 2       -> x3 = 0 (12 < 2 = false)
        0x00D0B193,  // sltiu x3, x1, 13      -> x3 = 1 (12 < 13 = true)
        
        // === MORE ARITHMETIC OPERATIONS ===
        0x002081B3,  // add x3, x1, x2        -> x3 = 14 (12+2)
        0x402081B3,  // sub x3, x1, x2        -> x3 = 10 (12-2)
        0x002081B3,  // add x3, x1, x2        -> x3 = 14 (12+2)
        0x402081B3,  // sub x3, x1, x2        -> x3 = 10 (12-2)
        
        // === MORE LOGICAL OPERATIONS ===
        0x0020C1B3,  // xor x3, x1, x2        -> x3 = 14 (12 ^ 2)
        0x0020E1B3,  // or x3, x1, x2         -> x3 = 14 (12 | 2)
        0x0020F1B3,  // and x3, x1, x2        -> x3 = 0 (12 & 2)
        
        // === MEMORY OPERATIONS ===
        0x0000A023,  // sw x0, 0(x1)          -> store x0 (0) to memory[x1+0] = memory[12]
        0x0000A103,  // lw x2, 0(x1)          -> load from memory[x1+0] = memory[12] to x2
        0x0040A223,  // sw x4, 4(x1)          -> store x4 (0) to memory[x1+4] = memory[16]
        0x0040A183,  // lw x3, 4(x1)          -> load from memory[x1+4] = memory[16] to x3
        
        // === AUIPC INSTRUCTIONS ===
        0x00001117,  // auipc x2, 1           -> x2 = PC + (1 << 12) = 0x1000
        0x00002197,  // auipc x3, 2           -> x3 = PC + (2 << 12) = 0x2000
        
        // === JALR INSTRUCTIONS ===
        0x000080E7,  // jalr x1, 0(x1)        -> jump to x1+0 (PC+4), save return address in x1
        
        // === HALT ===
        0x0000006F   // jal x0, 0             -> jump to PC+0 (infinite loop to halt)
    };
    
    // Individual instruction validations for comprehensive test
    test.validations = {
        // === SETUP ===
        {"ADDI", 1, 5, "addi x1, x0, 5 -> x1 = 5"},
        {"ADDI", 2, 3, "addi x2, x0, 3 -> x2 = 3"},
        {"ADDI", 3, 10, "addi x3, x0, 10 -> x3 = 10"},
        {"ADDI", 4, 15, "addi x4, x0, 15 -> x4 = 15"},
        
        // === IMMEDIATE OPERATIONS ===
        {"ADDI", 1, 12, "addi x1, x1, 7 -> x1 = 12 (5+7)"},
        {"ADDI", 2, 2, "addi x2, x2, -1 -> x2 = 2 (3-1)"},
        {"ADDI", 3, 11, "addi x3, x3, 1 -> x3 = 11 (10+1)"},
        
        // === LOGICAL IMMEDIATE OPERATIONS ===
        {"ANDI", 4, 12, "andi x4, x1, 12 -> x4 = 12 (12 & 12)"},
        {"ORI", 4, 12, "ori x4, x1, 12 -> x4 = 12 (12 | 12)"},
        {"XORI", 4, 0, "xori x4, x1, 12 -> x4 = 0 (12 ^ 12)"},
        
        // === SHIFT OPERATIONS ===
        {"SLLI", 3, 24, "slli x3, x1, 1 -> x3 = 24 (12<<1)"},
        {"SRLI", 3, 6, "srli x3, x1, 1 -> x3 = 6 (12>>1)"},
        {"SLL", 3, 48, "sll x3, x1, x2 -> x3 = 48 (12<<2)"},
        {"SRL", 3, 3, "srl x3, x1, x2 -> x3 = 3 (12>>2)"},
        {"SRA", 3, 3, "sra x3, x1, x2 -> x3 = 3 (12>>2, arithmetic)"},
        
        // === REGISTER-REGISTER OPERATIONS ===
        {"ADD", 3, 14, "add x3, x1, x2 -> x3 = 14 (12+2)"},
        {"SUB", 3, 10, "sub x3, x1, x2 -> x3 = 10 (12-2)"},
        {"SLT", 3, 0, "slt x3, x1, x2 -> x3 = 0 (12 < 2 = false)"},
        {"SLTU", 3, 0, "sltu x3, x1, x2 -> x3 = 0 (12 < 2 = false)"},
        {"XOR", 3, 14, "xor x3, x1, x2 -> x3 = 14 (12 ^ 2)"},
        {"OR", 3, 14, "or x3, x1, x2 -> x3 = 14 (12 | 2)"},
        {"AND", 3, 0, "and x3, x1, x2 -> x3 = 0 (12 & 2)"},
        
        // === COMPARISON OPERATIONS ===
        {"SLTI", 3, 0, "slti x3, x1, 2 -> x3 = 0 (12 < 2 = false)"},
        {"SLTI", 3, 1, "slti x3, x1, 13 -> x3 = 1 (12 < 13 = true)"},
        {"SLTIU", 3, 0, "sltiu x3, x1, 2 -> x3 = 0 (12 < 2 = false)"},
        {"SLTIU", 3, 1, "sltiu x3, x1, 13 -> x3 = 1 (12 < 13 = true)"},
        
        // === MORE ARITHMETIC OPERATIONS ===
        {"ADD", 3, 14, "add x3, x1, x2 -> x3 = 14 (12+2)"},
        {"SUB", 3, 10, "sub x3, x1, x2 -> x3 = 10 (12-2)"},
        {"ADD", 3, 14, "add x3, x1, x2 -> x3 = 14 (12+2)"},
        {"SUB", 3, 10, "sub x3, x1, x2 -> x3 = 10 (12-2)"},
        
        // === MORE LOGICAL OPERATIONS ===
        {"XOR", 3, 14, "xor x3, x1, x2 -> x3 = 14 (12 ^ 2)"},
        {"OR", 3, 14, "or x3, x1, x2 -> x3 = 14 (12 | 2)"},
        {"AND", 3, 0, "and x3, x1, x2 -> x3 = 0 (12 & 2)"},
        
        // === MEMORY OPERATIONS ===
        {"SW", 0, 0, "sw x0, 0(x1) -> store x0 (0) to memory[12]"},
        {"LW", 2, 0, "lw x2, 0(x1) -> load from memory[12] to x2"},
        {"SW", 4, 0, "sw x4, 4(x1) -> store x4 (0) to memory[16]"},
        {"LW", 3, 0, "lw x3, 4(x1) -> load from memory[16] to x3"},
        
        // === AUIPC INSTRUCTIONS ===
        {"AUIPC", 2, 0x1094, "auipc x2, 1 -> x2 = PC + (1 << 12) = 0x94 + 0x1000 = 0x1094"},
        {"AUIPC", 3, 0x2098, "auipc x3, 2 -> x3 = PC + (2 << 12) = 0x98 + 0x2000 = 0x2098"},
        
        // === JALR INSTRUCTIONS ===
        {"JALR", 1, 0xa0, "jalr x1, 0(x1) -> jump to x1+0, save return address in x1"},
        
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
    result.passed = false;
    
    std::cout << "🔍 Running: " << test.name << std::endl;
    std::cout << "📝 Description: " << test.description << std::endl;
    
    // Create test harness
    SimpleTestHarness harness("harness");
    
    // Load program
    std::cout << "📝 Loading program into memory:" << std::endl;
    for (size_t i = 0; i < test.instructions.size(); i++) {
        std::cout << "  Address 0x" << std::hex << (i * 4) << ": 0x" << test.instructions[i] << std::dec << std::endl;
    }
    harness.load_program(test.instructions);
    
    // Print initial state
    std::cout << "🚀 Initial CPU State:" << std::endl;
    harness.print_cpu_state();
    
    // Run simulation
    std::cout << "🔄 Starting simulation with real-time validation for " << test.max_cycles << " cycles..." << std::endl;
    
    if (test.validate_during_execution) {
        result = harness.run_simulation_with_validation(test);
        std::cout << "✅ Simulation completed" << std::endl;
    }
    
    // Print final state
    std::cout << "🏁 Final CPU State (after " << test.max_cycles << " cycles):" << std::endl;
    harness.print_cpu_state();
    
    // The real-time validation results are the authoritative results
    // No need for additional final validation since we already validated each instruction
    
    return result;
}

int sc_main(int /* argc */, char* /* argv */[]) {
    std::cout << "FemtoRV32 Quark SystemC Focused Validation Test Suite" << std::endl;
    std::cout << "====================================================" << std::endl;
    
    // Create focused test program - only failing instructions
    std::vector<SimpleTestProgram> tests = {
        create_focused_validation_test()
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
    
    // Count total commands passed/failed across all tests
    int total_commands_passed = 0;
    int total_commands_failed = 0;
    
    for (const auto& result : results) {
        total_commands_passed += result.commands_passed;
        total_commands_failed += (result.commands_total - result.commands_passed);
    }
    
    std::cout << "Commands passed: " << total_commands_passed << std::endl;
    std::cout << "Commands failed: " << total_commands_failed << std::endl;
    
    if (failed > 0) {
        std::cout << std::endl << "❌ Some tests failed. Check the output above for details." << std::endl;
        return 1;
    } else {
        std::cout << std::endl << "✅ All tests passed!" << std::endl;
        return 0;
    }
}
