#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "9034";

int get_listener_socket(void) {
  int listener;
  int yes = 1; // setsockopt reuse addr

  struct addrinfo hints, *ai, *p;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int status = getaddrinfo(NULL, "9034", &hints, &ai);
  if (status != 0) {
    fprintf(stderr, "server: %s\n", gai_strerror(status));
    exit(1);
  }

  for (p = ai; p != NULL; p = p->ai_next) {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener < 0) {
      continue;
    }

    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
      close(listener);
      continue;
    }

    break;
  }

  if (p == NULL) {
    // did not bind
    return -1;
  }

  if (listen(listener, 10) == -1) {
    return -1;
  }

  return listener;
}

void add_to_pollfds(struct pollfd *pfds, int *size, int *count, int newfd) {
  if (*count == *size) {
    // resize array
    *size = *size * 2;
    pfds = realloc(pfds, sizeof(struct pollfd) * (*size));

    if (pfds == NULL) {
      perror("out of memory");
      exit(1);
    }
  }

  pfds[*count].fd = newfd;
  pfds[*count].events = POLLIN;

  (*count)++;
}

void remove_from_pollfds(struct pollfd *pfds, int *count, int index) {
  pfds[index] = pfds[*count - 1];
  (*count)--;
}

int main() {
  int listener;
  int newfd;
  struct sockaddr_storage remoteaddr;
  socklen_t addrlen;

  char buf[256];

  int fd_count = 0;
  int fd_size = 5;
  struct pollfd *pollfds = malloc(sizeof(struct pollfd) * fd_size);

  listener = get_listener_socket();
  if (listener == -1) {
    fprintf(stderr, "error getting listening socket\n");
    exit(1);
  }

  pollfds[0].fd = listener;
  pollfds[0].events = POLLIN;
  fd_count++;

  while (true) {
    int pollcount = poll(pollfds, fd_count, -1);

    if (pollcount == -1) {
      fprintf(stderr, "poll\n");
      exit(1);
    }

    for (int i = 0; i < fd_count; i++) {
      if (pollfds[i].revents & POLLIN) {
        if (pollfds[i].fd == listener) {
          addrlen = sizeof(remoteaddr);
          newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

          if (newfd == -1) {
            perror("accept");
            exit(1);
          } else {
            add_to_pollfds(pollfds, &fd_size, &fd_count, newfd);
          }
        } else {
          int nbytes = recv(pollfds[i].fd, buf, sizeof(buf), 0);
          int sender_fd = pollfds[i].fd;

          if (nbytes <= 0) {
            if (nbytes < 0) {
              perror("recv failed");
            } else if (nbytes == 0) {
              // client connection closed
              printf("Client %d has left\n", pollfds[i].fd);
            }
            close(pollfds[i].fd);
            remove_from_pollfds(pollfds, &fd_count, i);
          } else {
            for(int j = 0; j < fd_count; j++) {
              int dest_fd = pollfds[j].fd;
              if(dest_fd != listener && dest_fd != sender_fd) {
                int status = send(dest_fd, buf, nbytes, 0);
                if(status == -1) {
                  perror("failed to send");
                  exit(1);
                }
              }
            }
          }
        }
      }
    }
  }
  return 0;
}
