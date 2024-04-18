#ifndef MEMORY_H
#define MEMORY_H
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

int parseMemory(FILE *infile); 
void memStartFetch(unsigned address, unsigned count, uint8_t *dataPtr, bool *donePtr);
void memStartStore(unsigned address, unsigned count, uint8_t *dataPtr, bool *validPtr, bool *donePtr);
void memStartTick();
bool memIsMoreCycleWorkNeeded();
void memDoCycleWork();
void memClean();

#endif