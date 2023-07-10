#ifndef _MY_SOCKUTIL_H
#define _MY_SOCKUTIL_H 1

#include "errorutil.h"  // error

// do socket,bind,listen.
// we should close socket_fd later.
error socket_listen(int *socket_fd, int port_no);

error socket_accept(int *socket_fd, int socket_id);

// connect server
// Input  : connected socket to return, hostname, and port number
// Output : error
//
// we should close socket_fd later.
error socket_connect(int *socket_fd, const char *hostname, int port_no);

#endif
