#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "imemory.h"
#include "memory.h"
#include "cache.h"

enum cpu_instr_T { ADD = 0, ADDI = 1, MUL = 2, INV = 3, BRANCH = 4, LOAD = 5, STORE = 6, HALTINSTR = 7};
enum cpu_branch_T { BEQ = 0, BNEQ = 1, BLT = 2};
static uint8_t regs[8]; // CPU Registers RA-RH
static uint8_t pc; // Index into imemory
static uint16_t tc; // Counts the total number of ticks acted on the cpu
enum cpuStates { IDLE, INSTRUCTION, WAIT, HALTSTATE };
static enum cpuStates cpuState = IDLE; // Defualt state: IDLE
static bool fetchDone = false; // Indicates whether a fetch is complete
static uint8_t fetchByte; // The byte to fetch
static unsigned cpuTicks; // Keep track of the tick count for instructions
static unsigned instr;    // The instruction to perform 
static uint8_t instrCode = 0; // The code of the instruction to perform
static uint8_t destReg = 0;   // The destination register from the instruction
static uint8_t srcReg = 0;    // The source register from the instruction
static uint8_t trgtReg = 0;   // The target register from the instruction
static uint8_t imValue = 0;   // The immediate values in the instruction

// Clear the cpu's registers
static void cpuReset() {

  for (int i = 0; i < 8; i++) {
    regs[i] = 0;
  }
  pc = 0;
  tc = 0;
  cpuState = IDLE;
}

// Set the data in a register
static void cpuSetReg(FILE *infile) {

  char regChar[3]; // The reg to set
  unsigned reg;    // The reg to set as an int
  unsigned inByte; // The input byte to set

  // Get the reg
  fscanf(infile, "%2s", regChar);

  // Get the input byte to set to the reg
  fscanf(infile, "%x", &inByte);

  // If the register to set is the PC
  if (0 == strcmp(regChar, "PC")) {
    pc = inByte;
    cpuState = INSTRUCTION; // Cancel any current instruction and move to fetch instruction state
  }

  // Otherwise it is RA-RH
  else {
    // Extract the register from reg
    reg = regChar[1];
    reg = reg - 'A'; // Translate the reg to the index

    // Set the value in the reg
    regs[reg] = inByte;
  }
}

// Dump the contents of the cpu
static void cpuDump() {

  // Print the PC content
  printf("PC: 0x%02X\n", pc);

  // Print the data registers (65 is ascii A, 73 is ascii I)
  for (int i = 'A'; i < 'I'; i++) {
    // i is used as the ascii value of the register's letter
    printf("R%c: 0x%02X\n", i, regs[i - 'A']);
  }

  // Print the TC content
  printf("TC: %d\n\n", tc);
}

// Fetch an instruction from imemory at pc
static unsigned fetchInstruction() {
  unsigned instr; // The instruction fetched
  instr = iMemFetch(pc); // Get the instruction from imemory at address pc
  return instr;
}

// Handle start ticks
void cpuStartTick() {
  //Determine if the cpu is in its haltstate, if it is, do not perform a tick
  if(cpuState != HALTSTATE) {
    // Increment the tc reg
    tc++;
    
    // If the cpu is in its IDLE state it needs to get an instruction 
    // and execute it
    if (IDLE == cpuState) {
      cpuState = INSTRUCTION; // Change the state
    }

    // If a branch or multiply instuction is being performed, increment cpuTicks
    else if ((WAIT == cpuState && BRANCH == instrCode) ||
              (WAIT == cpuState && MUL == instrCode)) {
      cpuTicks++;
    }
  }
}

// Check and see if the cpu has more work to do in this cycle
bool cpuIsMoreCycleWorkNeeded() {
  // Check and see if the instruction has been complete
  if ((WAIT == cpuState) && fetchDone) {
    return true; 
  }
  
  return false; 
}

