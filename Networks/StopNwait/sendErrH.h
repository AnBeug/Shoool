
/* in your main()

To use the sendErrH(...) function you must call sendErrH_init(...) first.
sendErrH_init(...) initializes fields that are used by sendErrH(...).  You
should only call sendErrH_init(...) once in each program.  

sendErrH_init(...)
      error_rate - between 0 and less than 1 (0 means no errors)
      drop_flag - should packets be dropped (DROP_ON or DROP_OFF)
      flip_flag - should bits be flipped (FLIP_ON or FLIP_OFF)
      debug_flag - print out debug info (DEBUG_ON or DEBUG_OFF)
      random_flag - if set to RSEED_ON, then seed is randon, if RSEED_OFF then seed is not random
      ex:

      sendErr_init(.1, DROP_ON, FLIP_OFF, DEBUG_ON, RSEED_ON);

*/

#define DEBUG_ON 1
#define DEBUG_OFF 0
#define FLIP_ON 1
#define FLIP_OFF 0
#define DROP_ON 1
#define DROP_OFF 0
#define RSEED_ON 1
#define RSEED_OFF 0


#ifdef __cplusplus
extern "C" {
#endif

int sendErrH_init(double error_rate, int drop_flag, int flip_flag, int debug_flag, int random_flag);
int sendErrH(int s, void *msg, int len,  unsigned  int flags);
  
#ifdef __cplusplus
}
#endif
