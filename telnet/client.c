#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "3490"
#define MAXDATASIZE 100

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(void) {
  struct addrinfo hints, *res, *p;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int rv = getaddrinfo(NULL, PORT, &hints, &res);
  if(rv == -1) {
    printf("%s\n", gai_strerror(rv));
    exit(1);
  }

  int sockfd;
  for(p = res; p != NULL; p = p->ai_next){
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if(sockfd == -1) {
      perror("Socket");
      continue;
    }

    if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("Connect");
      continue;
    }

    break;
  }

  if(p == NULL){
    perror("Client failed to connect");
    exit(1);
  }

  char s[INET6_ADDRSTRLEN];
  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
  printf("client: connecting to %s\n", s);

  freeaddrinfo(res);

  char buf[MAXDATASIZE];
  int numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0);
  if(numbytes == -1) {
    perror("recv");
    exit(1);
  }

  buf[numbytes] = '\0';
  printf("Client received: %s\n", buf);
  close(sockfd);
  return 0;
}
