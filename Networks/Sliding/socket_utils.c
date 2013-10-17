/*********************************************
 * File         : socket_utils.c             *
 * Author       : Annie Beug                 *
 * Date         : 28-FEB-2010                *
 * Modified     : 08-MAR-2010                *
 *********************************************/

#include "networks.h"
#include "debugger.h"

/**
 * Creates a UDP socket.
 * Returns socket descriptor number.
 **/
int create_udp_sock() {
    int sock_num = 0;
    
    /* Open the socket */
    sock_num = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_num < 0) {
        return -1;
    }

    ////debug_max("create_udp_sock(): sock_num = %i\n", sock_num);
    
    return sock_num;
}

/**
 * Creates and binds to a UDP socket.
 * Prints port number bound to.
 * Returns socket descriptor number.
 **/
int create_udp_srv_sock(int port) {
    int sock_num = 0;
    struct sockaddr_in local_addr;
    socklen_t sockaddr_len = sizeof(local_addr);

    if ((sock_num = create_udp_sock()) < 0) {
        return sock_num;
    }
    
    // Bind
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(port);
    
    // bind the name/address to a 
    if (bind(sock_num, (struct sockaddr *) &local_addr, sizeof(local_addr)) < 0) {
        perror("bind call");
        return -1;
    }
    
    // get the socket name
    if (getsockname(sock_num, (struct sockaddr*) &local_addr, &sockaddr_len) < 0)
    {
        perror("getsockname call");
        return -1;
    }
    debug_print("create_udp_srv_sock(): port = %d \n", ntohs(local_addr.sin_port));
    
    return sock_num;
}

/**
 * Receive a single byte in to the data buffer.
 * Returns the number of bytes received.
 **/
int recv_packet(int sock_num, char * data_buff, u_int data_len, struct sockaddr_in * sock_addr) {
    u_int sock_addr_len = sizeof(struct sockaddr);
    int bytes_trfd =  recvfrom(sock_num, data_buff, data_len, 0, (struct sockaddr *) sock_addr, &sock_addr_len);
    
    if (bytes_trfd < 0) {
        debug_errors("send_packet() err: %s\n", strerror(errno));
    }
    
    return bytes_trfd;
}

/**
 * Sends a packet of data to the specified address.
 * Returns the number of bytes transfered.
 **/
int send_packet(int sock_num, char * data_buff, unsigned int send_len, struct sockaddr_in * sock_addr) {
    unsigned int sockaddr_len = sizeof(struct sockaddr);
    
    //print_sockaddr_info(*sock_addr);
    ////debug_max("send_packet(): send_len = %u\n", send_len);  
    //int sendtoErrG(int sock_num, void *msg, int msg_len,  unsigned int sendto_flags, const struct sockaddr *to, int to_len) {
    int bytes_trfd = sendtoErrG(sock_num, (char *) data_buff, send_len, 0, (const struct sockaddr *) sock_addr,  sockaddr_len);
    
    if (bytes_trfd < 0) {
        debug_errors("send_packet() err: %s\n", strerror(errno));
    }
    
    ////debug_max("send_packet(): bytes_trfd = %i\n", bytes_trfd);
    
    return bytes_trfd;
}

/**
 * Handles select() call. Waits at most the specified amount of time
 * to receive data on the given socket. 
 * Returns 1 if there is data to be received on the socket; 0 otherwise.
 **/
int select_call(int sock_num, int seconds, int useconds) {
    int result = 0;
    struct timeval timeout;
    fd_set fdset;
    
    timeout.tv_sec = seconds;
    timeout.tv_usec = useconds;
    
    FD_ZERO(&fdset); // reset variables 
    FD_SET(sock_num, &fdset);
    
    select(sock_num + 1, (fd_set *) &fdset, (fd_set *) 0, (fd_set *) 0, &timeout); 
    
    if (FD_ISSET(sock_num, &fdset)) { 
        result = 1;
    }   
    
    ////debug_max("select_call(): result = %i\n", result);
    return result;
}

/**
 * Print out the socket address information.
 **/
void print_sockaddr_info(struct sockaddr_in * p_sock_addr) {
    debug_print("print_sockaddr_info(): sin_addr = '%s'; sin_port = %d\n", inet_ntoa(p_sock_addr->sin_addr), ntohs(p_sock_addr->sin_port));
}

/**
 * Print out the members of the init packet.
 **/
void print_init_pkt(char * caller, init_pkt_t * p_init_pkt) {
    //debug_max("%s(): seq_num = %i\n", caller, p_init_pkt->hdr.seq_num);
    //debug_max("%s(): chk_sum = %i\n", caller, p_init_pkt->hdr.chk_sum);
    //debug_max("%s(): pkt_type = %i\n", caller, p_init_pkt->hdr.pkt_type);
    //debug_max("%s(): filename_len = %i\n", caller, p_init_pkt->hdr.filename_len);
    //debug_max("%s(): buffer_sz = %i\n", caller, p_init_pkt->hdr.buffer_sz);
    //debug_max("%s(): window_sz = %i\n", caller, p_init_pkt->hdr.window_sz);         
    //debug_max("%s(): file_len = %i\n", caller, p_init_pkt->hdr.file_len);
    //debug_max("%s(): filename = '%s'\n", caller, p_init_pkt->filename);
}

/**
 * Print out the members of the data packet.
 **/
void print_data_pkt(char * caller, data_pkt_t * p_data_pkt) {
    //debug_max("%s(): seq_num = %i\n", caller, p_data_pkt->hdr.seq_num);
    //debug_max("%s(): chk_sum = %i\n", caller, p_data_pkt->hdr.chk_sum);
    //debug_max("%s(): pkt_type = %i\n", caller, p_data_pkt->hdr.pkt_type);
    //debug_max("%s(): buffer_sz = %i\n", caller, p_data_pkt->hdr.buffer_sz);
    //debug_max("%s(): chk_sum = %i\n", caller, p_data_pkt->hdr.chk_sum);
    //debug_max("%s(): data = '%s'\n", caller, p_data_pkt->data);
}

/**
 * Print out the members of the ack packet.
 **/
void print_ack_pkt(char * caller, ack_pkt_t * p_ack_pkt) {
    //debug_max("%s(): seq_num = %i\n", caller, p_ack_pkt->seq_num);
    //debug_max("%s(): chk_sum = %i\n", caller, p_ack_pkt->chk_sum);
    //debug_max("%s(): pkt_type = %i\n", caller, p_ack_pkt->pkt_type);
}

/**
 * Print out each byte in an array as an number.
 **/
void print_bytes(char * data_buff, int data_len) {
    int i = 0;
    for (;i<data_len;i++) {
        //debug_max("print_bytes(): %i = %i\n", i, data_buff[i]);
    }
}

/**
 * Return the minimal of two ints.
 **/
u_int min(u_int a, u_int b) {
    if (a < b) {
        return a;
    } else { // a >= b
        return b;
    }
}

/**
 * Return the maximal of two ints.
 **/
u_int max(u_int a, u_int b) {
    if (a > b) {
        return a;
    } else { // a <= b
        return b;
    }
}