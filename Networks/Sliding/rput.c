/**
 *
 */

#include "networks.h"
#include "debugger.h"
#include "rput.h"

int main(int argc, char * argv[]) {
    char *              local_file_name;
    char *              remote_file_name;
    int                 buffer_sz = 0;
    float               err_pct = 0.0;
    int                 window_sz = 0;
    char *              remote_host;
    int                 remote_port;
    
    int                 sock_num = -1;
    struct hostent *    host_ent;
    struct sockaddr_in  srv_addr;
    u_int               file_len;
    FILE *              file_ptr;
    
    /* Get command line arguments */
    if (argc != 8) {
        debug_errors("Usage: rput <local file name> <remote file name> <buffer size> <error percent> <window size> <remote host> <remote port>\n");
        return -1;
    }
    
    local_file_name = argv[1];
    remote_file_name = argv[2];
    buffer_sz = atoi(argv[3]);
    err_pct = atof(argv[4]);
    window_sz = atoi(argv[5]);
    remote_host = argv[6];
    remote_port = atoi(argv[7]);
    
    debug_print("Starting rput with paramters:\n");
    debug_print("\tlocal_file_name = '%s'\n", local_file_name);
    debug_print("\tremote_file_name = '%s'\n", remote_file_name);
    debug_print("\tbuff_sz = %i\n", buffer_sz);
    debug_print("\terr_pct = %f\n", err_pct);
    debug_print("\twindow_sz = %i\n", window_sz);
    debug_print("\tremote_host = '%s'\n", remote_host);
    debug_print("\tremote_port = %i\n", remote_port);
    
    /* Setup socket and server address */
    // Create a UDP socket
    sock_num = create_udp_sock();
    
    // Get the name
    host_ent = gethostbyname(remote_host);
    if (host_ent == NULL) {
        perror("could not create host entry");
        return -1;
    }
    
    // Get the address
    srv_addr.sin_family = AF_INET;
    memcpy((char *) &srv_addr.sin_addr, (char *) host_ent->h_addr, host_ent->h_length);
    srv_addr.sin_port = htons(remote_port);
    
    /* Check that the file can be accessed */
    file_len = check_file(local_file_name, &file_ptr);
    if (file_len < 0) {
        perror("could not stat file");
        return -1;
    }
    
    /* Init error sending */
    // error rate, drop flag, flip flag, debug flag, random flag
    if (sendtoErrG_init(err_pct, 1, 1, 0, 1) < 0) {
        perror("calling sendtoErrG_init");
        exit(-1);
    }   
    
    /* Send the packets */
    // If we hear nothing from the server after sending 10 times, quit
    
    /* Send the handshake packet */
    ack_pkt_t * p_ack_pkt = malloc(ACK_PKT_SZ);
    int ack_reply = -1;
    int send_attempt = 0;
    
    while (send_attempt < MAX_CLI_RETRIES && ack_reply < 0) {
        //debug_max("\n--------- hand shake ---------\n");
        
        /* Up our send attempt count */
        //debug_medium("main()/send_handshake_packet: send_attempt = %i\n", send_attempt);
        send_attempt++;
        
        /* Send the handshake packet */
        send_handshake_packet(sock_num, &srv_addr);
        
        /* Get an ack */
        if (select_call(sock_num, CLI_WAIT_SECS, 0)) {
            ack_reply = recv_ack(sock_num, &srv_addr, &p_ack_pkt);
            debug_medium("main()/send_handshake_packet: ack_reply = %i\n", ack_reply);
            print_ack_pkt("main() 2", p_ack_pkt);
            
            // We heard something, but not a positive ack
            // So, we reset our send attempt but still try again
            if (p_ack_pkt->chk_sum != 0) {
                //debug_medium("main()/send_handshake_packet: check sum = %i\n", p_ack_pkt->chk_sum);
                ack_reply = -1;
                send_attempt = 0;
            } else if (p_ack_pkt->pkt_type == NAK_PKT_TYPE) {
                //debug_medium("main()/send_handshake_packet: p_ack_pkt->pkt_type = %i\n", NAK_PKT_TYPE);
                ack_reply = -1;
                send_attempt = 0;
            } else {
                // positive ack
                //debug_medium("main()/send_handshake_packet: p_ack_pkt->seq_num = %i\n", p_ack_pkt->seq_num);
            }
        }
    }
    /* Check to see if we got a successful ack */
    if (ack_reply < 0) {
        debug_errors("main()/send_handshake_packet: could not communicate with server; giving up\n");
        return -1;
    }
    
    /* Send the init packet */
    ack_reply = -1;
    send_attempt = 0;
    
    while (send_attempt < MAX_CLI_RETRIES && ack_reply < 0) {
        //debug_max("--------- init ---------\n");
        
        /* Up our send attempt count */
        send_attempt++;
        debug_medium("main()/send_init_packet: send_attempt = %i\n", send_attempt);
        
        /* Send the init packet */
        send_init_packet(sock_num, &srv_addr, remote_file_name, file_len, buffer_sz, window_sz);
        
        /* Get an ack back */
        if (select_call(sock_num, CLI_WAIT_SECS, 0)) {
            ack_reply = recv_ack(sock_num, &srv_addr, &p_ack_pkt);
            //debug_medium("main()/send_init_packet: ack_reply = %i\n", ack_reply);
            
            if (p_ack_pkt->chk_sum != 0) {
                //debug_medium("main()/send_init_packet: check sum = %i\n", p_ack_pkt->chk_sum);
                ack_reply = -1;
                send_attempt = 0;
            } else if (p_ack_pkt->pkt_type == NAK_PKT_TYPE) {
                //debug_medium("main()/send_init_packet: pkt_type sum = %i\n", p_ack_pkt->pkt_type);
                ack_reply = -1;
                send_attempt = 0;
            } else if (p_ack_pkt->pkt_type == FILE_ERR_PKT_TYPE) {
                //debug_medium("main()/send_init_packet: server could not write file to disk\n");
                return -1;
            } else {
                //debug_medium("main()/send_init_packet: seq_num = %i\n", p_ack_pkt->seq_num);
            }
        }
    }
    if (ack_reply < 0) {
        debug_errors("main()/send_init_packet: could not communicate with server\n");
        return -1;
    }
    free(p_ack_pkt);
    
    /* Send the file */
    //debug_max("--------- file ---------\n");
    send_file(sock_num, &srv_addr, file_ptr, file_len, buffer_sz, window_sz);
    fclose(file_ptr);
    
    //debug_max("main() 5: done\n");
    
    return 0;
}

