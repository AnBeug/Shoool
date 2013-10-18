Stop and Wait
=====
This client-server uses UDP sockets to communicate. Using rput, a file can be transfered from the client to the server.

This implementation uses a stop-and-wait protocol for sending packets from the client to the server. The client sends a packet and waits for the server to acknowledge before sending the next packet.