#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
  struct addrinfo hint, *res, *p;

  if (argc != 2) {
    printf("usage: showip <hostname>");
    return 1;
  }

  memset(&hint, 0, sizeof(hint));
  hint.ai_family = PF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;

  int status = getaddrinfo(argv[1], NULL, &hint, &res);
  if (status != 0) {
    printf("%s\n", gai_strerror(status));
    return 2;
  }

  char ipstr[INET6_ADDRSTRLEN];
  p = res;
  while (p != NULL) {
    void *addr;

    if (p->ai_family == AF_INET) {
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      addr = &(ipv4->sin_addr);
    } else {
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
      addr = &(ipv6->sin6_addr);
    }

    inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
    printf("%s\n", ipstr);
    p = p->ai_next;
  }

  freeaddrinfo(res);
  return 0;
}
