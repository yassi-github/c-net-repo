#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> // getaddrinfo
#include <time.h> // timespec,nanosleep
#include <unistd.h> // read,write,close

// = 1 mili sec in nano unit
#define	MSEC 1000000
// default TCP dest port
#define	TCP_PORT 20000 

static const struct timespec sleep_nanosec = { .tv_sec = (long int) 0, .tv_nsec = (long int) 10 * MSEC };

void err_msg(char *msg)
{
  perror(msg);
  exit(1);
}

//
// connect server
// Input  : hostname and port number
// Output : socket for listen
//
int connect_server(char *hostname, int port_no)
{
  struct addrinfo hints = { .ai_family = AF_INET, .ai_socktype = SOCK_STREAM };
  struct addrinfo *result;
  char port_str[10];
  int sockfd;
  int ipaddr;

  snprintf(port_str,10, "%d", port_no);
  if (getaddrinfo(hostname, port_str, &hints, &result)) {
    fprintf(stderr, "%s: cannot get IP address\n", hostname);
    exit(1);
  }
  sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (sockfd == -1) {
    err_msg("client: can't open datastream socket");
  }
  ipaddr = ((struct sockaddr_in *)(result->ai_addr))->sin_addr.s_addr;
  printf("%d.%d.%d.%d (%d) にアクセスを試みます．\n",
	 ipaddr & 0xff, (ipaddr >> 8) & 0xff, (ipaddr >> 16) & 0xff, (ipaddr >> 24) & 0xff,
	 port_no);
  if (connect(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
    err_msg("client: can't connect to server");
  }
  printf("サーバに接続しました．適当な文字を入力してください．\n"
	 "Ctrl-A を入力すると終了します．\n");
  return sockfd;
}


int main(int argc, char *argv[])
{
  int port_no;
  int sockfd;
  int rflag;        // if rflag is zero then send is required else input is required 
  int wflag;        // if wflag is zero then reseive is required else output is required 
  int endflag;      // typed or received ^A
  char sch, rch;

  if (argc < 2) {
    fprintf(stderr,"Usage: client serv_addr [port_no]\n");
    exit(1);
  }
  port_no = (argc > 2) ? atoi(argv[2]) : TCP_PORT;
  sockfd = connect_server(argv[1], port_no);
  
  fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
  fcntl(fileno(stdout), F_SETFL, O_NONBLOCK);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  rflag = 1; wflag = 0;
  while (1) {
    if (rflag == 1) {
      if (read(fileno(stdin), &sch, 1) == 1) {
	if (sch == '\001') {
	  endflag = 0;
	  break;	// ^A
	}
	rflag = 0;
      }
    }
    if (rflag == 0) {
      if (write(sockfd, &sch, 1) == 1) {
	rflag = 1;
      }
    }
    if (wflag == 0) {
      if (read(sockfd, &rch, 1) == 1) {
	if (rch == '\001') {
	  endflag = 1;
	  break;	// ^A
	}
	wflag = 1;
      }
    }
    if (wflag == 1) {
      if (write(fileno(stdout), &rch, 1) == 1) {
	wflag = 0;
      }
    }
    // For save CPU usage 
    nanosleep(&sleep_nanosec, NULL);
  }
  if (endflag == 0) {
    while (write(sockfd, &sch, 1) != 1);
  }
  ssize_t err = write(fileno(stdout), "\nEnd.\n", strlen("\nEnd.\n")+1);
  if (err == -1) {
    err_msg("client: cannot write to stdout");
  }

  close(sockfd);
  return 0;
}
