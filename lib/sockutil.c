#include "sockutil.h"

#include <arpa/inet.h>   // htonl,htons
#include <netdb.h>       // getaddrinfo
#include <netinet/in.h>  // htonl,htons
#include <stdio.h>       // snprintf
#include <stdlib.h>      // exit
#include <sys/socket.h>  // socket,setsockopt,bind,listen
#include <sys/types.h>

#include "errorutil.h"  // error_exit

static const int sockopt_enabled = 1;

// internal error messages
struct _error_msg_list {
  const char *error_open_socket;
  const char *error_setsockopt;
  const char *error_bind;
  const char *error_listen;
  const char *error_accept;
  const char *error_connect;
};
static const struct _error_msg_list error_msg_list = {
    .error_open_socket = "cannot open datastream socket",
    .error_setsockopt = "failed to set socket option SO_REUSEADDR",
    .error_bind = "cannot bind local address",
    .error_listen = "listen failed",
    .error_accept = "failed to accept connection",
    .error_connect = "can't connect to server",
};

// do socket,bind,listen.
// returns opened socket_id.
error socket_listen(int *socket_fd, int port_no) {
  // socket
  int sockid = socket(AF_INET, SOCK_STREAM, 0);
  if (sockid < 0) {
    return error_new(error_msg_list.error_open_socket);
  }
  // set SO_REUSEADDR to be available same addr even less than 2 mins from
  // closed
  if (setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR, &sockopt_enabled,
                 sizeof(sockopt_enabled)) < 0) {
    return error_new(error_msg_list.error_setsockopt);
  }

  // bind
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port_no);
  server_address.sin_addr.s_addr = inet_addr("0.0.0.0");

  if (bind(sockid, (struct sockaddr *)&server_address, sizeof(server_address)) <
      0) {
    return error_new(error_msg_list.error_bind);
  }

  // listen
  if (listen(sockid, 5) == -1) {
    return error_new(error_msg_list.error_listen);
  }

  *socket_fd = sockid;
  return NULL;
}

error socket_accept(int *socket_fd, int socket_id) {
  struct sockaddr_in client_address;
  socklen_t client_address_len = sizeof(client_address);
  int socketid = accept(socket_id, (struct sockaddr *)&client_address,
                        &client_address_len);
  if (socketid == -1) {
    return error_new(error_msg_list.error_accept);
  }
  *socket_fd = socketid;
  return NULL;
}

//
// connect server
// Input  : connected socket to return, hostname, and port number
// Output : error
//
error socket_connect(int *socket_fd, const char *hostname, int port_no) {
  char port_str[10];
  snprintf(port_str, 10, "%d", port_no);

  struct addrinfo hints = {.ai_family = AF_INET, .ai_socktype = SOCK_STREAM};
  struct addrinfo *result;
  if (getaddrinfo(hostname, port_str, &hints, &result)) {
    return error_new("cannot get IP address");
  }

  int sockfd =
      socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (sockfd == -1) {
    return error_new(error_msg_list.error_open_socket);
  }

  // int ipaddr = ((struct sockaddr_in *)(result->ai_addr))->sin_addr.s_addr;

  if (connect(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
    return error_new(error_msg_list.error_connect);
  }

  *socket_fd = sockfd;
  return NULL;
}
