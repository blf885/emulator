This project is a simple computer emulator. The instructions
that can be run include ADD, ADDI MUL, INV, LOAD, STORE, HALTINSTR, 
BRANCH (BEQ, BNEQ, BLT). The system includes a 8 byte cache, 
data memory, intruction memory, clock, and a parser. 

Use "make" or "make all" to compile.

To run the code, type "./emul test.txt" in the command line. 
"test.txt" Contains some initialization instructions
for the emulator and by default runs "Sample2_Instructions.txt". 
Changing the 2 to 1 allows for a second
set of instructions to be executed. 

System requirements: Ubuntu Linux 22.04 LTS or similar.

Entropy's Instruction Set (Credit – James Walker)
Entropy Basics:
• 8-bit words
• 8 data registers (each register is 8 bits)
• 1 Program Counter register (8 bits)
• 8 bits for addressing its data memory
• 8 bits for addressing its instruction memory
• 8 instructions (2 implemented for Assignment 2)


Entropy's instructions are 20 bits each, and are divided up as follows: 
NNN DDD SSS TTT IIIIIIII
Where: 
• NNN is the three bit instruction encoding
• DDD is the three bit destination register selector
• SSS is the three bit source register selector
• TTT is the three bit target register selector
• IIIIIIII specifies an immediate value
