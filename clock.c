#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "cpu.h"
#include "memory.h"
#include "cache.h"
#include "iodev.h"

static uint16_t totalTicks; // Total clock ticks performed

// Reset the clock to zero
static void clockReset() { totalTicks = 0; }

// Get the number of ticks and perform clock ticks
static void clockTick(FILE *infile) {

  int ticks; // The number of ticks

  // Get the number of ticks
  fscanf(infile, "%d", &ticks);

  // Perform each tick
  for (int i = 0; i < ticks; i++) {
    bool workToDo = true; // Flag that indicates work is still being done
                          //    by device during this tick

    // Tell devices a new tick is starting
    cpuStartTick();
    memStartTick();
    cacheStartTick();
    iodevStartTick();
    
    

    // Loop while any device still has work to do this
    while (workToDo) {
      // Give devices a chance to do work
      cpuDoCycleWork();
      memDoCycleWork();
      cacheStartTick();
      
      // See if devices have more work to do this cycle
      workToDo = cpuIsMoreCycleWorkNeeded() || memIsMoreCycleWorkNeeded() || cacheIsMoreCycleWorkNeeded();
   }

    totalTicks++;
  }
}

// Display the total ticks
static void clockDump() { printf("Clock: %d\n\n", totalTicks); }

// Read clock commands from the file and call the functions
void parseClock(FILE *infile) {

  char cmd[11]; // Holds the command

  // Get the command to execute
  fscanf(infile, "%10s", cmd);

  // Call the command's function
  // Calls the reset function
  if (0 == strcmp(cmd, "reset")) {
    clockReset();
  }
  // Calls the tick function
  else if (0 == strcmp(cmd, "tick")) {
    clockTick(infile);

  }
  // Calls the dump function
  else if (0 == strcmp(cmd, "dump")) {
    clockDump();
  }
}