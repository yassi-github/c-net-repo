#ifndef _MY_SOCKUTILS_H
#define _MY_SOCKUTILS_H 1

// do socket,bind,listen.
// returns opened socket_id.
int init_socket(int port_no);

int accept_socket(int socket_id);

// connect server
// Input  : hostname and port number
// Output : socket for listen
//
int connect_server(const char *hostname, int port_no);

#endif
