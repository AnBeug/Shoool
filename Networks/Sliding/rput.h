/**
 * 
 */

void            send_handshake_packet(int sock_num, struct sockaddr_in * p_srv_addr);
void            send_init_packet(int sock_num, struct sockaddr_in * srv_addr, char * filename, u_int file_len, u_int buffer_sz, u_int window_sz);
void            send_file(int sock_num, struct sockaddr_in * p_srv_addr, FILE * file_ptr, u_int file_len, u_int buff_sz, u_int window_sz);
void            send_file_packet(int sock_num, struct sockaddr_in * srv_addr, data_pkt_t * p_data_pkt);
data_pkt_t *    build_packet(u_int seq_num, FILE *p_file, u_int buff_sz);
int             recv_ack(int sock_num, struct sockaddr_in * srv_addr, ack_pkt_t ** p_p_ack_pkt);
int             check_file(char * filename, FILE ** file_ptr);
char *          marshall_init_pkt(init_pkt_t * p_init_pkt);
char *          marshall_data_pkt(data_pkt_t * p_data_pkt);
ack_pkt_t *     unmarshall_ack_pkt(char * data_buff);