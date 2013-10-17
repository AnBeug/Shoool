/*********************************************
 * File         : server.c                   *
 * Author       : Annie Beug                 *
 * Date         : 28-FEB-2010                *
 * Modified     : 08-MAR-2010                *
 *********************************************/

#include "networks.h"
#include "debugger.h"

#define PORT_NUM 0

int main(int argc, char * argv[]) {
    float               err_pct;
    int                 sock_num;
    
    /* Get command line args */
    if (argc != 2) {
        debug_print("Usage: server <error percent>\n");
        return -1;
    }
    
    err_pct = atof(argv[1]);
    
    debug_print("main() 1: Starting rput with paramters:\n");
    debug_print("\terr_pct = %f\n", err_pct);

    /* Establish connection */
    // Get socket
    if ((sock_num = create_udp_srv_sock(PORT_NUM)) < 0) {
        debug_errors("creating socket");
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
        //debug_medium("main(): received handshake\n");
    }
    
    return 0;
}

/**
 *
 **/
void connect_handshake(int sock_num) {
    //debug_max("--------- receive handshake packet -------\n");
    
    /* Receive the handshake packet */
    // We don't care what is in it, just that it was a packet sent
    // to the main server thread
    char * data_buff = malloc(HS_PKT_SZ);
    struct sockaddr_in cli_addr;
    recv_packet(sock_num, data_buff, HS_PKT_SZ, &cli_addr);
    //int bytes_trfd = recv_packet(sock_num, data_buff, HS_PKT_SZ, &cli_addr);
    //debug_max("connect_handshake(): bytes_trfd = %i\n", bytes_trfd);  
    
    /* Create new socket and pass it to a new thread */
    pthread_t thr_1;
    int thr_id;
    int new_sock_num;
    if ((new_sock_num = create_udp_srv_sock(0)) < 0) { // didn't work
        debug_print("connect_handshake() : cannot open new socket to client");
    } else {
        // send ack on new port
        send_ack(new_sock_num, &cli_addr, HS_SEQ_NUM, ACK_PKT_TYPE);
        thr_id = pthread_create(&thr_1, NULL, do_threading, (void *) new_sock_num);         
        //debug_min("connect_handshake() 3: thr_id = %i\n", thr_id);
    }
}

/**
 * Receive the initial packet.
 * If no data is received, result = RES_NO_DATA and p_init_pkt is undefined;
 * Else if data is received but is corrupted, result = RES_DATA_CORRUPT;
 * Else if data is received but file cannot be opened, result = RES_FILE_ERR;
 * Else, data is received and check sum is verified, result = RES_SUCCESS, p_init_pkt contains init info
 *  and file
 **/
int recv_init(int sock_num, struct sockaddr_in * p_cli_addr, init_pkt_t ** p_p_init_pkt, FILE ** p_p_file) {
    int result = RES_NO_DATA;
    
    //debug_max("--------- receive init packet (wait %i secs) ---------\n", SRV_WAIT_SECS);
    if (select_call(sock_num, SRV_WAIT_SECS, 0)) {
        /* Get the packet */
        char * data_buff = malloc(MAX_INIT_PKT_SZ);
        recv_packet(sock_num, data_buff, MAX_INIT_PKT_SZ, p_cli_addr);
        //int bytes_trfd = recv_packet(sock_num, data_buff, MAX_INIT_PKT_SZ, p_cli_addr);
        //debug_max("recv_init(): bytes_trfd = %i\n", bytes_trfd);
        //print_bytes(data_buff, bytes_trfd);
        
        /* Parse the packet */
        *p_p_init_pkt = unmarshall_init_pkt(data_buff);
        
        // Check to see if we're getting the handshake again
        if ((*p_p_init_pkt)->hdr.seq_num == HS_SEQ_NUM) {
            send_ack(sock_num, p_cli_addr, HS_SEQ_NUM, ACK_PKT_TYPE);
        }
        
        if ((*p_p_init_pkt)->hdr.chk_sum != 0) {
            //debug_min("recv_init(): p_init_pkt->hdr.chk_sum = %hu\n", (*p_p_init_pkt)->hdr.chk_sum);
            //print_sockaddr_info(p_cli_addr);
            // NAK the packet
            send_ack(sock_num, p_cli_addr, INIT_SEQ_NUM, NAK_PKT_TYPE);
            return RES_DATA_CORRUPT;
        }
        
        /* Try to open the file */
        *p_p_file = fopen((*p_p_init_pkt)->filename, "w+");
        if (*p_p_file == NULL) {
            perror("Could not open file for writing");
            send_ack(sock_num, p_cli_addr, INIT_SEQ_NUM, FILE_ERR_PKT_TYPE);
            return RES_FILE_ERR;
        }
        
        /* Send positive acknowledgement */
        send_ack(sock_num, p_cli_addr, INIT_SEQ_NUM, ACK_PKT_TYPE);
        result = RES_SUCCESS;
        //debug_max("recv_init(): result = %i\n", result);    
    } else {
        //debug_medium("recv_int(): did not find any data to receive\n");
    }
    
    return result;
}