// Perform the work done in a clock tick
void cpuDoCycleWork() {

  // If the state is INSTRUCTION, fetch an instruction
  if (cpuState == INSTRUCTION) {
    // Fetch an instruction
    instr = fetchInstruction();

    // Decode the instruction and execute it
    instrCode = (instr >> 17) & 0x7;
    destReg = (instr >> 14) & 0x7;
    srcReg = (instr >> 11) & 0x7;
    trgtReg = (instr >> 8) & 0x7;
    imValue = instr & 0xFF;

    // If instrCode is equivalent to 0, perform the add operation
    if(ADD == instrCode) {
      // Get the value at srcReg and at it to the value at trgtReg and save it to destReg
      regs[destReg] = regs[srcReg] + regs[trgtReg];

      pc++; // Increment pc
      cpuState = IDLE; // Change the state to "IDLE
    }

    // If instrCode is equivalent to 1, perform the addi instruction
    else if(ADDI == instrCode) {
      // Get the value at srcReg and at it to the immediate value and save it to destReg
      regs[destReg] = regs[srcReg] + imValue;

      pc++; // Increment pc
      cpuState = IDLE; // Change the state to "IDLE
    }

    // If instrCode is equivalent to 2, start the mul instruction
    else if(MUL == instrCode) {
      // Change the state to "WAIT"
      cpuState = WAIT;
    }

    // If instrCode is equivalent to 3, perform the inv instruction
    else if(INV == instrCode) {
      // Get the value at srcReg and at it to the immediate value and save it to destReg
      regs[destReg] = ~regs[srcReg];

      pc++; // Increment pc
      cpuState = IDLE; // Change the state to "IDLE"
    }

    // If instrCode is equivalent to 4, start the beq, bneq, or blt instruction
    else if(BRANCH == instrCode) {
      cpuState = WAIT; // Change the state to "WAIT"
    }

    // If instrCode is equivalent to 5, start load word instruction
    else if (LOAD == instrCode) {
      
      // Start the load from cache
      cacheStartFetch(regs[trgtReg], &fetchByte, &fetchDone);

      cpuState = WAIT; // Change the state to "WAIT"
    }

    // If instrCode is equivalent to 6, start store word instruction
    else if (STORE == instrCode) {
      
      // Start the store word in cache
      cacheStartStore(regs[trgtReg], &regs[srcReg], &fetchDone);

      cpuState = WAIT; // Change the state to "WAIT"
    }

    // If instrCode is equivalent to 7, execute halt instruction
    else if (HALTINSTR == instrCode) {
      cpuState = HALTSTATE;// Change the state to "HALTSTATE"
      pc++;
    }
  }

  // If cpuState is WAIT and fetchDone is true
  // Complete the instruction
  if ((WAIT == cpuState) && fetchDone) {

    // If instrCode is equivalent to 5, complete load word instruction
    if (LOAD == instrCode) {      
      // set the fetch by to the destination register
      regs[destReg] = fetchByte;
      
      cpuState = IDLE; // Change the state to "IDLE"
      fetchDone = false; // Change fetchDone back to false
    }

    // If instrCode is equivalent to 6, complete save word instruction
    else if (STORE == instrCode) {
      cpuState = IDLE; // Change the state to "IDLE"
      fetchDone = false; // Change fetchDone back to false
    }
    
    // Now that the instruction is complete, increment PC
    pc++; 
  }   

  // Finish the branch instructions
  else if(WAIT == cpuState && BRANCH == instrCode) {

    // Perform a beq, bneq, or blt instruction
    if(BEQ == destReg) {
      // If the contents are equal, branch if cpuTicks is 1 (second cycle)
      if(regs[srcReg] == regs[trgtReg]) {
        if(1 == cpuTicks) {
          pc = imValue; // Branch to the line in imValue
          cpuTicks = 0; // After the instruction, reset ticks
          cpuState = IDLE; // Put the cpu back in IDLE
        }
      } 
      else { 
        pc++; // If they are not equal, branch in 0 additional cycles
        cpuState = IDLE; // Put the cpu back in IDLE
      }
    }
    else if (BNEQ == destReg) {
      // If the contents are not equal, branch if cpuTicks is 1 (second cycle)
      if(regs[srcReg] != regs[trgtReg]) {
        if(1 == cpuTicks) {
          pc = imValue; // Branch to the line in imValue
          cpuTicks = 0; // After the instruction, reset ticks
          cpuState = IDLE; // Put the cpu back in IDLE
        }
      } 
      else { 
        pc++; // If they are equal, branch in 0 additional cycles
        cpuState = IDLE; // Put the cpu back in IDLE
      }
    }
    else if (BLT == destReg) {
      // If content in srcReg is less than in trgtReg, branch if cpuTicks is 1 (second cycle)
      if(regs[srcReg] < regs[trgtReg]) {
        if(1 == cpuTicks) {
          pc = imValue; // Branch to the line in imValue
          cpuTicks = 0; // After the instruction, reset ticks
          cpuState = IDLE; // Put the cpu back in IDLE
        }
      } 
      else { 
        pc++; // If they are equal or it is greater, branch in 0 additional cycles
        cpuState = IDLE; // Put the cpu back in IDLE
      }
    }
  }

  // Finish the multiply instruction
  else if(WAIT == cpuState && MUL == instrCode) {
    
    // On the second tick complete the mul instruction
    if (1 == cpuTicks) {
      // Get the terms
      uint8_t term1 = regs[srcReg]&0x0F; // Bits [0:3] from the source reg
      uint8_t term2 = regs[srcReg]>>4; // Bits [4:7] from the source reg

      // Multiply the terms and save them to the destination register
      regs[destReg] = term1 * term2;

      cpuState = IDLE;// Change the state to "IDLE"
      pc++; // Increment PC
      cpuTicks = 0; // Reset cpuTicks
    }
  }
}

// Read cpu commands from the file and call the functions
void parseCpu(FILE *infile) {

  char cmd[11]; // Holds the command

  // Get the command to execute
  fscanf(infile, "%10s", cmd);

  // Call the command's function
  // Calls the reset function
  if (0 == strcmp(cmd, "reset")) {
    cpuReset();
  }
  // Calls the set function
  else if (0 == strcmp(cmd, "set")) {

    // Read "reg" to "skip" over it
    fscanf(infile, "%10s", cmd);

    cpuSetReg(infile);

  }
  // Calls the dump function
  else if (0 == strcmp(cmd, "dump")) {
    cpuDump();
  }
}