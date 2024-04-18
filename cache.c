#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "memory.h"

#define cacheSize 8
static bool isOn; // Flag that is true when the cache is on, false when off
static uint8_t clo; // Cache line offset
static uint8_t cacheData[cacheSize]; // The cache's main storage
static uint8_t fetchData[cacheSize]; // Temp array for holding data during memory fetch
static uint8_t *writeData; // Temp variable for the data to write to cache after a cache flush
enum cacheDataFlags { INVALID, VALID, WRITTEN };
static enum cacheDataFlags cacheFlags[cacheSize]; // Flag indicators of the state of each byte in the cache
static bool cacheDataWritten[cacheSize]; // Boolean list that indicates which bytes have been written
static unsigned cacheAddress;
static uint8_t* cacheAnswerPtr; 
static bool* cacheDonePtr; // Indicates when the cache is done with its task
static bool cacheFetchDone = false; // Indicates when the cache has completed a fetch from memory
static bool cacheStoreDone = false; // Indicates when the cache has completed the store to memory


// Reset the cache: cache to disabled, CLO to zero, data to be invalid
static void cacheReset() {
  isOn = false;
  clo = 0;

  for (int i = 0; i < cacheSize; i++) {
    cacheFlags[i] = INVALID;
    cacheDataWritten[i] = false;
  } 
}

// Turn the cache on
static void cacheOn() {
   isOn = true;
}

// Turn the cache off
static void cacheOff() {
   isOn = false;
}

// Dump the cache
static void cacheDump() {  
  // Print the CLO
  printf("clo        : 0x%02X\n", clo);

  // Print the cache data label
  printf("cache data :");

  // Print the cache data
  for (int i = 0; i < cacheSize; i++) {
    printf(" 0x%02X", cacheData[i]);
  }

  // Print the flag label
  printf("\nFlags      :");

  // Print the flag values
  for (int i = 0; i < cacheSize; i++) {
    
    // Translate the enumeration to the appropriate character to print
    char flag;
    if(cacheFlags[i] == 0)  flag = 'I';
    else if(cacheFlags[i] == 1)  flag = 'V';
    else if(cacheFlags[i] == 2)  flag = 'W'; 
    
    // Print the character
    printf("   %c ", flag);
  }

  // Print newlines
  printf("\n\n");
}

// Utility method to force invalid data
void forceInvalid() {
    // Clear data flags
    for(int i=0; i<cacheSize;i++) {
        cacheFlags[i] = INVALID;
        cacheDataWritten[i] = false;
    }  
}

// Utility method that handles writing a byte on a cache miss
void onCacheMissWriteByte(unsigned address, uint8_t *dataPtr,
                   bool *donePtr) {
    // Clear data flags
    for(int i=0; i<cacheSize;i++) {
        cacheFlags[i] = INVALID;
        cacheDataWritten[i] = false;
    }

    // Update the CLO
    clo = address>>3;
                
    // Write the byte to cache
    cacheData[address & 7] = *dataPtr; // Utilizes bitwise equivalent to "mod 8"
    cacheFlags[address & 7] = WRITTEN; // Mark the value as written
    cacheDataWritten[address & 7] = true; // Mark the value as written
    *donePtr = true; // Tell the CPU the copy is done  
}

// Alert cache of a tick
void cacheStartTick() {
    // check and see if memory is done with the fetch
    if(cacheFetchDone) {
        // Traverse the temp array and populate cacheData
        for(int i=0; i<cacheSize; i++){
            // If the data is not written, update it
            if(cacheFlags[i] != WRITTEN) {
                cacheData[i] = fetchData[i];
                cacheFlags[i] = VALID;
            }
        }

        // Finish the lw command
        *cacheAnswerPtr = cacheData[cacheAddress & 7]; // Utilizes bitwise equivalent to "mod 8"
        *cacheDonePtr = true; // Tell the CPU the copy is done
        cacheFetchDone = false;
    }

    // Check and see if memory is done with the store
    if(cacheStoreDone) {
        // Special Case: address = 0xFF, mark all written data as valid
        if(cacheAddress == 0xFF) {
            for(int i=0; i<cacheSize; i++){
                // Determine if the data was written
                if(cacheDataWritten[i]) {
                    cacheDataWritten[i] = false; // Mark it as false
                    cacheFlags[i] = VALID; // If so, mark it as valid now
                }
            }
            *cacheDonePtr = true; // Tell the CPU the copy is done
        }
        // In a normal cacheMiss case
        else {
        onCacheMissWriteByte(cacheAddress, writeData, cacheDonePtr);
        }

        cacheStoreDone = false; // Reset the cacheStoreDone variable
    }
  
}

// Check and see if the cache has more work to do in this cycle
bool cacheIsMoreCycleWorkNeeded() {
  return cacheFetchDone || cacheStoreDone;
}

