/* in your main()

To use the sendtoErrG(...) function you must call sendtoErrG_init(...) first.
sendtoErrG_init(...) initializes fields that are used by sendtoErrG(...).  You
should only call sendtoErrG_init(...) once in each program.  

sendtoErrG_init(error_rate, drop_flag, flip_flag, debug_flag, random_flag)
      error_rate - float between 0 and less than 1 (0 means no errors)
      drop_flag - should packets be dropped (DROP_ON or DROP_OFF)
      flip_flag - should bits be flipped (FLIP_ON or FLIP_OFF)
      debug_flag - print out debug info (DEBUG_ON or DEBUG_OFF)
      random_flag - causes srand48 to use a time based seed (RSEED_ON or RSEED_OFF)
                    (see man srand48 - RSEED_OFF makes your program
		    runs more repeatable)

      If you don't know what to set random_flag to, use RSEED_OFF for initial
      debugging.  Once your program works, try RSEED_ON.  This will make the
      drop/flip pattern random between program runs.
		    
      ex:

      sendtoErrG_init(.1, DROP_ON, FLIP_OFF, DEBUG_ON, RSEED_OFF);

*/

#define DROP_ON 1
#define DROP_OFF 0
#define FLIP_ON 1
#define FLIP_OFF 0
#define DEBUG_ON 1
#define DEBUG_OFF 0
#define RSEED_ON 1
#define RSEED_OFF 0 


#ifdef __cplusplus
extern "C" {
#endif
    
  int sendtoErrG(int sock_num, void *msg, int msg_len,  unsigned int sendto_flags, const struct sockaddr *to, int to_len);
  int sendtoErrG_init(double error_rate, int drop_flag, int flip_flag, int debug_flag, int random_flag);


#ifdef __cplusplus
}
#endif
