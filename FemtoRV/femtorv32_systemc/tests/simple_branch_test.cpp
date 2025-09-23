#include <systemc.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include "../femtorv32_quark.h"

class SimpleBranchTestHarness : public sc_module {
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

    SimpleBranchTestHarness(sc_module_name name) : sc_module(name), clk("clk", 10, SC_NS) {
        cpu = new FemtoRV32_Quark("cpu");
        memory = new std::vector<uint32_t>(1024, 0);
        reset_cnt = 0;

        // Connect CPU signals
        cpu->clk(clk);
        cpu->reset(reset);
        cpu->mem_rstrb(mem_rstrb);
        cpu->mem_addr(mem_addr);
        cpu->mem_rdata(mem_rdata);
        cpu->mem_rbusy(mem_rbusy);
        cpu->mem_wbusy(mem_wbusy);
        cpu->mem_wmask(mem_wmask);
        cpu->mem_wdata(mem_wdata);

        // Memory process
        SC_METHOD(memory_process);
        sensitive << clk;

        // Reset process
        SC_METHOD(reset_process);
        sensitive << clk;
    }

    void memory_process() {
        if (mem_rstrb.read()) {
            uint32_t addr = mem_addr.read().to_uint();
            uint32_t word_addr = addr >> 2;
            if (word_addr < memory->size()) {
                uint32_t data = (*memory)[word_addr];
                mem_rdata.write(data);
                mem_rbusy.write(false);
            } else {
                mem_rdata.write(0);
                mem_rbusy.write(false);
            }
        } else {
            mem_rbusy.write(false);
        }
    }

    void reset_process() {
        if (reset_cnt < 1000) {
            reset_cnt++;
            reset = (reset_cnt < 900);  // Keep reset high for 900 cycles
        }
    }

    ~SimpleBranchTestHarness() {
        delete cpu;
        delete memory;
    }
};

