/**
 * This file contains macros for printing debug statements for the set debug
 * level. Set _DEBUG_LEVEL to the desired level for regular debug output and 
 * _DEBUG_ERRORS to  0 or 1 to turn error debug printing off or on.
 */

#include <stdio.h>

/* Debug levels. */
#define DEBUG_PRINT     0
#define DEBUG_MIN       1
#define DEBUG_MEDIUM    2
#define DEBUG_MAX       3

/* Set debug level and error printing */
#define _DEBUG_LEVEL    DEBUG_MAX
#define _DEBUG_ERRORS   1

/* Print no matter what. */
#if _DEBUG_LEVEL >= DEBUG_PRINT
    #define debug_print printf
# else
    #define debut_print
#endif

/* Print some minimal amount of information. */
#if _DEBUG_LEVEL >= DEBUG_MIN
    #define debug_min printf
# else
    #define debug_min
#endif

/* Print most things. */
#if _DEBUG_LEVEL >= DEBUG_MEDIUM
    #define debug_medium printf
#else
    #define debug_medium
#endif

/* Print everything. */
#if _DEBUG_LEVEL >= DEBUG_MAX
    #define debug_max printf
# else
    #define debug_max
#endif

/* Print errors. */
#ifdef _DEBUG_ERRORS
    #define debug_errors printf
#else
    #define debug_errors
#endif