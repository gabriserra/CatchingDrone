#include "udp.h"
#include <string.h>
#include <arpa/inet.h>

//--------------------------------
// PRIVATE: UDP MANAGMENT FUNCTIONS
//--------------------------------

// ---
// Creates a client IPv4 UDP socket and return a descriptor or -1 otherwise
// return: int - socket descriptor in case of success, -1 otherwise
// ---
static int udp_socket() {
    return socket(AF_INET, SOCK_DGRAM, 0);
}

// ---
// Connect an UDP sock to ip and port, return 0 if success, -1 otherwise
// int sock: socket descriptor identifier
// char* ip_address: ip address of the recipient
// unsigned short udp_port: port number of the recipient
// return: int - 0 in case of success, -1 otherwise
// ---
static int udp_connect(int sock, char* ip_address, unsigned short udp_port) {
    struct sockaddr_in recipient;
    
    // set the sockaddr struct with ip and port num
    memset(&recipient, 0, sizeof(recipient));
    recipient.sin_family = AF_INET;
    recipient.sin_port = htons(udp_port);
    inet_pton(AF_INET, ip_address, &recipient.sin_addr);
    
    // connect to the recipient
    return connect(sock, (struct sockaddr*)&recipient, sizeof(recipient)); 
}

// ---
// Send buffer to a connected sock, return the num of byte sent or -1 otherwise
// int sock: socket descriptor identifier
// void* buffer: pointer to data structure to send
// size_t length: number of byte to send
// return: int - num of byte sent in case of success, -1 otherwise
// ---
static int udp_send(int sock, void* buffer, size_t length) {
    return send(sock, buffer, length, 0);
}

//--------------------------------
// PUBLIC: UDP INIT AND SEND
//--------------------------------

// ---
// Create an UDP sock and connect to client/server with ip and port provided
// char* ip_address: ip address of the recipient
// unsigned short udp_port: port number of the recipient
// return: int - socket descriptor in case of success, -1 otherwise
// ---
int udp_init(char* ip_address, unsigned short udp_port) {
    int     sock = udp_socket();

    // check if socket has not been initialized
	if(sock < 0) 
		return -1;
        
    // check if socket has not been connected
    if(udp_connect(sock, ip_address, udp_port))
        return -1;
	
    return sock;
}

// ---
// Send graphic data to connected UDP sock, return the num of byte sent or -1
// int sock: socket descriptor identifier
// float* d_lin_pos: pointer to Vector[3] that contains drone lin position
// float* d_ang_pos: pointer to Vector[3] that contains drone ang position
// float* b_pos: pointer to Vector[3] that contains ball lin position
// return: int - num of byte sent in case of success, -1 otherwise
// ---
int udp_grap_send(int sock, float* d_lin_pos, float* d_ang_pos, float* b_pos) {
    struct udp_graph_data {
	    float   d_lin_pos[SP_DIM]; 	// drone linear position
	    float   d_ang_pos[SP_DIM]; 	// drone angular position
	    float   b_pos[SP_DIM]; 		// ball linear position
    }; 
    
    int     i;                      // array index [0-SP_DIM]
    struct  udp_graph_data data;    // data to be sended
    
    for(i = 0; i < SP_DIM; i++) {
        data.d_lin_pos[i] = d_lin_pos[i];
        data.d_ang_pos[i] = d_ang_pos[i];
        data.b_pos[i] = b_pos[i];
    }

	return udp_send(sock, &data, sizeof(struct udp_graph_data));	
}
