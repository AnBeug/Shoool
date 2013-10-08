/*********************************************
 * File			: networks.h                 *
 * Author		: Annie Beug                 *
 * Date			: 5-FEB-2010                 *
 * Modified		: 10-FEB-2010                *
 *********************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h> 
#include <math.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include <pthread.h>

#include "checksum.h"
#include "sendtoErrG.h"

/* Definitions */
#define MAX_ACK_PKT_SZ		4
#define MAX_DATA_PKT_SZ		1500
#define MAX_HS_PKT_SZ		4

#define CLI_WAIT_SECS		5
#define MAX_CLI_RETRIES		10
#define SRV_WAIT_SECS		11

#define HS_SEQ_NUM			0
#define INIT_SEQ_NUM		1
#define FIRST_DATA_SEQ_NUM	2

/* Client functions */
int check_file(char * filename, FILE ** file_ptr);
int send_handshake_packet(int sock_num, struct sockaddr_in * srv_addr);
int send_init_packet(int sock_num, struct sockaddr_in * srv_addr, char * filename, int file_len);
void send_file(int sock_num, struct sockaddr_in * srv_addr, FILE * file_ptr, unsigned int file_len, unsigned int buff_sz);
void send_file_packet(int sock_num, unsigned int seq_num, struct sockaddr_in * srv_addr, char * send_buff, unsigned int buff_sz);
int recv_ack(int sock_num, struct sockaddr_in * srv_addr);

/* Server functions */
int create_udp_srv_sock(int port);
void connect_handshake(int sock_num);
int send_ack(int sock_num, struct sockaddr_in * cli_addr, unsigned int seq_num);
void * do_threading( void * ptr );

/* Shared functions */
int create_udp_sock();
int select_call(int socket, int seconds, int useconds);
int send_packet(int sock_num, char * send_buff, unsigned int send_len, struct sockaddr_in * sock_addr);
void print_sockaddr_info(struct sockaddr_in sock_addr);


/************************************************
 * Handshake Packet
 *
 * 4 byte		seq num = 0
 ************************************************
 * ACK Packet
 *
 * 4 byte		seq num = 0
 ************************************************
 * Init Packet
 * 
 * 4 byte		seq num = 1
 * 4 byte		file name length
 * 4 byte		file length
 * ? byte		file name (file name length)
 * 2 byte		check sum
 ************************************************
 * File Data Packet
 *
 * 4 byte		seq num > 1
 * 2 byte		check sum
 * 4 byte		buffer size
 * ? byte		data ( buffer size )
 ************************************************/
