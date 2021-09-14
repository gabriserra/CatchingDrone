//-----------------------------------------------------------------------------
// UDP_H: MANAGE EASILY UDP PACKET SEND
//-----------------------------------------------------------------------------

#ifndef UDP_H
#define UDP_H

#define SP_DIM 3	// Dimension of space in which we work

//--------------------------------
// PUBLIC: UDP INIT AND SEND
//--------------------------------

// Create an UDP sock and connect to client/server with ip and port provided
int udp_init(char* ip_address, unsigned short udp_port);

// Send graphic data to connected UDP sock, return the num of byte sent or -1
int udp_grap_send(int sock, float* d_lin_pos, float* d_ang_pos, float* d_lin_vel, float* b_pos, float* b_vel);

#endif


