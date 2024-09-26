#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT "3490"
#define BACKLOG 10

void sigchild_handler(int s) {
  int saved_errno = errno;
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
  errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(void) {
  struct addrinfo hints, *res, *p;

  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  int rv = getaddrinfo(NULL, PORT, &hints, &res);

  if (rv != 0) {
    printf("%s\n", gai_strerror(rv));
    return 1;
  }

  int sockfd, yes = 1;
  for(p = res; p != NULL; p = p->ai_next){
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1) {
      perror("error creating socket fd");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("set sock opt");
      exit(1);
    }

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
      close(sockfd);
      perror("error binding socket");
      continue;
    }

    break;
  }
  freeaddrinfo(res);

  if (p == NULL) {
    printf("Server failed to bind\n");
    exit(1);
  }

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  struct sigaction sa;
  sa.sa_handler = sigchild_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if(sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  printf("Server waiting for connections ... \n");

  struct sockaddr_storage their_addr;
  socklen_t their_addr_size;
  int new_fd;
  char s[INET6_ADDRSTRLEN];
  while(1) {
    their_addr_size = sizeof(their_addr);
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &their_addr_size);
    if(new_fd == -1) {
      perror("accept");
      continue;
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof(s));
    printf("Server got connection from: %s\n", s);
    
    if(!fork()) {
      close(sockfd);
      if(send(new_fd, "Hello World!", 13, 0) == -1) {
	perror("send");
      }
      close(new_fd);
      exit(0);
    }
    close(new_fd);
  }

  return 0;
}
