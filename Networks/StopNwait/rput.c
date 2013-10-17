/**
 *
 */

#include "networks.h"
#include "rput.h"

int main(int argc, char * argv[]) {
	char *				local_file_name;
	char *				remote_file_name;
	int					buff_sz;
	float				err_pct;
	char *				remote_host;
	int					remote_port;
	
	int					sock_num = -1;
	struct hostent *	host_ent;
	struct sockaddr_in	srv_addr;
	unsigned int		file_len;
	unsigned int		bytes_trfd;
	FILE *				file_ptr;
	
	/* Get command line arguments */
	if (argc != 7) {
		printf("Usage: rput <local file name> <remote file name> <buffer size> <error percent> <remote host> <remote port>\n");
		return -1;
	}
	
	local_file_name = argv[1];
	remote_file_name = argv[2];
	buff_sz = atoi(argv[3]);
	err_pct = atof(argv[4]);
	remote_host = argv[5];
	remote_port = atoi(argv[6]);
	
	printf("Starting rput with paramters:\n");
	printf("\tlocal_file_name = '%s'\n", local_file_name);
	printf("\tremote_file_name = '%s'\n", remote_file_name);
	printf("\tbuff_sz = %i\n", buff_sz);
	printf("\terr_pct = %f\n", err_pct);
	printf("\tremote_host = '%s'\n", remote_host);
	printf("\tremote_port = %i\n", remote_port);

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
		perror("could not open / stat file");
		return -1;
	}
	
	//printf("got here a\n");
	
	/* Init error sending */
	// error rate, drop flag, flip flag, debug flag, random flag
	if (sendtoErrG_init(err_pct, 1, 1, 0, 1) < 0) {
		perror("calling sendtoErrG_init");
		exit(-1);
	}	
	
	/* Send the packets */
	// Keep going for 10 times if we don't succeed; after the 10th try, give up and quit.
	int ack_reply = -1;
	int send_attempt = 0;
	
	//printf("\n--------- hand shake ---------\n");
	/* Send the handshake packet */
	send_attempt = 0;
	while (send_attempt < MAX_CLI_RETRIES && ack_reply < 0) {
		//printf("rput - main()/send_handshake_packet 1a: send_attempt = %i\n", send_attempt);
		// send the packet
		bytes_trfd = send_handshake_packet(sock_num, &srv_addr);
		//printf("rput - main() 1b: bytes_trfd = %i\n", bytes_trfd);

		// ack_reply address will contain the new port
		ack_reply = recv_ack(sock_num, &srv_addr);
		//printf("rput - main()/send_handshake_packet 1b: ack_reply = %i\n", ack_reply);
		
		send_attempt++;
	}
	if (ack_reply < 0) {
		printf("rput - main()/send_handshake_packet: could not communicate with server\n");
		return -1;
	}
	
	//printf("--------- init ---------\n");
	/* Send the init packet */
	ack_reply = -1;
	send_attempt = 0;
	while (send_attempt < MAX_CLI_RETRIES && ack_reply < 0) {
		//printf("rput - main()/send_init_packet 2a: send_attempt = %i\n", send_attempt);
		// send the packet
		bytes_trfd = send_init_packet(sock_num,  &srv_addr, remote_file_name, file_len);
		//printf("rput - main() 2b: bytes_trfd = %i\n", bytes_trfd);
		
		// wait for ack
		ack_reply = recv_ack(sock_num, &srv_addr);
		//printf("rput - main()/send_init_packet 1b: ack_reply = %i\n", ack_reply);

		send_attempt++;
	}
	if (ack_reply < 0) {
		printf("rput - main()/send_init_packet: could not communicate with server\n");
		return -1;
	}
	
	/* Send the file */
	//printf("--------- file ---------\n");
	send_file(sock_num, &srv_addr, file_ptr, file_len, buff_sz);	
	fclose(file_ptr);
	
	//printf("rput - main() 5: done\n");
	
	return 0;
}

/**
 * Sends the handshake packet.
 * Seq # = 0.
 * Returns the number of bytes transfered.
 **/
int send_handshake_packet(int sock_num, struct sockaddr_in * srv_addr) {
	char *			data_buff;
	unsigned int	seq_num = htonl(HS_SEQ_NUM);
	unsigned int	data_len = sizeof(seq_num);
	int				bytes_trfd = 0;

	/* Create handshake packet */
	// 4 bytes - seq num, always 0
	data_buff = malloc(data_len);
	memcpy(data_buff, &seq_num, data_len);
	bytes_trfd = send_packet(sock_num, data_buff, data_len, srv_addr);
	//printf("rput - send_handshake_packet() 1: bytes_trfd = %i\n", bytes_trfd);
	
	return bytes_trfd;
}

/**
 * Send init packet with filename and length.
 * Wait for ack. 
 * Return new port.
 **/
