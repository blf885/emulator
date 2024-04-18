#ifndef CACHE_H
#define CACHE_H 
#include <stdio.h>
#include <stdint.h>

 int parseCache(FILE *infile);
 void cacheStartFetch(unsigned address, uint8_t *dataPtr, bool *donePtr);
 void cacheStartStore(unsigned address, uint8_t *dataPtr, bool *donePtr);
 void cacheStartTick();
 bool cacheIsMoreCycleWorkNeeded();

#endif