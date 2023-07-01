#include "sockutil.h"

#include <arpa/inet.h>   // htonl,htons
#include <netdb.h>       // getaddrinfo
#include <netinet/in.h>  // htonl,htons
#include <stdio.h>       // snprintf
#include <stdlib.h>      // exit
#include <sys/socket.h>  // socket,setsockopt,bind,listen
#include <sys/types.h>

#include "errorutil.h"  // error_msg

static const int sockopt_enabled = 1;

// do socket,bind,listen.
// returns opened socket_id.
int init_socket(int port_no) {
  // socket
  int sockid = socket(AF_INET, SOCK_STREAM, 0);
  if (sockid < 0) {
    error_msg("server: cannot open datastream socket");
  }
  // set SO_REUSEADDR to be available same addr even less than 2 mins from
  // closed
  if (setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR, &sockopt_enabled,
                 sizeof(sockopt_enabled)) < 0) {
    error_msg("server: failed to set socket option SO_REUSEADDR");
  }

  // bind
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port_no);
  server_address.sin_addr.s_addr = inet_addr("0.0.0.0");

  if (bind(sockid, (struct sockaddr *)&server_address, sizeof(server_address)) <
      0) {
    error_msg("server: cannot bind local address");
  }

  // listen
  if (listen(sockid, 5) == -1) {
    error_msg("server: listen failed");
  }

  return sockid;
}

int accept_socket(int socket_id) {
  struct sockaddr_in client_address;
  socklen_t client_address_len = sizeof(client_address);
  return accept(socket_id, (struct sockaddr *)&client_address,
                &client_address_len);
}

//
// connect server
// Input  : hostname and port number
// Output : socket for listen
//
int connect_server(const char *hostname, int port_no) {
  char port_str[10];
  snprintf(port_str, 10, "%d", port_no);

  struct addrinfo hints = {.ai_family = AF_INET, .ai_socktype = SOCK_STREAM};
  struct addrinfo *result;
  if (getaddrinfo(hostname, port_str, &hints, &result)) {
    fprintf(stderr, "%s: cannot get IP address\n", hostname);
    exit(1);
  }

  int sockfd =
      socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (sockfd == -1) {
    error_msg("client: can't open datastream socket");
  }

  int ipaddr = ((struct sockaddr_in *)(result->ai_addr))->sin_addr.s_addr;

  printf("%d.%d.%d.%d (%d) にアクセスを試みます．\n", ipaddr & 0xff,
         (ipaddr >> 8) & 0xff, (ipaddr >> 16) & 0xff, (ipaddr >> 24) & 0xff,
         port_no);

  if (connect(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
    error_msg("client: can't connect to server");
  }

  printf(
      "サーバに接続しました．適当な文字を入力してください．\n"
      "Ctrl-A を入力すると終了します．\n");

  return sockfd;
}
