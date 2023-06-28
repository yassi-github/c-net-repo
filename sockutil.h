// #define _XOPEN_SOURCE 500

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "utils.h"

static const int sockopt_enabled = 1;

// do socket,bind,listen.
// returns opened socket_id.
extern int init_socket(int port_no) {
  // socket
  int sockid = socket(AF_INET, SOCK_STREAM, 0);
  if (sockid < 0) {
    err_msg("server: cannot open datastream socket");
  }
  // set SO_REUSEADDR to be available same addr even less than 2 mins from
  // closed
  if (setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR, &sockopt_enabled,
                 sizeof(sockopt_enabled)) < 0) {
    err_msg("server: failed to set socket option SO_REUSEADDR");
  }

  // bind
  struct sockaddr_in server_address = {.sin_family = AF_INET,
                                       .sin_addr.s_addr = htonl(INADDR_ANY),
                                       .sin_port = htons(port_no)};
  if (bind(sockid, (struct sockaddr *)&server_address, sizeof(server_address)) <
      0) {
    err_msg("server: cannot bind local address");
  }

  // listen
  if (listen(sockid, 5) == -1) {
    err_msg("server: listen failed");
  }

  return sockid;
}

extern int accept_socket(int socket_id) {
  struct sockaddr_in client_address;
  socklen_t client_address_len = sizeof(client_address);
  return accept(socket_id, (struct sockaddr *)&client_address, &client_address_len);
}