/**
 * Receives a data packet and unmarshalls it in to a data packet.
 * Select call is done before entering this function.
 * Malloc's and return filled data packet.
 * Check sum is calculated when the byte array is unmarshalled.
 **/
data_pkt_t * recv_data(int sock_num, struct sockaddr_in * p_cli_addr) {
    data_pkt_t *    p_data_pkt;
    char *          data_buff = malloc(MAX_DATA_PKT_SZ);
    
    /* Receive bytes */
    u_int cli_addr_len = sizeof(struct sockaddr_in);
    recvfrom(sock_num, data_buff, MAX_DATA_PKT_SZ, 0, (struct sockaddr *) p_cli_addr, &cli_addr_len);
    //int bytes_trfd = recvfrom(sock_num, data_buff, MAX_DATA_PKT_SZ, 0, (struct sockaddr *) p_cli_addr, &cli_addr_len);
    ////debug_max("recv_data(): bytes_trfd = %i\n", bytes_trfd);
    
    /* Unmarshall */
    p_data_pkt = unmarshall_data_packet(data_buff);
    
    return p_data_pkt;
}

/**
 * Implement Go-Back-N sliding windows to receive the data packets.
 **/
void recv_file(int sock_num, struct sockaddr_in * p_cli_addr, u_int file_len, u_int buffer_sz, FILE * p_file) {
    u_int   last_pkt_ackd = INIT_SEQ_NUM;
    u_int   next_pkt = last_pkt_ackd + 1;
    u_int   total_seqs = ((int) ceil((double) file_len / (double) buffer_sz)) + INIT_SEQ_NUM;
    
    //debug_max("------------- receive file -------------\n");
    //debug_max("recv_file(): total_seqs = %i\n", total_seqs);
    
    /* Start receiving packets */
    data_pkt_t * p_data_pkt = NULL;
    
    int i = 0;
    
    while (next_pkt <= total_seqs) { // we still have packets to receive
        i++;
        //debug_max("recv_file(): round = %i\n", i);
        //debug_max("recv_file(): next_pkt = %i <= total_seq = %i\n", next_pkt, total_seqs);
        
        if (select_call(sock_num, SRV_WAIT_SECS, 0)) {
            p_data_pkt = recv_data(sock_num, p_cli_addr);
            
            //debug_max("\trecv_file(): p_data_pkt->hdr.chk_sum = %i\n", p_data_pkt->hdr.chk_sum);
            //debug_max("\trecv_file(): p_data_pkt->hdr.seq_num = %i -v- next_pkt = %i\n", p_data_pkt->hdr.seq_num, next_pkt);

            if (p_data_pkt->hdr.chk_sum !=0 ) {
                // packet is corrupt, don't use
                //debug_max("\trecv_file(): check sum error\n");
                send_ack(sock_num, p_cli_addr, next_pkt, NAK_PKT_TYPE);
            } else if (p_data_pkt->hdr.seq_num == next_pkt) {
                // this is the one we're looking for
                // check the check sum to decide what to do
                //debug_max("\trecv_file(): send ack seq_num = %i\n", p_data_pkt->hdr.seq_num);
                // packet is uncorrupted, write to file and ACK
                fwrite(p_data_pkt->data, sizeof(char), p_data_pkt->hdr.buffer_sz, p_file);
                send_ack(sock_num, p_cli_addr, p_data_pkt->hdr.seq_num, ACK_PKT_TYPE);
                last_pkt_ackd = p_data_pkt->hdr.seq_num;
                next_pkt = last_pkt_ackd + 1;
            } else if (p_data_pkt->hdr.seq_num == 1) {
                // catch the case where the init packet gets resent
                // and simply re-ACK
                send_ack(sock_num, p_cli_addr, INIT_PKT_TYPE, ACK_PKT_TYPE);
            } else if (p_data_pkt->hdr.seq_num < next_pkt) { 
                // we've already seen, re-ack
                //debug_max("\t\trecv_file(): re-acking %i\n", p_data_pkt->hdr.seq_num);
                send_ack(sock_num, p_cli_addr, last_pkt_ackd, ACK_PKT_TYPE);
            } else {    
                // too high, we must have missed one, NAK what we were looking for
                //debug_max("\t\trecv_file(): NAKing... p_data_pkt->hdr.chk_sum = %hu\n", p_data_pkt->hdr.chk_sum);
                send_ack(sock_num, p_cli_addr, next_pkt, NAK_PKT_TYPE);
            }
            
        } else {
            // heard nothing from the client
            debug_errors("recv_file(): heard nothing from client; exiting\n");
            break;
        }
        
    }
    
    debug_print("recv_file(): finished receiving file from client\n");
}

