#include <sys/errno.h>
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

  int bind_status = bind(sockfd, res->ai_addr, res->ai_addrlen);
  if(bind_status == -1) {
    int err = errno;
    printf("%s\n", strerror(errno));
    return 2;
  }

  int listen_status = listen(sockfd, 20);
  if(listen_status == -1) {
    int err = errno;
    printf("%s\n", strerror(errno));
    return 3;
  }

  struct sockaddr_storage their_addr;
  int their_addr_size = sizeof their_addr;
  int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, (unsigned int *)&their_addr_size);

  if(new_fd == -1) {
    int err = errno;
    printf("%s\n", strerror(err));
    return 4;
  }
  printf("client connected\n");


  char *msg ="hello client!";
  int sent = send(new_fd, msg, strlen(msg), 0);
  if(sent == -1) {
    int err = errno;
    printf("%s\n", strerror(err));
    return 5;
  }

  char rec[100];
  int received = recv(new_fd, rec, strlen(rec), 0);
  printf("%s\n", rec);

  if(received == -1) {
    int err = errno;
    printf("%s\n", strerror(err));
    return 6;
  } else if (received == 0) {
    printf("Client connection has closed\n");
    return 7;
  }

}
