#include <sys/errno.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>

int main(void) {
  struct addrinfo hints, *res;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  int status = getaddrinfo("www.example.com", "3490", &hints, &res);
  if(status != 0){
    printf("%s\n", gai_strerror(status));
    return 1;
  }

  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if(sockfd == -1) {
    printf("Error creating socket\n");
    return 2;
  }

  int con_status = connect(sockfd, res->ai_addr, res->ai_addrlen);
  if(con_status == -1) {
    int err = errno;
    printf("%s\n", strerror(err));
    return 3;
  }

}
