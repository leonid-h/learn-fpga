/*******************************************************************/
// FemtoRV32 Quark - SystemC Implementation
// Translated from Verilog to SystemC C++
// 
// This version: The "Quark", the most elementary version of FemtoRV32.
//             A single SystemC module, compact & understandable code.
//
// Instruction set: RV32I + RDCYCLES
//
// Parameters:
//  Reset address can be defined using RESET_ADDR (default is 0).
//  The ADDR_WIDTH parameter lets you define the width of the internal
//  address bus (and address computation logic).
//
// Bruno Levy, Matthias Koch, 2020-2021
// SystemC Translation: 2024
/*******************************************************************/

#ifndef FEMTORV32_QUARK_H
#define FEMTORV32_QUARK_H

#include <systemc.h>
#include <vector>

// Default parameters
#define DEFAULT_RESET_ADDR 0x00000000
#define DEFAULT_ADDR_WIDTH 24

// State machine states
enum State {
    FETCH_INSTR     = 0,
    WAIT_INSTR      = 1,
    EXECUTE         = 2,
    WAIT_ALU_OR_MEM = 3,
    NB_STATES       = 4
};

// ALU function codes (funct3)
enum ALUFunction {
    ALU_ADD_SUB = 0,  // ADD, SUB, ADDI
    ALU_SLL     = 1,  // SLL, SLLI
    ALU_SLT     = 2,  // SLT, SLTI
    ALU_SLTU    = 3,  // SLTU, SLTIU
    ALU_XOR     = 4,  // XOR, XORI
    ALU_SRL_SRA = 5,  // SRL, SRA, SRLI, SRAI
    ALU_OR      = 6,  // OR, ORI
    ALU_AND     = 7   // AND, ANDI
};

// Branch function codes (funct3)
enum BranchFunction {
    BRANCH_BEQ  = 0,  // BEQ
    BRANCH_BNE  = 1,  // BNE
    BRANCH_BLT  = 4,  // BLT
    BRANCH_BGE  = 5,  // BGE
    BRANCH_BLTU = 6,  // BLTU
    BRANCH_BGEU = 7   // BGEU
};

// Load/Store function codes (funct3)
enum LoadStoreFunction {
    LOAD_STORE_BYTE  = 0,  // LB, LBU, SB
    LOAD_STORE_HALF  = 1,  // LH, LHU, SH
    LOAD_STORE_WORD  = 2   // LW, SW
};

SC_MODULE(FemtoRV32_Quark) {
    // Ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    
    // Memory interface
    sc_out<sc_uint<32> > mem_addr;
    sc_out<sc_uint<32> > mem_wdata;
    sc_out<sc_uint<4> >  mem_wmask;
    sc_in<sc_uint<32> >  mem_rdata;
    sc_out<bool>         mem_rstrb;
    sc_in<bool>          mem_rbusy;
    sc_in<bool>          mem_wbusy;
    
    // Constructor
    SC_CTOR(FemtoRV32_Quark) : 
        RESET_ADDR(DEFAULT_RESET_ADDR),
        ADDR_WIDTH(DEFAULT_ADDR_WIDTH),
        PC(0),
        state(WAIT_ALU_OR_MEM),
        cycles(0),
        registerFile(32, 0),
        aluReg(0),
        aluShamt(0) {
        
        // Register processes
        SC_METHOD(clock_process);
        sensitive << clk;
        
        SC_METHOD(combinational_process);
        sensitive << clk << reset << mem_rdata << mem_rbusy << mem_wbusy;
    }
    
    // Parameters
    const sc_uint<32> RESET_ADDR;
    const int ADDR_WIDTH;
    
    // Internal signals and registers
    sc_uint<32> PC;
    sc_uint<30> instr;  // 30 bits (bits 0,1 ignored in RV32I)
    sc_uint<32> full_instr;  // Full 32-bit instruction for immediate decoding
    State state;
    sc_uint<32> cycles;
    
    // Register file
    std::vector<sc_uint<32> > registerFile;
    
    // ALU registers
    sc_uint<32> aluReg;
    sc_uint<5>  aluShamt;
    
    // Decoded instruction fields
    sc_uint<5>  rdId;
    sc_uint<5>  rs1Id;
    sc_uint<5>  rs2Id;
    sc_uint<3>  funct3;
    sc_uint<7>  opcode;
    sc_uint<32> rs1;
    sc_uint<32> rs2;
    
    // Immediate values
    sc_uint<32> Uimm, Iimm, Simm, Bimm, Jimm;
    
    // Instruction type flags
    bool isLoad, isALUimm, isStore, isALUreg, isSYSTEM;
    bool isJAL, isJALR, isLUI, isAUIPC, isBranch, isALU;
    
    // ALU signals
    sc_uint<32> aluIn1, aluIn2, aluOut;
    sc_uint<32> aluPlus;
    sc_uint<33> aluMinus;
    bool LT, LTU, EQ;
    bool aluBusy, aluWr;
    bool funct3IsShift;
    
    // Branch predicate
    bool predicate;
    
    // Memory access signals
    bool mem_byteAccess, mem_halfwordAccess;
    sc_uint<32> loadstore_addr;
    sc_uint<32> LOAD_data;
    sc_uint<16> LOAD_halfword;
    sc_uint<8>  LOAD_byte;
    bool LOAD_sign;
    sc_uint<4>  STORE_wmask;
    
    // Control signals
    bool writeBack;
    bool jumpToPCplusImm;
    bool needToWait;
    
    // Address computation
    sc_uint<32> PCplus4, PCplusImm;
    
    // Write-back data
    sc_uint<32> writeBackData;
    
    // Process declarations
    void clock_process();
    void combinational_process();
    
    // Helper functions
    void decode_instruction();
    void compute_immediates();
    void compute_alu();
    void compute_branch_predicate();
    void compute_memory_access();
    void update_state();
    void update_pc();
    void update_registers();
    
    // Utility functions
    sc_uint<32> sign_extend(sc_uint<32> value, int bits);
    bool is_io_addr(sc_uint<32> addr);
};

#endif // FEMTORV32_QUARK_H
