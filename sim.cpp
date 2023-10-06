#include <stdio.h>
#include "MemoryStore.h"
#include "RegisterInfo.h"
#include "EndianHelpers.h"
#include <fstream>
#include <iostream>
#include <bitset>
using namespace std; 

union REGS 
{
    RegisterInfo reg;
    uint32_t registers[32] {0};
};

union REGS regData;

// TODO: fill in the missing hex values of the OP_IDs (opcodes)
enum OP_IDS
{
    //R-type opcodes...
    OP_ZERO = 0x0, // zero
    //I-type opcodes...
    OP_ADDI = 0x08, // addi
    OP_ADDIU = 0x09, // addiu
    OP_ANDI = 0x0c, // andi
    OP_BEQ = 0x04, // beq
    OP_BNE = 0x05, // bne
    OP_LBU = 0x24, // lbu
    OP_LHU = 0x25, // lhu
    OP_LUI = 0x0f, // lui
    OP_LW = 0x23, // lw
    OP_ORI = 0x0d, // ori
    OP_SLTI = 0x0a, // slti
    OP_SLTIU = 0x0b, // sltiu
    OP_SB = 0x28, // sb
    OP_SH = 0x29, // sh
    OP_SW = 0x2b, // sw
    OP_BLEZ = 0x06, // blez
    OP_BGTZ = 0x07, // bgtz
    //J-type opcodes...
    OP_J = 0x02, // j
    OP_JAL = 0x03 // jal
};

// TODO: fill in the missing hex values of FUNCT_IDs (function IDs)
enum FUNCT_IDS
{
    FUN_ADD = 0x20, // add
    FUN_ADDU = 0x21, // add unsigned (addu)
    FUN_AND = 0x24, // and
    FUN_JR = 0x08, // jump register (jr)
    FUN_NOR = 0x27, // nor
    FUN_OR = 0x25, // or
    FUN_SLT = 0x2a, // set less than (slt)
    FUN_SLTU = 0x2b, // set less than unsigned (sltu)
    FUN_SLL = 0x00, // shift left logical (sll)
    FUN_SRL = 0x02, // shift right logical (srl)
    FUN_SUB = 0x22, // substract (sub)
    FUN_SUBU = 0x23 // substract unsigned (subu)
};

// extract specific bits [start, end] from a 32 bit instruction
uint32_t extractBits(uint32_t instruction, int start, int end)
{
    int bitsToExtract = start - end + 1;
    uint32_t mask = (1 << bitsToExtract) - 1;
    uint32_t clipped = instruction >> end;
    return clipped & mask;
}

// sign extend smol to a 32 bit unsigned int
uint32_t signExt(uint16_t smol) 
{
    uint32_t x = smol;
    uint32_t extension = 0xffff0000;
    return (smol & 0x8000) ? x ^ extension : x;
}

// dump registers and memory
void dump(MemoryStore *myMem) {

    dumpRegisterState(regData.reg);
    dumpMemoryState(myMem);
}

