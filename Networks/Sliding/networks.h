/*********************************************
 * File         : networks.h                 *
 * Author       : Annie Beug                 *
 * Date			: 28-FEB-2010                *
 * Modified     : 07-MAR-2010                *
 * Project      : Assignment 4               *
 * Class        : CPE 464                    *
 * Professor    : Dr. Smith                  *
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
#include <sys/types.h>

#include "checksum.h"
#include "sendtoErrG.h"

/* Constants */
#define HS_PKT_SZ			sizeof(handshake_pkt_t)
#define INIT_HDR_SZ			sizeof(init_hdr_t)
#define INIT_PKT_SZ			sizeof(init_pkt_t)
#define MAX_INIT_PKT_SZ		INIT_HDR_SZ + 1000		// add another 1000 for the filename
#define ACK_PKT_SZ			sizeof(ack_pkt_t)
#define DATA_HDR_SZ			sizeof(data_hdr_t)
#define DATA_PKT_SZ			sizeof(data_pkt_t)
#define MAX_DATA_PKT_SZ		DATA_HDR_SZ + 1400

#define MIN_FILENAME_SZ		1
#define MAX_FILENAME_SZ		1000
#define MIN_BUFFER_SZ		1
#define MAX_BUFFER_SZ		1400

#define CLI_WAIT_SECS		1
#define MAX_CLI_RETRIES		10
#define SRV_WAIT_SECS		11

#define HS_SEQ_NUM			0
#define INIT_SEQ_NUM		1
#define FIRST_DATA_SEQ_NUM	2

#define RES_NO_DATA			-1
#define RES_DATA_CORRUPT	-2
#define RES_FILE_ERR		-3
#define RES_SUCCESS			1

#define HS_PKT_TYPE			0
#define INIT_PKT_TYPE		1
#define DATA_PKT_TYPE		2
#define ACK_PKT_TYPE		3
#define NAK_PKT_TYPE		4
#define FILE_ERR_PKT_TYPE	5

/* Structures */
typedef struct  __attribute__ ((__packed__)) {
    u_int		seq_num;		// 4-byte sequence number
	u_char		pkt_type;		// 1-byte packet type descriptor
	u_short		chk_sum;		// 2-byte check sum
} handshake_pkt_t;

typedef struct __attribute__ ((__packed__)) {
	u_int		seq_num;		// 4-byte sequence number
	u_short		chk_sum;		// 2-byte check sum
	u_char		pkt_type;		// 1-byte packet type descriptor
	u_int		filename_len;	// 4-byte length of the filename
	u_int		buffer_sz;		// 4-byte length of the filename
	u_int		window_sz;		// 4-byte window size
	u_int		file_len;		// 4-byte length of the file
} init_hdr_t;

typedef struct __attribute__ ((__packed__)) {
	init_hdr_t 	hdr;		// packet header
	char *		filename;	// data for header name
} init_pkt_t;

typedef struct __attribute__ ((__packed__)) {
	u_int		seq_num;		// 4-byte sequence number
	u_short		chk_sum;		// 2-byte check sum
	u_char		pkt_type;		// 1-byte packet type descriptor
	u_int		buffer_sz;		// 4-byte sequence number
} data_hdr_t;

typedef struct __attribute__ ((__packed__)) {
	data_hdr_t	hdr;
	char *		data;
} data_pkt_t;
	
typedef struct __attribute__ ((__packed__)) {
	u_int		seq_num;		// 4-byte sequence number
	u_short		chk_sum;		// 2-byte check sum
	u_char		pkt_type;		// 1-byte packet type descriptor
} ack_pkt_t;

/* Client functions */
void			send_handshake_packet(int sock_num, struct sockaddr_in * p_srv_addr);
void			send_init_packet(int sock_num, struct sockaddr_in * srv_addr, char * filename, u_int file_len, u_int buffer_sz, u_int window_sz);
void			send_file(int sock_num, struct sockaddr_in * p_srv_addr, FILE * file_ptr, u_int file_len, u_int buff_sz, u_int window_sz);
void			send_file_packet(int sock_num, struct sockaddr_in * srv_addr, data_pkt_t * p_data_pkt);
data_pkt_t *	build_packet(u_int seq_num, FILE *p_file, u_int buff_sz);
int				recv_ack(int sock_num, struct sockaddr_in * srv_addr, ack_pkt_t ** p_p_ack_pkt);
int				check_file(char * filename, FILE ** file_ptr);
char *			marshall_init_pkt(init_pkt_t * p_init_pkt);
char *			marshall_data_pkt(data_pkt_t * p_data_pkt);
ack_pkt_t *		unmarshall_ack_pkt(char * data_buff);

/* Server functions */
int				create_udp_srv_sock(int port);
void			connect_handshake(int sock_num);
void *			do_threading( void * ptr );
int				recv_init(int sock_num, struct sockaddr_in * p_cli_addr, init_pkt_t ** p_p_init_pkt, FILE ** p_p_file);
void			send_ack(int sock_num, struct sockaddr_in * p_cli_addr, u_int seq_num, u_char ack_type);
void			recv_file(int sock_num, struct sockaddr_in * p_cli_addr, u_int file_len, u_int buffer_sz, FILE * p_file);
init_pkt_t *	unmarshall_init_pkt(char * data_buff);
char *			marshall_ack_pkt(ack_pkt_t * p_ack_pkt);
data_pkt_t *	unmarshall_data_packet(char * data_buff);

/* Shared functions */
int				create_udp_sock();
int				select_call(int socket, int seconds, int useconds);
int				send_packet(int sock_num, char * data_buff, u_int data_len, struct sockaddr_in * sock_addr);
int				recv_packet(int sock_num, char * data_buff, u_int data_len, struct sockaddr_in * sock_addr);
u_int			min(u_int a, u_int b);
u_int			max(u_int a, u_int b);
void			print_sockaddr_info(struct sockaddr_in * p_sockaddr);
void			print_bytes(char * data_buff, int data_len);
void			print_data_pkt(char * caller, data_pkt_t * p_data_pkt);
void			print_init_pkt(char * caller, init_pkt_t * p_init_pkt);
void			print_ack_pkt(char * caller, ack_pkt_t * p_ack_pkt);