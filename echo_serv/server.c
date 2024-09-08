#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int main(void) {
  struct addrinfo hints, *res;

  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = PF_UNSPEC;

  int status = getaddrinfo(NULL, "8080", &hints, &res);
  if(status != 0) {
    printf("%s\n", gai_strerror(status));
    return 1;
  }

  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  bind(sockfd, res->ai_addr, res->ai_addrlen);
}