int main(int argc, char** argv) {

    // open instruction file
    ifstream infile;
    infile.open(argv[1], ios::binary | ios::in);

    // get length of the file and read instruction file into a buffer
    infile.seekg(0, ios::end);
    int length = infile.tellg();
    infile.seekg (0, ios::beg);

    char *buf = new char[length];
    infile.read(buf, length);
    infile.close();

    // initialize memory store with buffer contents
    MemoryStore *myMem = createMemoryStore();
    int memLength = length / sizeof(buf[0]);
    int i;
    for (i = 0; i < memLength; i++) {
        myMem->setMemValue(i * BYTE_SIZE, buf[i], BYTE_SIZE);
    }

    // initialize registers and our program counter
    regData.reg = {};
    uint32_t PC = 0;
    bool err = false;
    
    // variables to handle branch delay slot execution
    bool delaySlot1 = false;
    bool delaySlot2 = false;
    uint32_t delaySlotBranchAdr = 0;

    // start simulation
    // TODO: complete simulation loop and implement branch delay logic
    while (!err) {
        // fetch current instruction
        uint32_t instruction;
        myMem->getMemValue(PC, instruction, WORD_SIZE);

        
        std::cout << "instruction = " << std::bitset<32>(instruction)  << std::endl;
        instruction = ConvertWordToBigEndian(instruction);
        std::cout << "instruction = " << std::bitset<32>(instruction)  << std::endl;
        
        uint32_t next_instruction;

        if (delaySlot1) {
            delaySlot2 = true;
        }
        
        // increment PC & reset zero register
        PC += 4;
        regData.registers[0] = 0;

        // check for halt instruction
        if (instruction == 0xfeedfeed) {
            dump(myMem);
            return 0;
        }

        // TODO: parse instruction by completing function calls to extractBits()
        // and set operands accordingly
        uint32_t opcode = extractBits(instruction, 26, 31);
        uint32_t rs = extractBits(instruction, 21, 25);
        uint32_t rt = extractBits(instruction, 16, 20);
        uint32_t rd = extractBits(instruction, 11, 15);
        uint32_t shamt = extractBits(instruction, 6, 10);
        uint32_t funct = extractBits(instruction, 0, 5);
        uint16_t immediate = extractBits(instruction, 0, 15);
        uint32_t address = extractBits(instruction, 0, 15);

        int32_t signExtImm = signExt(immediate);
        uint32_t zeroExtImm = (0x0000 << 16) | immediate;

        uint32_t branchAddr = (0x0000 << 16) | immediate;
        uint32_t jumpAddr = (0x0000 << 6) | address; // assumes PC += 4 just happened

        uint32_t value = 0;
        switch(opcode) {
            case OP_ZERO: // R-type instruction 
                switch(funct) {
                    case FUN_ADD:                         
                        regData.registers[rd] = regData.registers[rs] + regData.registers[rt];
                        break;
                    case FUN_ADDU: 
                        regData.registers[rd] = uint32_t(regData.registers[rs]) + uint32_t(regData.registers[rt]);
                        break;
                    case FUN_AND: 
                        regData.registers[rd] = regData.registers[rs] & regData.registers[rt];
                        break;
                    case FUN_JR: 
                        PC = regData.registers[rs];
                        break;
                    case FUN_NOR: 
                        regData.registers[rd] = !(regData.registers[rs] | regData.registers[rt]);
                        break;
                    case FUN_OR: 
                        regData.registers[rd] = (regData.registers[rs] | regData.registers[rt]);
                        break;
                    case FUN_SLT: 
                        regData.registers[rd] = (regData.registers[rs] < regData.registers[rt]) ? 1 : 0;
                        break;
                    case FUN_SLTU: 
                        regData.registers[rd] = (uint32_t(regData.registers[rs]) < uint32_t(regData.registers[rt])) ? 1 : 0;
                        break;
                    case FUN_SLL: 
                        regData.registers[rd] = regData.registers[rt] << shamt;
                        break;
                    case FUN_SRL: 
                        regData.registers[rd] = regData.registers[rt] >> shamt;
                        break;
                    case FUN_SUB:  
                        regData.registers[rd] = regData.registers[rs] - regData.registers[rt];
                        break;
                    case FUN_SUBU: 
                        regData.registers[rd] = unsigned(regData.registers[rs]) - unsigned(regData.registers[rt]);
                        break;
                    default:
                        fprintf(stderr, "\tIllegal operation...\n");
                        err = true;
                }
                break;

            case OP_ADDI: 
                regData.registers[rd] = regData.registers[rs] + signExtImm;
                break;
            case OP_ADDIU: 
                regData.registers[rt] = regData.registers[rs] + signExtImm;
                break;
            case OP_ANDI: 
                regData.registers[rt] = regData.registers[rs] & zeroExtImm;
                break;
            case OP_BEQ: 
                if (myMem->getMemValue(PC, instruction, WORD_SIZE) == 0xfeedfeed){
                    if (regData.registers[rs] == regData.registers[rt])
                        PC += branchAddr-4;
                }
                else{
                    if (regData.registers[rs] == regData.registers[rt])
                        delaySlotBranchAdr = branchAddr;
                        delaySlot1 = true;
                }
                break;
            case OP_BNE:
                if (myMem->getMemValue(PC, instruction, WORD_SIZE) == 0xfeedfeed){
                    if (regData.registers[rs] != regData.registers[rt])
                            PC += branchAddr-4;
                }
                else{
                    if (regData.registers[rs] != regData.registers[rt])
                        delaySlotBranchAdr = branchAddr;
                        delaySlot1 = true;
                }
                break;
            case OP_BLEZ: 
                if (myMem->getMemValue(PC, instruction, WORD_SIZE) == 0xfeedfeed){
                    if (regData.registers[rs] <= 0)
                            PC += branchAddr-4;
                }
                else{
                    if (regData.registers[rs] <= 0)
                        delaySlotBranchAdr = branchAddr;
                        delaySlot1 = true;
                }
                break;
            case OP_BGTZ: 
                if (myMem->getMemValue(PC, instruction, WORD_SIZE) == 0xfeedfeed){
                    if (regData.registers[rs] > 0)
                            PC += branchAddr-4;
                }
                else{
                    if (regData.registers[rs] > 0)
                        delaySlotBranchAdr = branchAddr;
                        delaySlot1 = true;
                }
                break;
            case OP_J: 
                PC = jumpAddr;
                break;
            case OP_JAL: 
                regData.registers[31] = PC + 4;
                PC = jumpAddr;
                break;
            case OP_LBU: 
                myMem->getMemValue((regData.registers[rs]+signExtImm),value,BYTE_SIZE);
                regData.registers[rt] = (0x000000 << 8) | value;
                break;
            case OP_LHU: 
                myMem->getMemValue((regData.registers[rs]+signExtImm),value, HALF_SIZE);
                regData.registers[rt] = (0x0000 << 16) | value;
                break;
            case OP_LUI: 
                regData.registers[rt] = (immediate << 16) | 0x00000000;
                break;
            case OP_LW: 
                myMem->getMemValue((regData.registers[rs]+signExtImm),value, WORD_SIZE);
                regData.registers[rt] = value;
                break;
            case OP_ORI: 
                regData.registers[rt] = regData.registers[rs] | signExtImm;
                break;
            case OP_SLTI: 
                regData.registers[rt] = (regData.registers[rs] < signExtImm) ? 1 : 0;
                break;
            case OP_SLTIU: 
                regData.registers[rt] = (regData.registers[rs] < signExtImm) ? 1 : 0;
                break;
            case OP_SB: 
                value = extractBits(regData.registers[rt], 0, 7);
                myMem->setMemValue((regData.registers[rs]+signExtImm),value, BYTE_SIZE);
                break;                
            case OP_SH: 
                value = extractBits(regData.registers[rt], 0, 15);
                myMem->setMemValue((regData.registers[rs]+signExtImm),value, HALF_SIZE);
                break;
            case OP_SW: 
                value = regData.registers[rt];
                myMem->setMemValue((regData.registers[rs]+signExtImm),value, WORD_SIZE);
                break;
            default:
                fprintf(stderr, "\tIllegal operation...\n");
                err = true;
        }

        if(delaySlot2 && delaySlot1)
        {
            delaySlot2 = false;
            delaySlot1 = false;
            PC += delaySlotBranchAdr - 8;
            delaySlotBranchAdr = 0;
        }
    }


   
    // dump and exit with error
    dump(myMem);
    exit(127);
    return -1;  
}

