#include "smashme.h"

char MyStack[STACK_SZ];
int char_idx;

int main(int argc, char * const argv[]) 
{
  // Save original stack ptr.
  long st_ptr;
  GetSP(st_ptr);

  // Move the stack ptr to where we want it.
  SetSP(MyStack + STACK_SZ);

  // Get address of MyStack. The address where we start
  // unprotecting is the nearest lowest page boundary address.
  // If MyStack spans a page boundary, protect two pages.
  long start = (long)MyStack;
  start = (start / PAGESIZE) * PAGESIZE;
  size_t amount = ((long)(MyStack + STACK_SZ)) - start;
  if(mprotect((void*)start, amount, PROT_READ|PROT_WRITE|PROT_EXEC) < 0) {
  	perror("mprotect");
  	exit(1);
  }

  // This is definitely not malicious code. Nope, nothing to see here.
  // Carry on.
  get_input();

  // Set the stack ptr back to the original location.
  SetSP(st_ptr);

  return 0;
}

/**
 * OK. This really is bad code. It provides an exploitable
 * vulnerability because it does not check the amount of text
 * being read in, only that it has not reached EOF.
 */
void get_input()
{ 
 char stack[BUFF_SZ];
 
 while (1)
   {
     int cur_char = getchar();
     
     // Lois Brady would definitely not approve.
     if (cur_char != EOF)
       {
	 stack[char_idx++] = cur_char; 
       }
     else
       {
	 break;
       }
   }
 printf("read %d bytes\n", char_idx);
}

/**
 * This is the function we want to execute (for 90%) with
 * explicitly calling it from this program.
 */
void Uncalled()
{ 
  printf("Uncalled was called. Strange that.\n"); 
  exit(-1); 
}
