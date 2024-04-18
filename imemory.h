#ifndef IMEMORY_H
#define IMEMORY_H
#include <stdio.h> 

int parseIMemory(FILE *infile); 
unsigned iMemFetch();
void iMemClean();

#endif