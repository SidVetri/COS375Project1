/*
COS375 Project 1

Authors: Jack Zhang (jz4267) and Siddarth Vetrivel 
*/

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
    // delaySlot1 is set to true if the current instruction requires a delay slot after it (for jumps and branches)
    // delaySlot2 is set to true during the instruction that is next in memory, and after that instruction is run, the PC is set to the new branch/jump location
    bool delaySlot1 = false;
    bool delaySlot2 = false;
    bool jump = false;
    uint32_t delaySlotBranchAdr = 0;
    uint32_t delaySlotJumpAdr = 0;

    uint32_t value;
    // start simulation
    // TODO: complete simulation loop and implement branch delay logic
    while (!err) {
        value = 0;
        // fetch current instruction
        uint32_t instruction;
        myMem->getMemValue(PC, instruction, WORD_SIZE);
    
        

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
        uint32_t opcode = extractBits(instruction, 31, 26);
        uint32_t rs = extractBits(instruction, 25, 21);
        uint32_t rt = extractBits(instruction, 20, 16);
        uint32_t rd = extractBits(instruction, 15, 11);
        uint32_t shamt = extractBits(instruction, 10, 6);
        uint32_t funct = extractBits(instruction, 5, 0);
        uint16_t immediate = extractBits(instruction, 15, 0);
        uint32_t address = extractBits(instruction, 15, 0);

        int32_t signExtImm = signExt(immediate);
        uint32_t zeroExtImm = (0x0000 << 16) | immediate;

        uint32_t branchAddr = signExtImm << 2;
        uint32_t jumpAddr = (PC & 0xf0000000) | (address << 2); // assumes PC += 4 just happened

      
        switch(opcode) {
            case OP_ZERO: // R-type instruction 
                switch(funct) {
                    case FUN_ADD:                         
                        regData.registers[rd] = regData.registers[rs] + regData.registers[rt];
                        break;
                    case FUN_ADDU: 
                        regData.registers[rd] = regData.registers[rs] + regData.registers[rt];
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
                        regData.registers[rd] = (int32_t(regData.registers[rs]) < int32_t(regData.registers[rt])) ? 1 : 0;
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
                regData.registers[rt] = regData.registers[rs] + signExtImm;
                break;
            case OP_ADDIU: 
                regData.registers[rt] = regData.registers[rs] + signExtImm;
                break;
            case OP_ANDI: 
                regData.registers[rt] = regData.registers[rs] & zeroExtImm;
                break;
            case OP_BEQ: 
                
                if (int32_t(regData.registers[rs]) == int32_t(regData.registers[rt]))
                {
                    delaySlotBranchAdr = branchAddr;
                    delaySlot1 = true;
                }
                
                
                break;
            case OP_BNE:
                
                
                if (int32_t(regData.registers[rs]) != int32_t(regData.registers[rt]))
                {
                    delaySlotBranchAdr = branchAddr;
                    delaySlot1 = true;
                }
            
                break;
            case OP_BLEZ: 
                
                if (int32_t(regData.registers[rs]) <= 0)
                {
                    delaySlotBranchAdr = branchAddr;
                    delaySlot1 = true;
                }
                
                break;
            case OP_BGTZ: 
                
                if (int32_t(regData.registers[rs]) > 0)
                {
                    delaySlotBranchAdr = branchAddr;
                    delaySlot1 = true;
                }
                
                
                break;
            case OP_J: 
                delaySlotJumpAdr = jumpAddr;
                delaySlot1 = true;
                jump = true;
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
                myMem->getMemValue((regData.registers[rs]+signExtImm), value, WORD_SIZE);
                regData.registers[rt] = value;
                break;
            case OP_ORI: 
                regData.registers[rt] = regData.registers[rs] | signExtImm;
                break;
            case OP_SLTI: 
                regData.registers[rt] = (int32_t(regData.registers[rs]) < signExtImm) ? 1 : 0;
                break;
            case OP_SLTIU: 
                regData.registers[rt] = (uint32_t(regData.registers[rs]) < uint32_t(signExtImm)) ? 1 : 0;
                break;
            case OP_SB: 
                value = extractBits(regData.registers[rt], 7, 0);
                myMem->setMemValue((regData.registers[rs]+signExtImm),value, BYTE_SIZE);
                break;                
            case OP_SH: 
                value = extractBits(regData.registers[rt], 15, 0);
                printf("%x\n", value);
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
            if(jump){
                PC = delaySlotJumpAdr;
            }
            else{
                PC += delaySlotBranchAdr - 4;
            }
            delaySlotBranchAdr = 0;
            delaySlotJumpAdr = 0;
            jump = false;
        }
    }


   
    // dump and exit with error
    dump(myMem);
    exit(127);
    return -1;  
}