/**
 * Sends the handshake packet.
 * Seq # = 0.
 * Returns the number of bytes transfered.
 **/
void send_handshake_packet(int sock_num, struct sockaddr_in * srv_addr) {
    //debug_max("send_handshake_packet()\n");
    
    /* Create handshake packet */
    handshake_pkt_t hs_pkt;
    hs_pkt.seq_num = HS_SEQ_NUM;
    hs_pkt.pkt_type = HS_PKT_TYPE;      // don't need to put in network order because only 1 byte
    hs_pkt.chk_sum = 0;
    
    /* Calculate check sum */
    u_short chk_sum = in_cksum((unsigned short *) &hs_pkt, HS_PKT_SZ);
    //debug_max("send_handshake_packet(): chk_sum = %hu\n", chk_sum);
    hs_pkt.chk_sum = chk_sum;
    
    /* Send it on over */
    send_packet(sock_num, (char *) &hs_pkt, HS_PKT_SZ, srv_addr);
    //int bytes_trfd = send_packet(sock_num, (char *) &hs_pkt, HS_PKT_SZ, srv_addr);
    //debug_max("send_handshake_packet(): bytes_trfd = %i\n", bytes_trfd);
}

/**
 * Send init packet with filename and length.
 * Wait for ack. 
 * Return new port.
 **/
