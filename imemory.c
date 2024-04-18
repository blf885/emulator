#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

static unsigned *iMemPtr; // The imemory array
static unsigned iMemSize; // The size of the imemory array

// Allocates the designated ammount of imemory
static void iMemoryCreate(FILE *infile) {
  // Get the size
  fscanf(infile, "%x", &iMemSize);

  iMemPtr = malloc(iMemSize*sizeof(unsigned));
}

// Reset the imemory allocated to zeros
static void iMemoryReset() {
  
  for (unsigned i = 0; i < iMemSize; i++) {
    iMemPtr[i] = 0;
  } 
}

// Dump the imemory starting at the given address
// and dumping the given number of words
static void iMemoryDump(FILE *infile) {
  unsigned address; // The address to start at
  unsigned count; // The amount to print
  
  // Get the address
  fscanf(infile, "%x", &address);

  // Get the count
  fscanf(infile, "%x", &count);
  
  // Print the header
  printf("Addr");
  for (int i = 0; i < 8; i++) {
    printf("    %2X",i);
  }
  printf("\n"); 

  // Print the address of the start row
  printf("0x%02X",(address / 0x8) * 0x8);

  // Print the blank spaces before the first word to print
  for (unsigned i = 0; i < (address % 0x8); i++) {
    printf("      ");
  }

  // Print the memory contents 
  for (unsigned i = address; i < address+count; i++) {
    printf(" %05X",iMemPtr[i]);

    // Print a newline at mutliples of 0x8
    if(7 == i % 0x8) {

      // Unless the last word has been printed
      if(i != address+count-1) {     
        // Print a new line and the row's address label
        printf("\n0x%02X",(i+0x01));
      }
    }
  }
  // Put an empty newline after the dump
  printf("\n\n"); 
  
}

// Set the iMemory to the given values
// This method works with two different files:
//     The input file with system commands called infile
//     The cpu instruction file with cpu instructions called instrFile
static void iMemorySet(FILE *infile) {
  
  unsigned address; // The address to start at
  char instrFileName[NAME_MAX + PATH_MAX + 1]; // The path of the instruction file
  FILE *instrFile; // The file with the cpu instructions
  unsigned inWord; // The input word 
  
  // Get the address
  fscanf(infile, "%x", &address);  

  // Read "file" to skip over it
  fscanf(infile, "%10s", instrFileName);

  // Read the instruction file's path
  fscanf(infile, "%1000s", instrFileName);

  // Open the instruction file
  instrFile = fopen(instrFileName, "r"); 

   // Index for setting the word in memory
  unsigned i = address;
  
  // Read the whole file and set the values at the given address
  while (1 == fscanf(instrFile, "%x", &inWord)) {

    // Set the word into memory
    iMemPtr[i] = inWord; 

    i++; // Increment the index variable
  }

  // Close the file
  fclose(instrFile);
}

// Fetch an instruction and 
unsigned iMemFetch(unsigned address) {
  unsigned rVal = iMemPtr[address];
  return rVal;
}


// Free the imemory
void iMemClean() {
  free(iMemPtr);
}


// Read imemory commands from the file and call the functions
void parseIMemory(FILE *infile) {

  char cmd[11]; // The command to do

  // Get the command to execute
  fscanf(infile, "%10s", cmd);

  // Call the command's function

  // Calls the create function
  if (0 == strcmp(cmd, "create")) {
    iMemoryCreate(infile);
  }
  // Calls the reset function
  else if (0 == strcmp(cmd, "reset")) {
    iMemoryReset();
  }
  // Calls the dump function
  else if (0 == strcmp(cmd, "dump")) {
    iMemoryDump(infile);
  }
  // Calls the set function
  else if (0 == strcmp(cmd, "set")) {
    iMemorySet(infile);
  }
}