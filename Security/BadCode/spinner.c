/***************************************
 * File           : spinner.c          *
 * Author         : Annie Beug         *
 * Date           : 23-MAY-2010        *
 ***************************************/

#include <limits.h> 
#include <stdio.h>	
#include <stdlib.h> 
#include <sys/mman.h> 
#include <unistd.h> 

#ifndef PAGESIZE
#define PAGESIZE 4096 
#endif

typedef void (* voidfun)(void);
short spin = -277;

int main() 
{ 
	printf("did get here\n");
	long start; 
	voidfun f;
	start = (long) &spin;
	start -= start % PAGESIZE;

	if ( 0 > mprotect((void*)start, PAGESIZE, PROT_READ|PROT_WRITE|PROT_EXEC) ) { 
		perror("memprotect"); 
		exit(1); 
	} 
	f = (voidfun) &spin; 
	f(); 
	printf("I’d better not get here.\n");
	return 1; 
} 