void send_init_packet(int sock_num, struct sockaddr_in * srv_addr, char * filename, u_int file_len, u_int buffer_sz, u_int window_sz) {
    /* Allocate the memory */
    int filename_len = strlen(filename) + 1;    // account for null termination
    int data_len = INIT_HDR_SZ + filename_len;  // total packet size is header size + filename length at end
    init_pkt_t * p_init_pkt = malloc(INIT_PKT_SZ);
    
    /* Put stuff in it */
    p_init_pkt->hdr.seq_num = INIT_SEQ_NUM;
    p_init_pkt->hdr.chk_sum = 0;
    p_init_pkt->hdr.pkt_type = INIT_PKT_TYPE;
    p_init_pkt->hdr.filename_len = filename_len;
    p_init_pkt->hdr.file_len = file_len;
    p_init_pkt->hdr.buffer_sz = buffer_sz;
    p_init_pkt->hdr.window_sz = window_sz;
    p_init_pkt->filename = malloc(filename_len);
    memcpy(p_init_pkt->filename, filename, filename_len);
    
    print_init_pkt("send_init_packet", p_init_pkt);
    
    /* Marshall it */
    char * data_buff = marshall_init_pkt(p_init_pkt);
    
    //print_bytes(data_buff, data_len);
    send_packet(sock_num, data_buff, data_len, srv_addr);
    
    free(data_buff);
    free(p_init_pkt);
}

/**
 * Returns the total number of bytes successfully transfered.
 */
