/**
 *
 **/

#include <stdio.h>
#include <stdlib.h>

#define GetSP(sp) __asm__("movl %%esp,%0": "=r" (sp) : ) 
#define SetSP(sp) __asm__("movl %0,%%esp": : "r" (sp) ) 
#define STACKSIZE 16384 

unsigned long MyStack[STACKSIZE]; 

int realMain(int argc, char *argv[]){ 
	printf("Hello, world, on my own stack!\n"); 
	return 0; 
} 

int main(int argc, char *argv[]){ 
	void *oldSp; 
	GetSP(oldSp); 
	SetSP(MyStack + STACKSIZE); /* stacks are upside–down */ 
	realMain(argc,argv); 
	SetSP(oldSp); 
	return 0; 
} 