/**
 * Called when a new thread is created.
 * Receives the init packet and then the data packets.
 * Exits when done or if communication with client is lost.
 **/
void * do_threading(void * ptr) {
    int                     sock_num = (int) ptr;
    struct sockaddr_in      cli_addr;
    FILE *                  p_file = malloc(sizeof(FILE));
    init_pkt_t *            p_init_pkt = malloc(INIT_PKT_SZ);
    
    //debug_max("-------- new thread -----------\n");
    /* Receive the init packet */
    // Keep trying to receive the init pointer
    // How many times should we try this?
    int init_res = RES_NO_DATA;
    while (init_res != RES_SUCCESS) {
        init_res = recv_init(sock_num, &cli_addr, &p_init_pkt, &p_file);
        //debug_max("do_threading()/recv_init: init_res = %i\n", init_res);
        
        if (init_res == RES_NO_DATA) {
            debug_errors("do_threading(): did not receive init packet from client; exiting\n");
            return NULL;
        } else if (init_res == RES_FILE_ERR) {
            debug_errors("do_threading(): could not open file for writing; exiting\n");
            return NULL;
        }
    
    }
    if (init_res != RES_SUCCESS) {
        debug_errors("do_threading(): could not get the init packet; exiting\n");
        return NULL;
    }
    
    /* Receive the data packets */
    recv_file(sock_num, &cli_addr, p_init_pkt->hdr.file_len, p_init_pkt->hdr.buffer_sz, p_file);
    
    fclose(p_file);
    free(p_init_pkt);
    
    //debug_max("do_threading(): finished with this client\n");
    
    return NULL;
}

/**
 * Send ack packet.
 **/
void send_ack(int sock_num, struct sockaddr_in * cli_addr, u_int seq_num, u_char ack_type) {
    /* Create ack packet */
    ack_pkt_t ack_pkt;
    ack_pkt.seq_num = seq_num;
    ack_pkt.chk_sum = 0;
    ack_pkt.pkt_type = ack_type;
    //debug_max("send_ack(): ack_type = %i\n", ack_type);
    
    /* Marshall to a byte array */
    char * data_buff = marshall_ack_pkt(&ack_pkt);
        
    /* Send */
    send_packet(sock_num, data_buff, ACK_PKT_SZ, cli_addr);
    //int bytes_trfd = send_packet(sock_num, data_buff, ACK_PKT_SZ, cli_addr);
    //debug_max("send_ack(): bytes_trfd = %i\n", bytes_trfd);
}

/**
 * Copy data from a byte array in to an init packet struct.
 * Calculates the check sum and sticks it in the checksum location
 * in the header. If checksum value in the header is 0 after
 * unmarshalling, the packet is uncorrupted.
 * Malloc's and returns the init packet.
 **/