void send_file(int sock_num, struct sockaddr_in * srv_addr, FILE * p_file, u_int file_len, u_int buff_sz, u_int window_sz) {
    u_int           last_seq_ackd = INIT_SEQ_NUM;
    u_int           seq_num_to_send = last_seq_ackd + 1;
    u_int           last_seq = ((int) ceil((double) file_len / (double) buff_sz)) + INIT_SEQ_NUM;
    u_int           highest_sendable_seq = min(last_seq_ackd + window_sz, last_seq);
    u_int           windex = seq_num_to_send % window_sz;
    u_int           send_attempt = 0;
    data_pkt_t *    p_send_pkt;
    ack_pkt_t *     p_ack_pkt;
    
    
    data_pkt_t *    packets[window_sz];
    int j;
    for (j = 0; j < window_sz; j++) {
        packets[j] = NULL;
    }
    
    //debug_max("send_file(): last_seq = %i\n", last_seq);
    
    debug_max("\tsend_file(): seq_num_to_send = %i <= last_seq = %i\n", seq_num_to_send, last_seq);
    debug_max("\tsend_file(): or last_seq_ackd = %i < last_seq = %i\n", last_seq_ackd, last_seq); // TODO should this be equal?
    while (seq_num_to_send <= last_seq || last_seq_ackd < last_seq) {
        // there's still an outstanding packet to send
        debug_max("\t\tsend_file(): seq_num_to_send = %i <= highest_sendable_seq = %i\n", seq_num_to_send, highest_sendable_seq);
        
        /* Window is open */
        while (seq_num_to_send <= highest_sendable_seq) {
            
            /* Send packet */
            // check to see if it's already buffered
            windex = seq_num_to_send % window_sz;
            if (packets[windex] == NULL || packets[windex]->hdr.seq_num != seq_num_to_send) {
                // not buffered, read in
                p_send_pkt = build_packet(seq_num_to_send, p_file, buff_sz);
                packets[windex] = p_send_pkt;
            } else {
                // buffered, use
                p_send_pkt = packets[windex];
            }
            // send it
            send_file_packet(sock_num, srv_addr, p_send_pkt);
            seq_num_to_send = seq_num_to_send + 1;
            
            /* Check for an ACK */
            if (select_call(sock_num, 0, 1)) {
                int ack_reply = recv_ack(sock_num, srv_addr, &p_ack_pkt);
                //debug_max("\t\tsend_file(): ack_reply = %i\n",  ack_reply);
                
                if (ack_reply > 0 && p_ack_pkt->chk_sum == 0 && p_ack_pkt->pkt_type == ACK_PKT_TYPE) {
                    // ACK is good to use
                    debug_max("\t\tsend_file(): p_ack_pkt->seq_num = %hu\n",  p_ack_pkt->seq_num);
                    last_seq_ackd = max(last_seq_ackd, p_ack_pkt->seq_num);
                    //highest_sendable_seq = min(last_seq_ackd + window_sz, (last_seq-1)); // Why am I minusing one
                    highest_sendable_seq = min(last_seq_ackd + window_sz, (last_seq));
                } else if (p_ack_pkt->chk_sum != 0) {
                    // ACK is corrupt, do not use
                    debug_max("\t\tsend_file(): p_ack_pkt->chk_sum = %hu\n",  p_ack_pkt->chk_sum);
                } else if (p_ack_pkt->pkt_type == NAK_PKT_TYPE) {
                    // ACK signals an error we need to deal with
                    debug_max("\t\tsend_file(): received NAK for seq_num = %i\n",  p_ack_pkt->seq_num);
                    
                    // roll back to seq num sent in NAK
                    // assume last_seq_ackd < NAK->seq_num < seq_num_to_send
                    seq_num_to_send = p_ack_pkt->seq_num;                   
                } else {
                    //debug_max("\t\tsend_file(): unknown ack problem");
                }
                
                send_attempt = 0;
            } else {
                // Do not timeout if we don't find an ack, continue sending
                //debug_max("\t\tsend_file(): did not find an ack\n");
            }
        }
        
        // window closed, wait on an ACKs/NAKs
        debug_max("send_file(): window closed; seq_num_to_send = %i <= last_seq = %i\n", seq_num_to_send, last_seq);
        if (seq_num_to_send <= last_seq && select_call(sock_num, CLI_WAIT_SECS, 0)) {
            int ack_reply = recv_ack(sock_num, srv_addr, &p_ack_pkt);

            debug_max("\t\tsend_file(): window closed; ack reply = %i\n",  ack_reply);
            debug_max("\t\tsend_file(): window closed; chk sum = %i\n",  p_ack_pkt->chk_sum);
            debug_max("\t\tsend_file(): window closed; pkt_type = %i\n",  p_ack_pkt->pkt_type);

            if (ack_reply > 0 && p_ack_pkt->chk_sum == 0 && p_ack_pkt->pkt_type == ACK_PKT_TYPE) {              
                // ACK is good to use
                //debug_max("\t\tsend_file()-2: p_ack_pkt->seq_num = %hu\n",  p_ack_pkt->seq_num);
                last_seq_ackd = max(last_seq_ackd, p_ack_pkt->seq_num);
                //debug_max("\t\tsend_file()-2: last_seq_ackd = %hu\n",  last_seq_ackd);
                highest_sendable_seq = min(last_seq_ackd + window_sz, last_seq);
                //debug_max("\t\tsend_file()-2: highest_sendable_seq = %hu\n",  highest_sendable_seq);
            } else if (p_ack_pkt->chk_sum != 0) {
                // ACK is corrupt, do not use
                //debug_max("\t\tsend_file()-2: p_ack_pkt->chk_sum = %hu\n",  p_ack_pkt->chk_sum);
            } else if (p_ack_pkt->pkt_type == NAK_PKT_TYPE) {
                // ACK signals an error we need to deal with
                //debug_max("\t\tsend_file()-2: p_ack_pkt->pkt_type = %i\n",  p_ack_pkt->pkt_type);
                
                // roll back to seq num sent in NAK
                // assume last_seq_ackd < NAK->seq_num < seq_num_to_send
                seq_num_to_send = p_ack_pkt->seq_num;                   
            } else {
                //debug_max("\t\tsend_file()-2: unknown ack problem");
            }
            
            send_attempt = 0;
        } else {
            /* Timeout condition */
            // As long as we're not waiting for the final ack, resend if we haven't heard from the server after a certain
            //debug_max("\t\tsend_file()-2: send_attempt = %i < MAX_CLI_RETRIES = %i\n", send_attempt, MAX_CLI_RETRIES);
            if (send_attempt < MAX_CLI_RETRIES) {
                send_attempt++;
                // resend from after the last ACK'd packet
                seq_num_to_send = last_seq_ackd + 1;
            } else {
                // Haven't heard anything back from the server in too long
                debug_errors("send_file(): unable to communicate with server; exiting\n");
                break;
            }
        }
        
    }
    
    debug_print("send_file(): finished sending file\n");
    
    free(p_ack_pkt);
}

