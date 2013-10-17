/**
 *
 */

int create_udp_srv_sock(int port);
void connect_handshake(int sock_num);
int send_ack(int sock_num, struct sockaddr_in * cli_addr, unsigned int seq_num);
void * do_threading( void * ptr );