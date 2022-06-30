#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_MESSAGE_SIZE 500
#define MAX_CONECTIONS 15

int sockfd;
int numberThreads = 0;

enum COMMAND_TYPE {
  REQ_ADD = 1,
  REQ_REM = 2,
  REQ_INF = 5,
};

void *ThreadMain(void *threadArgs);

struct ThreadArgs {
  socklen_t clientAdressSize;
  struct sockaddr_in clientAdress;
  char buffer[MAX_MESSAGE_SIZE];
};

int initializeServerSocket(const char *port, struct sockaddr **address) {
  int domain, addressSize, serverfd, yes = 1;
  struct sockaddr_in addressv4;

  addressv4.sin_family = AF_INET;
  addressv4.sin_port = htons(atoi(port));
  addressv4.sin_addr.s_addr = htonl(INADDR_ANY);

  domain = AF_INET;
  addressSize = sizeof(addressv4);
  *address = (struct sockaddr *)&addressv4;

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

typedef struct {
  int idMessage;
  int idOrigem;
  int idDestino;
  int idPayload[MAX_CONECTIONS];
} Command;

Command interpretCommand(char* buffer) {
  char *commandToken = strtok(buffer, " ");
  int commandSent = atoi(commandToken);
  Command command;

  switch (commandSent) {
    case REQ_ADD:
      command.idMessage = REQ_ADD;
      break;
    case REQ_REM:
      command.idMessage = REQ_REM;
      commandToken = strtok(NULL, " ");
      command.idOrigem = atoi(commandToken);
      break;
    case REQ_INF:
      command.idMessage = REQ_INF;
      commandToken = strtok(NULL, " ");
      command.idOrigem = atoi(commandToken);
      commandToken = strtok(NULL, " ");
      command.idDestino = atoi(commandToken);
      break;
    default:
      break;
  }
  return command;
}

void mountCommand(Command command, char *buffer) {
    strcat(buffer,command.idMessage);
    if(command.idOrigem!=-1){
      strcat(buffer," ");
      strcat(buffer,command.idOrigem);
    }
    if(command.idDestino!=-1){
      strcat(buffer," ");
      strcat(buffer,command.idDestino);
    }
    for (int i = 0; i < command.idPayload[i]!=-1; i++)
    {
      strcat(buffer," ");
      strcat(buffer,command.idPayload[i]);
    }
}

int executeCommand(Command command, int sockfd, char* commandOutput) {
  switch (command.idMessage) {
    case REQ_ADD:
      printf("Adicionando novo cliente\n");
      break;
    case REQ_REM:
      printf("Removendo cliente\n");
      break;
    case REQ_INF:
     printf("Informando cliente\n");
      break;
  }
  return 0;
}

void *ThreadMain(void *threadArgs) {
  struct ThreadArgs *args = (struct ThreadArgs *)threadArgs;
  char *buffer = args->buffer;
  int byteSent =
      sendto(sockfd, buffer, strlen(buffer), 0,
             (struct sockaddr *)&args->clientAdress, args->clientAdressSize);
  if (byteSent < 0) {
    perror("Could not send message to client");
    exit(EXIT_FAILURE);
  }
  printf("Sent message to client: %s\n", buffer);
  free(args);
  return NULL;
}

int main(int argc, char const *argv[]) {
  if (argc != 2) {
    return 0;
  }
  struct sockaddr *address;
  sockfd = initializeServerSocket(argv[1], &address);

  pthread_t threads[MAX_CONECTIONS];

  while (1) {
    struct ThreadArgs *args =
        (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    args->clientAdressSize = sizeof(struct sockaddr_in);
    int byteReceived = recvfrom(sockfd, args->buffer, MAX_MESSAGE_SIZE, 0,
                                (struct sockaddr *)&args->clientAdress,
                                &args->clientAdressSize);
    if (byteReceived < 0) {
      perror("Could not receive message from client");
      exit(EXIT_FAILURE);
    }
    printf("Received message from client: %s\n", args->buffer);

    Command command = interpretCommand(args->buffer);
    executeCommand(command, sockfd, args->buffer);

    int threadResponse =
        pthread_create(&threads[numberThreads], NULL, ThreadMain, (void *)args);
    if (threadResponse) {
      printf("Error creating thread\n");
      exit(EXIT_FAILURE);
    } else {
      numberThreads++;
    }
  }
  return 0;
}