/**
 * Send a single file packet
 * Assumes p_data_pkt is malloc'd and set.
 * Marshalls data before sending.
 **/
void send_file_packet(int sock_num, struct sockaddr_in * srv_addr, data_pkt_t * p_data_pkt) {
    /* Marshall data for sending */
    char * data_buff = marshall_data_pkt(p_data_pkt);
    
    /* Send data */
    int data_len = DATA_PKT_SZ + p_data_pkt->hdr.buffer_sz;
    send_packet(sock_num, data_buff, data_len, srv_addr);
}

/**
 * Fetches data from the file.
 * Returns the built data packet.
 **/
data_pkt_t * build_packet(u_int seq_num, FILE * p_file, u_int buff_sz) {
    data_pkt_t *    p_data_pkt = malloc(DATA_PKT_SZ);
    char *          read_buff = malloc(buff_sz);
    
    /* Read a buffer in from the file */
    bzero(read_buff, buff_sz);
    int bytes_read = fread(read_buff, sizeof(char), buff_sz, p_file);
    ////debug_max("build_packet(): read_buff = '%s'\n", read_buff);
    //debug_max("build_packet(): bytes_read = %i\n", bytes_read);
    
    /* Fill in the packet */
    p_data_pkt->hdr.seq_num = seq_num;
    p_data_pkt->hdr.pkt_type = DATA_PKT_TYPE;
    p_data_pkt->hdr.buffer_sz = bytes_read;
    p_data_pkt->hdr.chk_sum = 0;
    p_data_pkt->data = malloc(bytes_read);
    memcpy(p_data_pkt->data, read_buff, bytes_read);
    
    return p_data_pkt;
}

/**
 * Receive ack packet. Assumes select call has already been.
 * Returns the sequence number received in the ack, -1 if the ack packet has been
 * corrupted (check sum does not check out).
 * Assumes the packet pointer has already been malloc'ed.
 **/
int recv_ack(int sock_num, struct sockaddr_in * srv_addr, ack_pkt_t ** p_p_ack_pkt) {
    u_int       seq_num = -1;
    
    /* Receive the data */
    char * data_buff = malloc(ACK_PKT_SZ);
    recv_packet(sock_num, data_buff, ACK_PKT_SZ, srv_addr);
    //int bytes_trfd = recv_packet(sock_num, data_buff, ACK_PKT_SZ, srv_addr);
    *p_p_ack_pkt = unmarshall_ack_pkt(data_buff);
    print_ack_pkt("recv_ack", *p_p_ack_pkt);
    
    //debug_max("recv_ack(): bytes_trfd = %i\n", bytes_trfd);
    
    /* Check to see if packet is corrupt */
    if ((*p_p_ack_pkt)->chk_sum == 0) {
        seq_num = (*p_p_ack_pkt)->seq_num;
        //debug_max("recv_ack(): seq_num = %i\n", (*p_p_ack_pkt)->seq_num);
    } else {
        //debug_max("recv_ack(): chk_sum = %hu\n", (*p_p_ack_pkt)->chk_sum);
    }
    
    return seq_num;
}

/**
 * Opens the file pointer and stats the file.
 * Return the size of the file or -1 if the file cannot be opened or the file stats cannot be read.
 **/
