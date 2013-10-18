/**
 * This file helper functions for network communications.
 * @auther asbeug
 **/

#include "networks.h"

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

    /* Setup and bind to socket */
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
    printf("create_udp_srv_sock(): port = %d \n", ntohs(local_addr.sin_port));
    
    return sock_num;
}

/**
 * Sends a packet of data to the specified address.
 * Returns the number of bytes transfered.
 **/
int send_packet(int sock_num, char * send_buff, unsigned int send_len, struct sockaddr_in * sock_addr) {
    unsigned int sockaddr_len = sizeof(struct sockaddr);
    
    //print_sockaddr_info(*sock_addr);
    //printf("socket_utils - send_packet() 1: send_len = %u\n", send_len);  
    //int sendtoErrG(int sock_num, void *msg, int msg_len,  unsigned int sendto_flags, const struct sockaddr *to, int to_len) {
    int bytes_trfd = sendtoErrG(sock_num, (char *) send_buff, send_len, 0, (const struct sockaddr *) sock_addr,  sockaddr_len);
    
    if (bytes_trfd < 0) {
        printf("socket_utils - send_packet() err: %s\n", strerror(errno));
    }
    
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
    
    return result;
}

/**
 * Print out the socket address information.
 **/
void print_sockaddr_info(struct sockaddr_in sock_addr) {
    printf("\tsin_addr = '%s'; sin_port = %d", inet_ntoa(sock_addr.sin_addr), ntohs(sock_addr.sin_port));
}