init_pkt_t * unmarshall_init_pkt(char * data_buff) {
    init_pkt_t *    p_init_pkt = malloc(INIT_PKT_SZ);
    char *          fill_buff = data_buff;
    
    /* Copy in to the header */
    init_hdr_t init_hdr;
    memcpy(&init_hdr, fill_buff, INIT_HDR_SZ);
    p_init_pkt->hdr = init_hdr; 
    
    /* Check filename length */
    if (p_init_pkt->hdr.filename_len >= MIN_FILENAME_SZ && p_init_pkt->hdr.filename_len <= MAX_FILENAME_SZ) {
        /* Copy over the file name */
        fill_buff = fill_buff + INIT_HDR_SZ;
        p_init_pkt->filename = malloc(p_init_pkt->hdr.filename_len);
        memcpy(p_init_pkt->filename, fill_buff, p_init_pkt->hdr.filename_len);
        
        /* Calculate the checksum */
        int data_len = INIT_HDR_SZ + p_init_pkt->hdr.filename_len;
        u_short chk_sum = in_cksum((unsigned short *) data_buff, data_len);
        p_init_pkt->hdr.chk_sum = chk_sum;
    } else {
        // We know the filename length is corrupt
        // so we can't actually calculate the checksum
        // Force the checksum to be non-zero
        //debug_max("unmarshall_init_pkt(): problem with filename_len = %i\n", p_init_pkt->hdr.filename_len);
        p_init_pkt->hdr.chk_sum = 1;
    }
    
    print_init_pkt("unmarshall_init_pkt", p_init_pkt);
    
    return p_init_pkt;
}

/**
 * Copy data from a byte array in to an init packet struct.
 * Malloc's and returns the packet.
 * Calculates the check sum and sticks it in the checksum location
 * in the header. If checksum value in the header is 0 after
 * unmarshalling, the packet is uncorrupted.
 * Other values are converted to host network order as necessary.
 **/
data_pkt_t * unmarshall_data_packet(char * data_buff) {
    data_pkt_t *    p_data_pkt = malloc(DATA_PKT_SZ);
    char *          fill_buff = data_buff;
    
    /* Copy over ther header */
    data_hdr_t data_hdr;
    memcpy(&data_hdr, data_buff, DATA_HDR_SZ);
    p_data_pkt->hdr = data_hdr;
    
    /* Init packet resent */
    // check the case where the init packet has gotten resent
    // If so, check sum will never work
    if (p_data_pkt->hdr.seq_num == INIT_PKT_TYPE) {
        p_data_pkt->hdr.chk_sum = 0;
        return p_data_pkt;
    }
    
    /* Check that the buffer size is not corrupt */
    if (p_data_pkt->hdr.buffer_sz >= MIN_BUFFER_SZ && p_data_pkt->hdr.buffer_sz <= MAX_BUFFER_SZ) {
        /* Copy over the payload */
        fill_buff = fill_buff + DATA_HDR_SZ;
        p_data_pkt->data = malloc(p_data_pkt->hdr.buffer_sz);
        memcpy(p_data_pkt->data, fill_buff, p_data_pkt->hdr.buffer_sz);
        
        /* Calculate the check sum */
        int data_len = DATA_HDR_SZ + p_data_pkt->hdr.buffer_sz;
        u_short chk_sum = in_cksum((unsigned short *) data_buff, data_len);
        p_data_pkt->hdr.chk_sum = chk_sum;
    } else {
        // We cannot properly calculate the checksum
        // since the buffer size is corrupt
        // Force it to be non-zero
        //debug_max("unmarshall_data_packet(): buffer size error\n");
        p_data_pkt->hdr.chk_sum = 1;
    }

    //print_data_pkt("unmarshall_data_packet", p_data_pkt);
    
    return p_data_pkt;
}

/**
 * Copy from the ack packet in to the byte array.
 **/
char * marshall_ack_pkt(ack_pkt_t * p_ack_pkt) {
    char * data_buff = malloc(ACK_PKT_SZ);
    
    /* Copy in to byte array */
    memcpy(data_buff, p_ack_pkt, ACK_PKT_SZ);
    
    /* Calculate check sum */
    ((ack_pkt_t *) data_buff)->chk_sum = 0;
    ((ack_pkt_t *) data_buff)->chk_sum = in_cksum((unsigned short *) data_buff, ACK_PKT_SZ);
    
    return data_buff;
}