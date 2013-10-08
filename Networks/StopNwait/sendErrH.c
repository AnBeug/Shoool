/* This function flips multiple bits in a byte and also drops packets. */

/* SEE comments in sendErrH.h file for help on options     */


/*created H version -  modified to use RSEED in sendErrH_init
                       also changed to use thread safe drand48_r on linux
		       HMS - Oct, 2008  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "sendErrH.h"

static int start_flag_ = 0;
static int debug_ = 0;
static int drop_ = 0;
static int flip_ = 0;
static int random_ = 0;
static double error_rate_ = 0.0;

/* the _r version of drand48_r only works on linux */
/* otherwise use normal drand48 on solaris  */
#ifdef __linux__
static struct drand48_data drand48_buf;
#endif

static int create_error(unsigned char *msg, int len);
static int drop_and_flips(unsigned char * msg, int len, unsigned int packet_num);
static int flip_a_byte(unsigned char * msg, int len, unsigned int packet_num);

int sendErrH(int s, void *msg, int len,  unsigned  int flags)
{

    unsigned char * new_msg = NULL;
    int send_len = 0;

    if (start_flag_ == 0)
      {
	printf("Must call sendErrH_init(...) before using sendErrH(...) \n");
	printf("Plesae fix your code and try again.\n");
	exit(-1);
      }
    
    /*NOTE if your program segfaults on the following mallco()... the problem is in
      your code and not the sendErrH code!!!                                         */

    new_msg = (unsigned char *) malloc(len);
    if (new_msg < 0)
      {
	printf("malloc() failed in sendErrH(..) the problem is in your code not mine, it just shows up here, len: %d\n", len);
	perror("sendErrH malloc");
	exit(-1);
      }
    
    memcpy(new_msg, msg, len);
        
    int error = create_error(new_msg, len);
    
    /* if error == 1 then drop the packet else send it */
    if (error == 1)
    {
	free(new_msg);
	return (len);
    }
    
    send_len = send(s,  new_msg, len, flags);
    free(new_msg);
    return (send_len);
}

int sendErrH_init(double error_rate, int drop_flag, int flip_flag, int debug_flag, int random_flag)
{
  if (start_flag_ != 0)
    {
      printf("Cannot call sendErrH_init(...) more than once per run!\n");
      printf("Please fix your code and try again.\n");
      exit(-1);
    }


  if (random_flag == RSEED_ON)
    {
#ifdef __linux__      
      srand48_r(time(NULL), &drand48_buf);
#else
      srand48(time(NULL));
#endif
    } else
    {
#ifdef __linux__      
      srand48_r(10, &drand48_buf);
#else
      srand48(10);
#endif
    }
    
  start_flag_ = 1;
  error_rate_ = error_rate;
  drop_ = drop_flag;
  flip_ = flip_flag;
  debug_ = debug_flag;
  random_ = random_flag;
  
  if (error_rate < 0 || error_rate >= 1) 
    {
      printf("ERROR:  The error rate must be between 0 and less than 1. Your value: ");
      printf("%f \n", error_rate);
      return (-1);
    }

  
  if (error_rate_ > 0 && !drop_flag && !flip_flag)
    {
      printf("Error rate is greater than zero: %f but not dropping or flipping\n", error_rate_);
      printf("Therefore, your error rate will be ignored\n");
      return(-1);
    }

  printf("\nError rate: %f ", error_rate_);
  printf("Drop Packets: %s ", (drop_) ? "Yes" : "No");
  printf("Flip Bits: %s ", (flip_) ? "Yes" : "No");
  printf("Debug mode: %s\n ", (debug_) ? "Yes" : "No");
  printf("Using Random Seed: %s\n ", (random_) ? "Yes" : "No");

  
  return(0);
    
}


static int create_error(unsigned char *msg, int len)
{
    double check_error = 0.0;
    unsigned int temp = 0;
    unsigned int packet_num = 0;
    
    if (len > 3)
      {
	memcpy(&temp, msg, 4);
	packet_num = ntohl(temp);
      } else
	{
	  packet_num = 0;
        }
     
    /* now see if we should do anything at all */
#ifdef __linux__      
    drand48_r(&drand48_buf, &check_error);
#else
    check_error = drand48();
#endif        

    if (error_rate_ < check_error || (!drop_ && !flip_))
      {
	if (debug_)
	  printf("Packet %d: sent without errors\n", packet_num);
	
	return(0);
      } else
      {
	
	/* if we are here then we should be doing some type of error  */
	
	if (drop_ && flip_)
	  return(drop_and_flips(msg, len, packet_num));
	
	if (drop_)
	  {
	    if (debug_) printf("Packet %d: Dropped (did not send)\n", packet_num);
	    return(1);
	}
	
	if (flip_)
	  return(flip_a_byte(msg, len, packet_num));
	
	printf("hmm should not have gotten here!!!\n");
	return 0;
      }
}


static int drop_and_flips(unsigned char * msg, int len, unsigned int packet_num)
{
  /* give equal chance to drop/flip byte  */

  double rate = 0.0;
  int action = 0;

#ifdef __linux__      
  drand48_r(&drand48_buf, &rate);
#else
  rate = drand48();
#endif          
  
  action = (int) (2.0 * rate);
  
  switch (action)
    {
    case 0:
      /* drop the packet */
	if (debug_) printf("Packet %d: Dropped (did not send)\n", packet_num);
	return 1;	
      break;

    case 1:
      /* flip one byte */
      return (flip_a_byte(msg, len, packet_num));
      break;

    default:
      printf("What are you doing here.... error in my code... \n");
      return(-1);
      break;
    }
  
}

static int flip_a_byte(unsigned char * msg, int len, unsigned int packet_num)
{
    int byte_to_flip = 0;
    double d_len = len;
    double drand_value;
    
#ifdef __linux__      
    drand48_r(&drand48_buf, &drand_value);
#else
    drand_value = drand48();
#endif        
    
    byte_to_flip = (int) (d_len * drand_value);
    msg[byte_to_flip] = msg[byte_to_flip] ^ 255;

    if (debug_)
    {
      printf("Packet %d: Flipped all bits in byte %d \n", packet_num, byte_to_flip);
    }

    /*return 0 says to send packet */
    return(0);
}





