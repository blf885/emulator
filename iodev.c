#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include "memory.h"

enum iodev_ops_T {READ = 1, WRITE = 2};
static uint8_t reg; // IO device register
static unsigned iodevTicks[100]; // Holds the ticks read in from the file sequentially
static enum iodev_ops_T iodevOps[100]; // Holds the operations read in from the file sequentially 
static unsigned iodevAddresses[100]; // Holds the Addresses read in from the file sequentially
static unsigned iodevValues[100]; // Holds the inpute values read in from the file sequentially
static bool iodevOpDone = true; // Indicates when the IO device has completed an operation with memory
static unsigned currentOp = 0; // Keeps track of the operation to be completed next
static uint8_t storeVal[1]; // Pointer that holds the value to be stored
static bool iodevValid[1]; // Tells memory the value stored in the register is valid
static uint16_t iodevTotTicks = 0; // Count the total ticks the device is called for from the clock

// Clear the io device's register
static void iodevReset() {
  reg = 0;

  // Clear the schedule arrays
  for(int i=0; i<100;i++) {
    iodevTicks[i] = 0;
    iodevOps[i] = 0;
    iodevAddresses[i] = 0;
    iodevValues[i] = 0;
  }
}

// Load the IO devices event schedule from a file
// This method works with two different files:
//     The input file with system commands called infile
//     The IO Device file with its event schedule called eventFile
static void iodevLoad(FILE *infile) {
    char eventFileName[NAME_MAX + PATH_MAX + 1]; // The path of the instruction file
    FILE *eventFile; // The file with the I/O event schedule
    unsigned tick; // The tick the event is to occur during
    char operation[6]; // "read" or "write"
    unsigned address; // The address on which to perform the operation
    unsigned inValue; // The input value to be written during a "write" operation

    // Read the event file's path
    fscanf(infile, "%1000s", eventFileName);

    // Open the instruction file
    eventFile = fopen(eventFileName, "r"); 

    // Index for setting the word in the schedule array
    unsigned i = 0;
  
    // Read the whole file and set the values at the given address
    while (1 == fscanf(eventFile, "%d", &tick)) { // Get the target tick
        iodevTicks[i] = tick; // Save the target tick to the array

        // Get the operation
        fscanf(eventFile, "%5s", operation);
    
        // Save the operation to the array
        if(0 == strcmp(operation, "read")) {
            iodevOps[i] = READ;
        } else if (0 == strcmp(operation, "write")) {
            iodevOps[i] = WRITE;
        }

        // Get the address
        fscanf(eventFile, "%x", &address);
        iodevAddresses[i] = address; // Save the address to the array

        // If the operation is a "write" get the value to write
        if(0 == strcmp(operation, "write")) {
            // Get the input value
            fscanf(eventFile, "%x", &inValue);
            iodevValues[i] = inValue; // Save the input value to the array
        }

         i++; // Increment the index variable

    }

    // Close the file
    fclose(eventFile);
}

// Dump the contents of the register
static void iodevDump() {
    printf("IO Device: 0x%02X\n\n", reg);
}

// Handle the work done in a tick
void iodevStartTick() {
    
    iodevTotTicks++;

    // Determine if there is an operation to complete on this tick
    if(iodevTotTicks == iodevTicks[currentOp]) {
        // Perform the appropriate operation
        if(iodevOps[currentOp] == READ) {
            memStartFetch(iodevAddresses[currentOp], 1, &reg, &iodevOpDone);
            currentOp++; // Increment to the next operation
        } else if (iodevOps[currentOp] == WRITE) {
            storeVal[0] = iodevValues[currentOp];
            iodevValid[0] = true; 
            memStartStore(iodevAddresses[currentOp], 1, storeVal, iodevValid, &iodevOpDone); 
            currentOp++; // Increment to the next operation      
        }
    }

    
}

// Read cpu commands from the file and call the functions
void parseIODevice(FILE *infile) {
    
    char cmd[11]; // Holds the command

    // Get the command to execute
    fscanf(infile, "%10s", cmd);

    // Call the command's function
    // Calls the reset function
    if (0 == strcmp(cmd, "reset")) {
        iodevReset();
    }
    
    // Calls the load function 
    else if (0 == strcmp(cmd, "load")) {
        iodevLoad(infile);
    }

    // Calls the dump function
    else if (0 == strcmp(cmd, "dump")) {
        iodevDump();
    }
}
