#ifndef CPU_H
#define CPU_H
#include <stdio.h>
#include <stdbool.h>

int parseCpu(FILE *infile);
void cpuStartTick();
bool cpuIsMoreCycleWorkNeeded();
void cpuDoCycleWork();

#endif