int send_init_packet(int sock_num, struct sockaddr_in * srv_addr, char * filename, int file_len) {
	char *			data_buff;
	char *			fill_buff;
	int				data_len = 0;
	int				bytes_trfd = 0;
	unsigned short	chk_sum = 0;
	unsigned int	seq_num = INIT_SEQ_NUM;
	
	// construct the init packet
	int filename_len = strlen(filename) + 1;		// add 1 for null terminator
	data_len = sizeof(seq_num) + sizeof(filename_len) + sizeof(file_len) + filename_len + sizeof(chk_sum);
	//printf("rput - send_init_packet() 1: data_len = %i\n", data_len);

	// allocate buffer
	data_buff = malloc(data_len);
	fill_buff = data_buff;
	bzero(data_buff, data_len);
	
	// seq num - 4 bytes
	//printf("rput - send_init_packet() 1: seq_num = %u\n", seq_num);
	seq_num = htonl(seq_num);
	//printf("rput - send_init_packet() 1: htonl(seq_num) = %u\n", seq_num);
	memcpy(fill_buff, &seq_num, sizeof(seq_num));
	fill_buff = fill_buff + sizeof(seq_num);
	
	// filename length - 4 bytes
	//printf("rput - send_init_packet() 2: filename_len = %i\n", filename_len);
	filename_len = htonl(filename_len);
	memcpy(fill_buff, (char *) &filename_len, sizeof(filename_len));
	filename_len = ntohl(filename_len);
	fill_buff = fill_buff + sizeof(filename_len);

	// file length - 4 bytes
	//printf("rput - send_init_packet() 3: file_len = %i\n", file_len);
	file_len = htonl(file_len);
	memcpy(fill_buff, (char *) &file_len, sizeof(file_len));
	file_len = ntohl(file_len);
	//printf("rput - send_init_packet() 3a: sizeof(file_len) = %lu\n", sizeof(file_len));
	fill_buff = fill_buff + sizeof(file_len);

	// file name - variable
	//printf("rput - send_init_packet() 4: filename = '%s'\n", filename);
	//printf("rput - send_init_packet() 4a: filename_len = %i\n", filename_len);
	memcpy(fill_buff, (char *) filename, filename_len);
	fill_buff = fill_buff + filename_len;

	// checksum - 2 bytes
	chk_sum = in_cksum((unsigned short *) data_buff, data_len);
	memcpy(fill_buff, &chk_sum, sizeof(chk_sum));
	//printf("rput - send_init_packet() 5: chk_sum = %hu\n", chk_sum);
	
	//printf("rput - send_init_packet() 6: srv_addr = \n");
	//print_sockaddr_info(*srv_addr);
	
	// send
	bytes_trfd = send_packet(sock_num, data_buff, data_len, srv_addr);
	//printf("rput - send_init_packet() 6: bytes_trfd = %i\n", bytes_trfd);

	free(data_buff);
	
	return bytes_trfd;
}

/**
 * Returns the total number of bytes successfully transfered.
 */
void send_file(int sock_num, struct sockaddr_in * srv_addr, FILE * file_ptr, unsigned int file_len, unsigned int buff_sz) {
	unsigned int	seq_num;
	unsigned int	total_seq_num;
	int				curr_buff_sz = buff_sz;
	char *			data_buff;
	unsigned int	ack_reply = 0;
	unsigned short	current_send_attempt = 0;
		
	/* Setup sequence numbers */
	seq_num = INIT_SEQ_NUM + 1;
	total_seq_num = ((int) ceil((double) file_len / (double) buff_sz)) + seq_num;
	//printf("rput - send_file() 1b: total_seq_num = %i\n", total_seq_num);
	
	/* Send file */
	data_buff = malloc(buff_sz);
	//printf("rput - send_file() : file_len = %i\n", file_len);

	//printf("\n======================\n");
	//printf("rput - send_file() 2a: seq_num = %i\n", seq_num);
	while (current_send_attempt < MAX_CLI_RETRIES && seq_num < total_seq_num) {
		//printf("----------\n");
		//printf("rput - send_file() 3a: current_send_attempt = %i\n", current_send_attempt);

		if (current_send_attempt == 0) {
			// get a new buff of data
			//printf("rput - send_file() 3b: reading in part of file\n");
			//printf("rput - send_file() 3b: buff_sz = %i\n", buff_sz);
			curr_buff_sz = fread(data_buff, sizeof(char), buff_sz, file_ptr);
			file_len = file_len - curr_buff_sz;
			//printf("rput - send_file() 3b: curr_buff_sz = %i\n", curr_buff_sz);
			//printf("rput - send_file() 3b: file_len = %i\n", file_len);
		}

		// send current buffer
		send_file_packet(sock_num, seq_num, srv_addr, data_buff, curr_buff_sz);

		// wait for ack
		ack_reply = recv_ack(sock_num, srv_addr);
		//printf("rput - send_file() 3d: ack_reply = %i\n", ack_reply);

		if (ack_reply == seq_num) {
			// increment seq num
			seq_num++;
			// reset the current send attempt
			current_send_attempt = 0;
		} else {
			current_send_attempt++;
		}
	}
		
	// we were never got an ack back from the last packet
	if (current_send_attempt >= (MAX_CLI_RETRIES-1)) {
		printf("rput - send_file(): never got an ack for seq_num = %u\n", seq_num);
	}
	
	free(data_buff);
}