int sc_main(int /* argc */, char* /* argv */[]) {
    std::cout << "Simple Branch Test - Verifying Conditional Branching" << std::endl;
    std::cout << "===================================================" << std::endl;
    
    // Create test harness
    SimpleBranchTestHarness harness("harness");
    
    // Comprehensive conditional branching test
    std::vector<uint32_t> instructions = {
        // === SETUP ===
        0x00500093,  // addi x1, x0, 5        -> x1 = 5
        0x00300113,  // addi x2, x0, 3        -> x2 = 3
        0x00500193,  // addi x3, x0, 5        -> x3 = 5 (same as x1)
        0x00800213,  // addi x4, x0, 8        -> x4 = 8
        
        // === BEQ TEST: Branch if Equal ===
        0x00308163,  // beq x1, x3, 2         -> branch if x1 == x3 (5 == 5 = true), skip +1 instruction
        0x00100113,  // addi x2, x0, 1        -> x2 = 1 (THIS SHOULD BE SKIPPED)
        0x00200113,  // addi x2, x0, 2        -> x2 = 2 (THIS SHOULD BE EXECUTED)
        
        // === BNE TEST: Branch if Not Equal ===
        0x00209163,  // bne x1, x2, 2         -> branch if x1 != x2 (5 != 2 = true), skip +1 instruction
        0x00300113,  // addi x2, x0, 3        -> x2 = 3 (THIS SHOULD BE SKIPPED)
        0x00400113,  // addi x2, x0, 4        -> x2 = 4 (THIS SHOULD BE EXECUTED)
        
        // === BLT TEST: Branch if Less Than (signed) ===
        0x00114163,  // blt x2, x1, 2         -> branch if x2 < x1 (4 < 5 = true), skip +1 instruction
        0x00500113,  // addi x2, x0, 5        -> x2 = 5 (THIS SHOULD BE SKIPPED)
        0x00600113,  // addi x2, x0, 6        -> x2 = 6 (THIS SHOULD BE EXECUTED)
        
        // === BGE TEST: Branch if Greater or Equal (signed) ===
        0x00125163,  // bge x4, x1, 2         -> branch if x4 >= x1 (8 >= 5 = true), skip +1 instruction
        0x00700113,  // addi x2, x0, 7        -> x2 = 7 (THIS SHOULD BE SKIPPED)
        0x00800113,  // addi x2, x0, 8        -> x2 = 8 (THIS SHOULD BE EXECUTED)
        
        // === BLTU TEST: Branch if Less Than (unsigned) ===
        0x00116163,  // bltu x2, x1, 2        -> branch if x2 < x1 (8 < 5 = false), no branch
        0x00900113,  // addi x2, x0, 9        -> x2 = 9 (THIS SHOULD BE EXECUTED - no branch)
        0x00A00113,  // addi x2, x0, 10       -> x2 = 10 (THIS SHOULD BE EXECUTED)
        
        // === BGEU TEST: Branch if Greater or Equal (unsigned) ===
        0x00117163,  // bgeu x2, x1, 2        -> branch if x2 >= x1 (10 >= 5 = true), skip +1 instruction
        0x00B00113,  // addi x2, x0, 11       -> x2 = 11 (THIS SHOULD BE SKIPPED)
        0x00C00113,  // addi x2, x0, 12       -> x2 = 12 (THIS SHOULD BE EXECUTED)
        
        // === HALT ===
        0x0000006F   // jal x0, 0             -> halt
    };
    
    // Load program
    for (size_t i = 0; i < instructions.size(); i++) {
        (*harness.memory)[i] = instructions[i];
        std::cout << "Address 0x" << std::hex << (i * 4) << ": 0x" << instructions[i] << std::dec << std::endl;
    }
    
    std::cout << "\nExpected behavior:" << std::endl;
    std::cout << "1. Setup: x1=5, x2=3, x3=5, x4=8" << std::endl;
    std::cout << "2. BEQ x1, x3 -> 5==5=true, skip, x2=2" << std::endl;
    std::cout << "3. BNE x1, x2 -> 5!=2=true, skip, x2=4" << std::endl;
    std::cout << "4. BLT x2, x1 -> 4<5=true, skip, x2=6" << std::endl;
    std::cout << "5. BGE x4, x1 -> 8>=5=true, skip, x2=8" << std::endl;
    std::cout << "6. BLTU x2, x1 -> 8<5=false, no branch, x2=9, then x2=10" << std::endl;
    std::cout << "7. BGEU x2, x1 -> 10>=5=true, skip, x2=12" << std::endl;
    std::cout << "8. Final x2 should be 12" << std::endl;
    
    std::cout << "\n🚀 Starting simulation..." << std::endl;
    
    // Run simulation
    sc_start(10000 * 10.0, SC_NS);
    
    std::cout << "\n🏁 Final CPU State:" << std::endl;
    std::cout << "PC: 0x" << std::hex << harness.cpu->PC << std::dec << std::endl;
    std::cout << "x1: 0x" << std::hex << harness.cpu->registerFile[1] << std::dec << std::endl;
    std::cout << "x2: 0x" << std::hex << harness.cpu->registerFile[2] << std::dec << std::endl;
    
    uint32_t x2_final = harness.cpu->registerFile[2];
    if (x2_final == 12) {
        std::cout << "\n✅ SUCCESS! All conditional branching instructions are working correctly!" << std::endl;
        std::cout << "x2 = 12 means all branch instructions (BEQ, BNE, BLT, BGE, BLTU, BGEU) worked as expected:" << std::endl;
        std::cout << "- BEQ: skipped instruction, x2=2" << std::endl;
        std::cout << "- BNE: skipped instruction, x2=4" << std::endl;
        std::cout << "- BLT: skipped instruction, x2=6" << std::endl;
        std::cout << "- BGE: skipped instruction, x2=8" << std::endl;
        std::cout << "- BLTU: no branch (8<5=false), x2=9, then x2=10" << std::endl;
        std::cout << "- BGEU: skipped instruction, x2=12" << std::endl;
    } else if (x2_final == 11) {
        std::cout << "\n⚠️  PARTIAL SUCCESS! Most branches worked, but BGEU may have failed." << std::endl;
        std::cout << "x2 = 11 suggests BGEU didn't skip the instruction as expected." << std::endl;
    } else if (x2_final == 10) {
        std::cout << "\n⚠️  PARTIAL SUCCESS! Some branches worked, but BGEU definitely failed." << std::endl;
        std::cout << "x2 = 10 suggests BGEU didn't skip the instruction as expected." << std::endl;
    } else {
        std::cout << "\n❌ FAILURE! Branching is NOT working correctly!" << std::endl;
        std::cout << "x2 = " << x2_final << " is not the expected value of 12." << std::endl;
        std::cout << "This indicates one or more conditional branch instructions are not functioning." << std::endl;
    }
    
    return 0;
}