// Determine if there is any valid data in the cache
static bool anyValid() {
    bool valid = false;
    
    // Traverse the flag array and find any VALID or WRITTEN
    for( int i = 0; i < cacheSize; i++) {
        if(cacheFlags[i] == VALID || cacheFlags[i] == WRITTEN) {
            valid = true;
        }
    }
    return valid;
}

// Determine if there is any written data in the cache
static bool anyWritten() {
    bool written = false;
    
    // Traverse the flag array and find any WRITTEN
    for( int i = 0; i < cacheSize; i++) {
        if(cacheFlags[i] == WRITTEN) {
            written = true;
        }
    }
    return written;
}



// Start a cache fetch at the given address
// address – the offset in memory where the read should begin
// dataPtr – a pointer where data should be placed
// memDonePtr – a pointer to a boolean that the Cache Device will set to true when
// the data transfer has completed (possibly multiple cycles after request)
void cacheStartFetch(unsigned address, uint8_t *dataPtr,
                   bool *donePtr) {
    // If the cache is off, fetch the single byte
    if(!isOn) {
        memStartFetch(address, 1, dataPtr, donePtr);
    } else {
        // Special Case: if the address is 0xFF, force the data to be invalid
        if(address == 0xFF) {
            forceInvalid();
            *dataPtr = 0; // Return 0
            *donePtr = true; // Tell the CPU the copy is done
        } 
        // Otherwise perform a standard fetch
        else {

            // Calculate the cash line
            int cashLine = address>>3;
            
            // Determine if the byte is in cache aka "cache hit"
            // (if the clo matches the computed offset of the address and there is valid data)
            if(clo == cashLine && anyValid()) {
                // Finish the lw command
                *dataPtr = cacheData[address & 7]; // Utilizes bitwise equivalent to "mod 8"
                *donePtr = true; // Tell the CPU the copy is done
            }
            else {
                // Store the arguments
                cacheAddress = address;
                cacheAnswerPtr = dataPtr;
                cacheDonePtr = donePtr;     

                // Get the address based on the cash line
                unsigned newAddress = cashLine<<3; 

                // initiate a memStartFetch for all 8 bytes starting at the new CLO
                // Put it in a temporary array
                memStartFetch(newAddress, cacheSize, fetchData, &cacheFetchDone);
            }
        }
    }
}

// Start a memory store at the given address 
// address – the offset in memory where the write should begin
// count – the number of bytes that should be written
// dataPtr – a pointer that is the source of data to write
// memDonePtr – a pointer to a boolean that the Memory Device will set to true when
void cacheStartStore(unsigned address, uint8_t *dataPtr,
                   bool *donePtr) {
    // If the cache is off, store the single byte
    if(isOn == false) {
        cacheDataWritten[address & 7] = true; 
        memStartStore(address, 1, dataPtr, cacheDataWritten, donePtr);
    } else { 
        // Special Case: if the address is 0xFF, perform a cache flush
        if(address == 0xFF) {
            // Determine if any data needs to be written to memory
            if(anyWritten()) {
                // Store the arguments
                cacheAddress = address;
                cacheDonePtr = donePtr; 

                // Flush the cache to memory
                memStartStore(clo<<3, cacheSize, cacheData, cacheDataWritten, &cacheStoreDone);
            }
        } 
        // Otherwise perform a standard store
        else {
            // Calculate the cash line
            int cashLine = address>>3;
            
            // Determine if the address is in cache aka "cache hit"
            // (if the clo matches the computed offset of the address)
            if(clo == cashLine) {
                // Write the value to cache
                cacheData[address & 7] = *dataPtr; // Utilizes bitwise equivalent to "mod 8"
                cacheFlags[address & 7] = WRITTEN; // Mark the value as written 
                cacheDataWritten[address & 7] = true; // Mark the value as written
                *donePtr = true; // Tell the CPU the copy is done
            }

            // On cache miss
            else {

                // Determine if any data in cache as been written to
                if(anyWritten()) {
                    // Store the arguments
                    cacheAddress = address;
                    writeData = dataPtr;
                    cacheDonePtr = donePtr; 

                    // Flush the cache to memory
                    memStartStore(clo<<3, cacheSize, cacheData, cacheDataWritten, &cacheStoreDone);
                } else {
                    onCacheMissWriteByte(address, dataPtr, donePtr); 
                }      
            }
        }
    }               
}


// Read cache commands from the file and call the functions
void parseCache(FILE *infile) {

  char cmd[11]; // Holds the command

  // Get the command to execute
  fscanf(infile, "%10s", cmd);

  // Call the command's function
  // Calls the reset function
  if (0 == strcmp(cmd, "reset")) {
    cacheReset();
  }
  // Calls the on function
  else if (0 == strcmp(cmd, "on")) {
    cacheOn(infile);

  }
  // Calls the off function
  else if (0 == strcmp(cmd, "off")) {
    cacheOff();
  }
  // Calls the dump function
  else if (0 == strcmp(cmd, "dump")) {
    cacheDump();
  }
}