/**
 * Returns the number of bytes transfered.
 *
 * Looks like:
 * 4 byte		seq num > 1
 * 2 byte		check sum
 * 4 byte		buffer size
 * ? byte		data ( buffer size )
 **/
void send_file_packet(int sock_num, unsigned int seq_num, struct sockaddr_in * srv_addr, char * send_buff, unsigned int buff_sz) {
	char *			data_buff;
	char *			fill_buff;
	char *			chk_sum_ptr;
	unsigned short	chk_sum = 0;
	unsigned int	data_len = sizeof(seq_num) + sizeof(chk_sum) + sizeof(buff_sz) + buff_sz;
	int				bytes_trfd = 0;
	
	//printf("rput - send_file_packet() 1: seq_num = %i\n", seq_num);
	//printf("rput - send_file_packet() 2: data_len = %i\n", data_len);
	
	/* Build packet */
	data_buff = malloc(data_len);
	bzero(data_buff, data_len);
	fill_buff = data_buff;

	// 4 bytes sequence number
	seq_num = htonl(seq_num);			// put in network byte order
	memcpy(fill_buff, &seq_num, sizeof(seq_num));
	seq_num = ntohl(seq_num);
	fill_buff = fill_buff + sizeof(seq_num);

	// 2 byte checksum
	// skip over the checksum for now until we have the whole packet built
	chk_sum_ptr = fill_buff;	// save the location of the check sum
	fill_buff = fill_buff + sizeof(chk_sum);

	// 4 byte buffer size
	//printf("rput - send_file_packet() 2: buff_sz = %i\n", buff_sz);
	buff_sz = htonl(buff_sz);			// put in network byte order
	memcpy(fill_buff, &buff_sz, sizeof(buff_sz));
	buff_sz = ntohl(buff_sz);
	fill_buff = fill_buff + sizeof(buff_sz);

	// data
	memcpy(fill_buff, send_buff, buff_sz);
	
	// now calculate the check sum
	chk_sum = in_cksum((unsigned short *) data_buff,  data_len);
	memcpy(chk_sum_ptr, &chk_sum, sizeof(chk_sum));
	
	/* Send the packet */
	bytes_trfd = send_packet(sock_num, data_buff, data_len, srv_addr);
	//printf("rput - send_file_packet() 2: bytes_trfd = %i\n", bytes_trfd);
	free(data_buff);
}

/**
 * Wait on and receive ack packet.
 * Returns the sequence number received in the ack, -1 if no ack is received
 * before the select call times out or if the ack packet has been
 * corrupted.
 **/
int recv_ack(int sock_num, struct sockaddr_in * srv_addr) {
	char *			data_buff = malloc(MAX_ACK_PKT_SZ);
	unsigned int	seq_num = -1;
	int				bytes_trfd = 0;
	unsigned int	srvaddr_len = sizeof(struct sockaddr);
	
	//printf("rput - recv_ack() 1: waiting for ack\n");
	
	if (select_call(sock_num, CLI_WAIT_SECS, 0) > 0) {
		bytes_trfd = recvfrom(sock_num,  data_buff, MAX_ACK_PKT_SZ, 0, (struct sockaddr *) srv_addr, &srvaddr_len);
		//printf("rput - recv_ack() 2a: bytes_trfd = %i\n", bytes_trfd);
		
		// 4 byte seq num
		memcpy(&seq_num, data_buff, sizeof(seq_num));
		seq_num = ntohl(seq_num);
		//printf("rput - recv_ack() 2b: seq_num = %u\n", seq_num);
	}
	
	free(data_buff);
	
	return seq_num;
}

/**
 * Opens the file pointer and stats the file.
 * Return the size of the file or -1 if the file cannot be opened or the file stats cannot be read.
 **/
int check_file(char * filename, FILE ** file_ptr) {
	int				file_len = -1;
	struct stat		file_stats;
	
	/* Make sure we can open the file or there's not point in doing anything else */
	if (stat(filename,  &file_stats) != 0) {
		perror("could not stat() file");
		return -1;
	}
	// make sure we can open the file
	if ((* file_ptr = fopen(filename, "r")) == NULL)
	{
		perror("could not fopen() file");
		return -1;
	}
	
	// size of file
	file_len = file_stats.st_size;
	
	//printf("rput - check_file() : file_len = %i\n", file_len);
	
	return file_len;
}
