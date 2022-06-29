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
#include <time.h>
#include <unistd.h>

#define MAX_CONNECTIONS 5
#define MAX_MESSAGE_SIZE 500

int initializeServerSocket(const char* port, struct sockaddr** address) {
  int domain, addressSize, serverfd, yes = 1;
  struct sockaddr_in addressv4;

  addressv4.sin_family = AF_INET;
  addressv4.sin_port = htons(atoi(port));
  addressv4.sin_addr.s_addr = htonl(INADDR_ANY);

  domain = AF_INET;
  addressSize = sizeof(addressv4);
  *address = (struct sockaddr*)&addressv4;

  if ((serverfd = socket(domain, SOCK_DGRAM, IPPROTO_UDP)) == 0) {
    perror("Could not create socket");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes,
                 sizeof(yes)) == -1) {
    perror("Could not set socket option");
    exit(EXIT_FAILURE);
  }

  if (bind(serverfd, *address, addressSize) < 0) {
    perror("Could not bind port to socket");
    exit(EXIT_FAILURE);
  }

  return serverfd;
}

int main(int argc, char const* argv[]) {
  if (argc != 2) {
    return 0;
  }
  struct sockaddr* address;
  int sockfd = initializeServerSocket(argv[1], &address);
  char buffer[MAX_MESSAGE_SIZE] = {0};
  while (1) {
    memset(buffer, 0, sizeof(buffer));
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    int bytesRead =
        recvfrom(sockfd, buffer, MAX_MESSAGE_SIZE - 1, 0,
                 (struct sockaddr*)&client_address, &client_address_len);

    if (bytesRead < 1) {
      close(sockfd);
      break;
    }

    buffer[bytesRead] = '\0';
    printf("%s\n", buffer);

    char* mensagem = "oi tudo bem\n";
    int bytesSent =
        sendto(sockfd, mensagem, strlen(mensagem), 0,
               (struct sockaddr*)&client_address, client_address_len);
    if (bytesSent < 1) {
      close(sockfd);
      break;
    }
  }

  return 0;
}