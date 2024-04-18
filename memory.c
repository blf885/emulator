#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static uint8_t *memPtr; // The memory array
static unsigned memSize; // The size of the memory array
static unsigned memAddress;
static unsigned memCount; 
static uint8_t* memAnswerPtr;
static bool* memValidPtr;
static bool* memDonePtr;
static unsigned memTicks;
enum memStates { IDLE, FETCH, STORE, MOVE_DATA, SAVE_DATA};
static enum memStates memState= IDLE; // Initialize state to idle


// Allocates the designated ammount of memory
static void memoryCreate(FILE *infile) {
  
  // Get the size
  fscanf(infile, "%x", &memSize);

  memPtr = malloc(memSize*sizeof(uint8_t));

}

// Reset the memory allocated to zeros
static void memoryReset() {

  for (unsigned i = 0; i < memSize; i++) {
    memPtr[i] = 0;
  }
}

// Dump the memory starting at the given address
// and dumping the given number of bytes
static void memoryDump(FILE *infile) {

  unsigned address; // The address to start at
  unsigned count; // The amount to print
  
  // Get the address
  fscanf(infile, "%x", &address);

  // Get the count
  fscanf(infile, "%x", &count);
  
  // Print the header
  printf("Addr");
  for (int i = 0; i < 16; i++) {
    printf(" %02X",i);
  }
  printf("\n"); 

  // Print the address of the start row
  printf("0x%02X",(address / 0x10) * 0x10);

  // Print the blank spaces before the first byte to print
  for (unsigned i = 0; i < (address % 0x10); i++) {
    printf("   ");
  }

  // Print the memory contents 
  for (unsigned i = address; i < address+count; i++) {
    printf(" %02X",memPtr[i]);

    // Print a newline at mutliples of 0x10
    if(15 == i % 0x10) {

      // Unless the last byte has been printed
      if(i != address+count-1) {     
        // Print a new line and the row's address label
        printf("\n0x%02X",(i+0x01));
      }
    }
  }
  // Put an empty newline after the dump
  printf("\n\n"); 
}

// Set the memory to the given values
static void memorySet(FILE *infile) {

  unsigned address; // The address to start at
  unsigned count; // The amount to print
  unsigned inByte; // The input byte
  
  // Get the address
  fscanf(infile, "%x", &address);

  // Get the count
  fscanf(infile, "%x", &count);

  // Set the values at the given address
  for (unsigned i = address; i < address+count; i++) {

    // Get the input byte
    fscanf(infile, "%x", &inByte);

    // Set the byte into memory
    memPtr[i] = inByte; 
  }
}

// Set up memory for the beginning of a cycle
void memStartTick() {
  // If memory is in its Fetch state or Store state, it must count 5 ticks
  if ((FETCH == memState) || (STORE == memState)) {
    memTicks++; // Increment ticks

    // On the fifth tick change the state to MOVE_DATA or STORE_DATA
    if (4 == memTicks) {
      // If the state is FETCH have it complete the FETCH instruction
      if(FETCH == memState) {
        memState = MOVE_DATA;  
      }
      // If the state is STORE have it complete the STORE instruction
      if(STORE == memState) {
        memState = SAVE_DATA;  
      }
    }
  }
}

// Check and see if the memory has more work to do in this cycle
bool memIsMoreCycleWorkNeeded() {

  // Do nothing for Assignment 2 
  
  return false; 
}

// Perform memory work
void memDoCycleWork() {

  // if memState is MOVE_DATA then move it
  if( MOVE_DATA == memState) {
    
    // Copy the memory contents to the answer pointer
    memcpy(memAnswerPtr, memPtr + memAddress, memCount);
    // could always use memcpy, but useful to show what
    // is going on with a single byte example
    *memDonePtr = true; // tell cache copy is done
    memState = IDLE; // Memory is ready for a new instruction
  } 
  
  // if memState is SAVE_DATA then move it
  if( SAVE_DATA == memState) {

    // Traverse the array and copy any values that indicate have been written
    for(int i=0; i<memCount; i++) {
      // If the value has been written
      if(memValidPtr[i]) {
        memPtr[memAddress+i] = memAnswerPtr[i]; // Update the value in memory
      }
    }
    *memDonePtr = true; // tell the device the copy is done
    memState = IDLE; // Memory is ready for a new instruction
  }
   
}

// Start a memory fetch at the given address
// address – the offset in memory where the read should begin
// count – the number of bytes that should be read
// dataPtr – a pointer where data should be placed
// memDonePtr – a pointer to a boolean that the Memory Device will set to true when
// the data transfer has completed (possibly multiple cycles after request)
void memStartFetch(unsigned address, unsigned count, uint8_t *dataPtr,
                   bool *donePtr) {
  memState = FETCH;
  memAddress = address;
  memCount = count;
  memAnswerPtr = dataPtr;
  memDonePtr = donePtr;
  memTicks = 0; // Reset the number of ticks to zero
}

// Start a memory store at the given address
// address – the offset in memory where the write should begin
// count – the number of bytes that should be written
// dataPtr – a pointer that is the source of data to write
// validPtr - pointer to list of Booleans indicating if matching byte should be written
// memDonePtr – a pointer to a boolean that the Memory Device will set to true when
void memStartStore(unsigned address, unsigned count, uint8_t *dataPtr, bool *validPtr,
                   bool *donePtr) {
  memState = STORE;
  memAddress = address;
  memCount = count;
  memAnswerPtr = dataPtr;
  memValidPtr = validPtr;
  memDonePtr = donePtr;
  memTicks = 0; // Reset the number of ticks to zero
}

// Free the memory
void memClean() {
  free(memPtr);
}

// Read memory commands from the file and call the functions
void parseMemory(FILE *infile) {

  char cmd[11]; // The command to do

  // Get the command to execute
  fscanf(infile, "%10s", cmd);

  // Call the command's function

  // Calls the create function
  if (0 == strcmp(cmd, "create")) {
    memoryCreate(infile);
  }
  // Calls the reset function
  else if (0 == strcmp(cmd, "reset")) {
    memoryReset();
  }
  // Calls the dump function
  else if (0 == strcmp(cmd, "dump")) {
    memoryDump(infile);
  }
  // Calls the set function
  else if (0 == strcmp(cmd, "set")) {
    memorySet(infile);
  }
}