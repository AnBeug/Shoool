/**
 * 
 */

int             create_udp_srv_sock(int port);
void            connect_handshake(int sock_num);
void *          do_threading( void * ptr );
int             recv_init(int sock_num, struct sockaddr_in * p_cli_addr, init_pkt_t ** p_p_init_pkt, FILE ** p_p_file);
void            send_ack(int sock_num, struct sockaddr_in * p_cli_addr, u_int seq_num, u_char ack_type);
void            recv_file(int sock_num, struct sockaddr_in * p_cli_addr, u_int file_len, u_int buffer_sz, FILE * p_file);
init_pkt_t *    unmarshall_init_pkt(char * data_buff);
char *          marshall_ack_pkt(ack_pkt_t * p_ack_pkt);
data_pkt_t *    unmarshall_data_packet(char * data_buff);