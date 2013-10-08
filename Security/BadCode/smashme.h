#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>

// For PAGESIZE needed for mprotect
#include <limits.h>    /* for PAGESIZE */
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

// The vulnerable code
void get_input();

// Stack manipulation
#define GetSP(sp) asm("movq %%rsp,%0": "=r" (sp) : )
#define SetSP(sp) asm("movq %0,%%rsp":  : "r" (sp) )


#define STACK_SZ 4096 
#define BUFF_SZ 58 // should round to even multiple of 8 

