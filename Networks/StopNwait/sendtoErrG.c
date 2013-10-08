/* This function flips multiple bits in a byte and also drops packets. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "sendtoErrG.h"

static int		start_flag_ = 0;		// record if init function has been called
static int		debug_ = 0;
static int		drop_ = 0;
static int		flip_ = 0;
static int		random_ = 0;
static double	error_rate_ = 0.0;

static int drop_and_flips(unsigned char * msg, int len, unsigned int packet_num);
static int flip_a_byte(unsigned char * msg, int len, unsigned int packet_num);
static int create_error(unsigned char *msg, int len);
static void output_debug();

/**
 * This function sends to the specified address with a specified error rate. Before calling this function,
 * sendtoErrG_init() must be called to set error rates and flags.
 **/
int sendtoErrG(int sock_num, void *msg, int msg_len,  unsigned int sendto_flags, const struct sockaddr *to, int to_len) {
    unsigned char * new_msg = NULL;
    int send_len = 0;
    int error = 0;
        
    if (start_flag_ == 0) {
		perror("Must call sendtoErrG_init() before using sendtoErrG()\n");
		exit(-1);
	}
    
    /* NOTE: if your program segfaults on the following malloc()... the problem is in
      your code and not the sendtoErrG code!!! */

    new_msg = (unsigned char *) malloc(msg_len);
    if (new_msg < 0) {
		printf("malloc() failed in sendtoErrG(..) check your code, len: %d\n", msg_len);
		perror("sendtoErrG malloc");
		exit(-1);
	}

    memcpy(new_msg, msg, msg_len);
        
    error = create_error(new_msg, msg_len);

    /* if error == 1 then drop the packet else send it */
    if (error == 1) {
		free(new_msg);
		return (msg_len);
    }
    
    send_len = sendto(sock_num,  new_msg, msg_len, sendto_flags, to,  to_len);
    free(new_msg);
    return (send_len);
}

/**
 * This function sets the options for sending (sendto) with errors.
 **/
int sendtoErrG_init(double error_rate, int drop_flag, int flip_flag, int debug_flag, int random_flag) {
	if (start_flag_ != 0) {
		printf("sendtoErrG - sendtoErrG_init(): Cannot call sendtoErrG_init() more than once per run!\n");
		exit(-1);
	}

	if (random_flag == RSEED_ON) {
		srand48(time(NULL));
    } else {
		srand48(9);
    }
  
	start_flag_ = 1;
	error_rate_ = error_rate;
	drop_ = (drop_flag) ? 1 : 0;
	flip_ = (flip_flag) ? 1 : 0;
	debug_ = (debug_flag) ? 1 : 0;
	random_ = (random_flag) ? 1 : 0;
    
	if (error_rate < 0 || error_rate >= 1) {
		printf("sendtoErrG - sendtoErrG_init(): The error rate must be between 0 and less than 1. Your value: ");
		printf("%f \n", error_rate);
		return (-1);
	}

	if (error_rate_ > 0 && !drop_flag && !flip_flag) {
		printf("sendtoErrG - sendtoErrG_init(): Error rate is greater than zero: %f but not dropping or flipping\n", error_rate_);
		printf("Therefore, your error rate will be ignored\n");
		return(-1);
	}

	if (debug_) {
		output_debug();
	}
    
  return(0);
    
}

static int create_error(unsigned char *msg, int len) {
	double check_error = 0.0;
	unsigned int temp = 0;
	unsigned int packet_num = 0;
    
	if (len > 3) {
		memcpy(&temp, msg, 4);
		packet_num = ntohl(temp);
    } else {
		packet_num = 0;
	}

	/* now see if we should do anything at all */
	check_error = drand48();
        
	if (error_rate_ < check_error) {
		if (debug_) {
			printf("Packet %d: sent without errors\n", packet_num);
		}

		return(0);
	} else {
		/* if we are here then we should be doing some type of error  */

		if (drop_ && flip_) {
			return(drop_and_flips(msg, len, packet_num));
		}
      
      if (drop_) {
		  if (debug_) {
			  printf("Packet %d: Dropped (did not send)\n", packet_num);
		  }

		  return(1);
	  }
      
	  if (flip_) {
		  return(flip_a_byte(msg, len, packet_num));
	  }
      
	}
    
	printf("hmm should not have gotten here!!!\n");
	return 0;
}

static void output_debug() {
	printf("sendToErrG - output_debug(): error_rate_ = %f\n", error_rate_);
	printf("sendToErrG - output_debug(): drop_ = %i\n", drop_);
	printf("sendToErrG - output_debug(): flip_ = %i\n", flip_);
	printf("sendToErrG - output_debug(): debug_ = %i\n", debug_);
	printf("sendToErrG - output_debug(): random_ = %i\n", random_);
}


static int drop_and_flips(unsigned char * msg, int len, unsigned int packet_num)
{
  /* give equal change to drop/flip byte/flip bits  */

  int action = (int) (2.0 * drand48());

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

    byte_to_flip = (int) (d_len * drand48());
    msg[byte_to_flip] = msg[byte_to_flip] ^ 255;

    if (debug_)
    {
      printf("Packet %d: Flipped all bits in byte %d \n", packet_num, byte_to_flip);
    }

    /*return 0 says to send packet */
    return(0);
}





