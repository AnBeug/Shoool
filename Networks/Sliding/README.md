Sliding Windows
==========
This client-server uses UDP sockets to communicate. Using rput, a file can be transfered from the client to the server.

The client and server use a sliding window protocol to send the packets and acknowledge the packets. The client sends some set number of packets until stoping and waiting for acknowledgement from the server. Once the server sends acknowledgement for a packet, the client slides over its window of packets it is currently sending. There can only be so many packets outstanding and unacknowledged at a time.