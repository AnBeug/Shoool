/**
 * This file contains function prototypes for rput.
 * @author asbeug
 **/

int check_file(char * filename, FILE ** file_ptr);
int send_handshake_packet(int sock_num, struct sockaddr_in * srv_addr);
int send_init_packet(int sock_num, struct sockaddr_in * srv_addr, char * filename, int file_len);
void send_file(int sock_num, struct sockaddr_in * srv_addr, FILE * file_ptr, unsigned int file_len, unsigned int buff_sz);
void send_file_packet(int sock_num, unsigned int seq_num, struct sockaddr_in * srv_addr, char * send_buff, unsigned int buff_sz);
int recv_ack(int sock_num, struct sockaddr_in * srv_addr);
