/***************************************
 * File           : spinny.c           *
 * Authors        : Annie Beug         *
 *                : Evan Hecht         *
 * Date           : 23-MAY-2010        *
 * Modified       : 31-MAY-2010        *
 * Class          : CPE 456            *
 * Professor      : Dr. Nico           *
 ***************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	int x = 5;
	asm("movq $4, %rax");
		asm("movq $1, %rbx");
	asm("movq $236, 0x600ba0");
	asm("mov $0x600ba0, %rcx");
	asm("movq $1, %rdx");
	asm("int $0x80");
	int y = 6;
}