int check_file(char * filename, FILE ** file_ptr) {
    int             file_len = -1;
    struct stat     file_stats;
    
    /* Make sure we can open the file or there's not point in doing anything else */
    if (stat(filename,  &file_stats) != 0) {
        perror("could not stat() file");
        return -1;
    }
    // make sure we can open the file
    if ((*file_ptr = fopen(filename, "r")) == NULL)
    {
        perror("could not fopen() file");
        return -1;
    }
    
    // size of file
    file_len = file_stats.st_size;
    
    //debug_max("check_file() : file_len = %i\n", file_len);
    
    return file_len;
}

/**
 * Converts each element in the packet struct to network byte order if necessary, 
 * copies it in to the byte array, converts back to host order. At the end, 
 * the filename is copied in to the byte array after the 
 * Check sum is calculated copied in to the appropriate location.
 * Malloc's and returns the byte array.
 **/
char * marshall_init_pkt(init_pkt_t * p_init_pkt) {
    int     data_len = INIT_HDR_SZ + p_init_pkt->hdr.filename_len;
    char *  data_buff = malloc(data_len);
    char *  fill_buff = data_buff;
    
    /* Copy over the header */
    memcpy(fill_buff, &(p_init_pkt->hdr), INIT_HDR_SZ);
    fill_buff = fill_buff + INIT_HDR_SZ;
    
    /* Copy over the filename */
    memcpy(fill_buff, p_init_pkt->filename, p_init_pkt->hdr.filename_len);
    
    /* Calculate the check sum */
    ((init_hdr_t *) data_buff)->chk_sum = 0;
    ((init_hdr_t *) data_buff)->chk_sum = in_cksum((unsigned short *) data_buff, data_len);
    
    return data_buff;
}

/**
 * Converts each element in the packet struct to network byte order if necessary, 
 * copies it in to the byte array, converts back to host order. At the end, 
 * the filename is copied in to the byte array after the 
 * Check sum is calculated copied in to the appropriate location.
 * Assumes that the init packet and the byte array has already been properly malloc'd and 
 * filled.
 **/
char * marshall_data_pkt(data_pkt_t * p_data_pkt) {
    int     data_len = DATA_PKT_SZ + p_data_pkt->hdr.buffer_sz;
    char *  data_buff = malloc(data_len);
    char *  fill_buff = data_buff;
    
    /* Copy over the header */
    bzero(data_buff, data_len);
    //debug_max("marshall_data_pkt(): seq_num = %i\n", p_data_pkt->hdr.seq_num);
    memcpy(fill_buff, &(p_data_pkt->hdr), DATA_HDR_SZ);
    fill_buff = fill_buff + DATA_HDR_SZ;
    
    /* Copy over the data */
    memcpy(fill_buff, p_data_pkt->data, p_data_pkt->hdr.buffer_sz);
    
    /* Calculate the check sum */
    ((data_hdr_t *) data_buff)->chk_sum = 0;
    ((data_hdr_t *) data_buff)->chk_sum = in_cksum((unsigned short *) data_buff, data_len);
    ////debug_max("marshall_data_pkt(): ((data_hdr_t *) data_buff)->chk_sum = %hu\n", ((data_hdr_t *) data_buff)->chk_sum);
    
    return data_buff;
}

/**
 * Copy from the byte array to the ack packet.
 * Calculates and sets the check sum.
 **/
ack_pkt_t * unmarshall_ack_pkt(char * data_buff) {
    ack_pkt_t * p_ack_pkt = malloc(ACK_PKT_SZ);
    
    /* Copy over the packet */
    memcpy(p_ack_pkt, data_buff, ACK_PKT_SZ);
    
    /* Calculate the check sum */
    u_short chk_sum = in_cksum((unsigned short *) p_ack_pkt, ACK_PKT_SZ);
    p_ack_pkt->chk_sum = chk_sum;
    
    /* Print it out */
    print_ack_pkt("unmarshall_ack_pkt", p_ack_pkt);
    
    return p_ack_pkt;
}
