#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "clock.h"
#include "memory.h"
#include "imemory.h"
#include "cache.h"
#include "iodev.h"


int main(int argc, char *argv[]) {
  
  char device[11]; // The device being called (cpu, clock, or memory)
  static FILE *infile;
  

  // Open the file
  infile = fopen(argv[1], "r");
  
  // Read the whole file
  while (1 == fscanf(infile, "%10s", device)) {
    
    // Handles the clock
    if (0 == strcmp(device, "clock")) {
      parseClock(infile);
    }
    // Handles the memory
    else if (0 == strcmp(device, "memory")) {
      parseMemory(infile);
    }
    // Handles the cpu
    else if (0 == strcmp(device, "cpu")) {
      parseCpu(infile);
    }

    // Handles the imemory
    else if (0 == strcmp( device, "imemory")) {
      parseIMemory(infile);
    }

    // Handles the cache
    else if (0 == strcmp( device, "cache")) {
      parseCache(infile);
    }

    // Handles the iodevice
    else if (0 == strcmp( device, "iodev")) {
      parseIODevice(infile);
    }

  }

  // Close the file
  fclose(infile);

  // Free the memory and imemory
  memClean();
  iMemClean();

   return 0;
}