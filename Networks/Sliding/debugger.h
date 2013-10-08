/*********************************************
 * File         : debugger.h                 *
 * Author       : Annie Beug                 *
 * Date			: 28-FEB-2010                *
 * Modified     : 08-MAR-2010                *
 * Project      : Assignment 4               *
 * Class        : CPE 464                    *
 * Professor    : Dr. Smith                  *
 *********************************************/

#include <stdio.h>

#define	DEBUG_PRINT		0
#define DEBUG_MIN		1
#define DEBUG_MEDIUM	2
#define DEBUG_MAX		3

#define _DEBUG_LEVEL	DEBUG_MAX
#define _DEBUG_ERRORS	1

// Print no matter what
#if _DEBUG_LEVEL >= DEBUG_PRINT
	#define debug_print printf
# else
	#define debut_print
#endif
// Print minimal amount
#if _DEBUG_LEVEL >= DEBUG_MIN
	#define debug_min printf
# else
	#define debug_min
#endif
// Print most things
#if _DEBUG_LEVEL >= DEBUG_MEDIUM
	#define debug_medium printf
#else
	#define debug_medium
#endif
// Print everything
#if _DEBUG_LEVEL >= DEBUG_MAX
	#define debug_max printf
# else
	#define debug_max
#endif

#ifdef _DEBUG_ERRORS
	#define debug_errors printf
#else
	#define debug_errors
#endif