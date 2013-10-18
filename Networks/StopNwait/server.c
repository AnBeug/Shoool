/**
 * This file contains the server.
 * The server starts up and opens a UDP socket for listening to for
 * incoming requests.
 * @author asbeug
 **/

#include "networks.h"
#include "server.h"

#define PORT_NUM 0

/**
 * Read in command line arguments and create a UDP socket to listen for
 * incoming requests.
 **/
int main(int argc, char * argv[]) {
    float               err_pct;
    int                 sock_num;
    
    /* Get command line args */
    if (argc != 2) {
        printf("Usage: server <error percent>\n");
        return -1;
    }
    
    err_pct = atof(argv[1]);
    
    printf("server - main() 1: Starting rput with paramters:\n");
    printf("\terr_pct = %f\n", err_pct);

    /* Establish connection */
    // Get socket
    if ((sock_num = create_udp_srv_sock(PORT_NUM)) < 0) {
        perror("creating socket");
    }
    
    /* Init error sending */
    // error rate, drop flag, flip flag, debug flag, random flag
    if (sendtoErrG_init(err_pct, 1, 1, 0, 1) < 0) {
        perror("calling sendtoErrG_init");
        exit(-1);
    }   
    
    while (1) {
        /* Receive handshake packet */
        connect_handshake(sock_num);
    }
    
    return 0;
}

/**
 * Setup and send the handshake packet to the new client. Spawn a new thread
 * to handle the client.
 **/
void connect_handshake(int sock_num) {
    char *              data_buff;
    struct sockaddr_in  cli_addr;
    unsigned int        cli_addr_len = sizeof(struct sockaddr_in);
    int                 bytes_trfd = 0;
    pthread_t           thr_1;
    int                 thr_id;
    int                 new_sock_num;
    
    /* Receive the handshake packet */
    // We don't care what is in it, just that it was a packet sent to the main server thread
    data_buff = malloc(MAX_HS_PKT_SZ);
    bytes_trfd = recvfrom(sock_num, data_buff, MAX_HS_PKT_SZ, 0, (struct sockaddr *) &cli_addr, &cli_addr_len);

    // create new socket / port
    if ((new_sock_num = create_udp_srv_sock(0)) < 0) { // didn't work
        printf("server - connect_handshake() : cannot open new socket to client");
    } else {
        // send ack on new port
        send_ack(new_sock_num, &cli_addr, 0);
        thr_id = pthread_create(&thr_1, NULL, do_threading, (void *) new_sock_num);         
    }
}

/**
 *
 **/
void * do_threading(void * ptr) {
    int                 sock_num = (int) ptr;
    char *              data_buff;
    char *              fill_buff;
    char *              file_buff;
    struct sockaddr_in  cli_addr;
    unsigned int        cli_addr_len = sizeof(struct sockaddr_in);
    unsigned int        seq_num = 0;    // if we get here, we know we've seen seq num 0, which can only be sent to main process
    int                 bytes_trfd = 0;
    char *              filename;
    int                 file_len;
    int                 filename_len = 0;
    FILE *              file_ptr;
    int                 data_len = 0;
    int                 buff_sz = 0;
    unsigned short  chk_sum = -1;
    unsigned int        last_seq_acked = 0;     // if we get here, we know we've seen and acked seq num 0

    /* Receive either data packet or init packet */
    while (1) {

        if (select_call(sock_num, SRV_WAIT_SECS, 0) == 1) {
            data_buff = malloc(MAX_DATA_PKT_SZ);
            fill_buff = data_buff;
            bzero(data_buff, MAX_DATA_PKT_SZ);

            bytes_trfd = recvfrom(sock_num, data_buff, MAX_DATA_PKT_SZ, 0, (struct sockaddr *) &cli_addr, &cli_addr_len);

            // first 4 bytes is seq num
            memcpy(&seq_num, fill_buff, sizeof(seq_num));
            seq_num = ntohl(seq_num);
            fill_buff = fill_buff + sizeof(seq_num);

            if (seq_num == last_seq_acked) {
                /* Previously seen, resend ack */
                send_ack(sock_num, &cli_addr, seq_num);
            } else if (seq_num == INIT_SEQ_NUM) {
                /* Parse the init packet */
                // 4 bytes - filename length
                memcpy(&filename_len, fill_buff, sizeof(filename_len));             // copy in to variable
                filename_len = ntohl(filename_len);                                 // translate to local byte order
                fill_buff = fill_buff + sizeof(filename_len);                       // advance pointer

                // 4 bytes - file length
                memcpy(&file_len, fill_buff, sizeof(file_len));                     // copy in to variable
                file_len = ntohl(file_len);                                         // translate to local byte order
                fill_buff = fill_buff + sizeof(file_len);                           // advance the pointer
                
                // filename
                filename = malloc(filename_len);
                memcpy(filename, fill_buff, filename_len);                          // copy in the variable
                
                // 2 bytes - checksum
                data_len = sizeof(seq_num) + sizeof(filename_len) + sizeof(file_len) + filename_len + sizeof(chk_sum);
                chk_sum = in_cksum((unsigned short *) data_buff, data_len);
                
                // open file for writing
                file_ptr = fopen(filename, "w+");

                // send acknowledgement             
                if (chk_sum == 0) {
                    send_ack(sock_num, &cli_addr, seq_num);
                    last_seq_acked = seq_num;
                }
            } else if (seq_num < INIT_SEQ_NUM) {
                /* Error Condition */
                printf("server - do_threading(): error sequence number");
                break;
            } else { // new data packet
                /* Setup a new packet */
                // 4 byte - seq num
                // already got the seq number from above
                
                // 2 byte - check sum
                // skip past it for now because we don't have the buff size yet
                fill_buff = fill_buff + sizeof(chk_sum);
                
                // 4 byte - buffer size
                memcpy(&buff_sz, fill_buff, sizeof(buff_sz));
                buff_sz = ntohl(buff_sz);

                // data
                file_buff = malloc(buff_sz);
                memcpy(file_buff, fill_buff, buff_sz);
                fill_buff = fill_buff + buff_sz;
                
                // calculate check sum
                data_len = sizeof(seq_num) + sizeof(chk_sum) + sizeof(buff_sz) + buff_sz;
                chk_sum = in_cksum((unsigned short *) data_buff, data_len);
                
                /* Write out if check sum is correct */
                if (chk_sum == 0) {
                    // write to file out
                    fwrite(file_buff, sizeof(char), buff_sz, file_ptr);
                    file_len = file_len - buff_sz;
                    
                    send_ack(sock_num, &cli_addr, seq_num);
                    last_seq_acked = seq_num;
                }

            }
        } else {
            if (file_len > 0) {
                printf("server - do_threading() : did not hear from client for %i secs; exiting this thread.\n", SRV_WAIT_SECS);
            }
            break;
        }
    }
    
    free(data_buff);
    fclose(file_ptr);
    return NULL;
}

/**
 * Send ack packet.
 **/
int send_ack(int sock_num, struct sockaddr_in * cli_addr, unsigned int seq_num) {
    int     bytes_trfd = 0;
    
    /* Create ack packet */
    seq_num = htonl(seq_num);
    
    /* Send */
    bytes_trfd = send_packet(sock_num, (char *) &seq_num, sizeof(seq_num), cli_addr);
    
    return bytes_trfd;
}