/*#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>  //FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "irc.h"

struct arg_struct {
  pthread_mutex_t mutex;
  int* clients;
  int n;
  int clientSock;
};

void sendToAll(char* msg, int curr, pthread_mutex_t mutex, int* clients, int n) {
  pthread_mutex_lock(&mutex);
  for (int i = 0; i < n; i++) {
    if (clients[i] != curr) {
        printf("MESSAGE: %s\n", msg);
      if (send(clients[i], msg, strlen(msg), 0) < 0) {
        printf("ERROR at sending to client %d\n", clients[i]);
        continue;
      }
    }
  }
  pthread_mutex_unlock(&mutex);
}

void* receiveMsg(void* arguments) {
  struct arg_struct args = *((struct arg_struct*)arguments);
  char message[BUFFER_SIZE];
  int len;
  while ((len = recv(args.clientSock, message, BUFFER_SIZE, 0) > 0)) {
    message[len] = '\0';
    sendToAll(message, args.clientSock, args.mutex, args.clients, args.n);
  }
}

int main(int argc, char const* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }
  int port = atoi(argv[1]);
  struct sockaddr_in serverIp;
  pthread_t recvt;
  pthread_mutex_t mutex;
  int clients[20];
  int n = 0, sockfd, clientSock = 0;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd <= 0) error("socket: ");

  serverIp.sin_family = AF_INET;
  serverIp.sin_addr.s_addr = inet_addr("127.0.0.1");
  serverIp.sin_port = htons(port);

  if (bind(sockfd, (struct sockaddr*)&serverIp, sizeof(serverIp))< 0 ) {
    error("Cannot bind, error!\n");
  } else {
    printf("Server started!\nListening at port %d\n", port);
  }

  if (listen(sockfd, 20) == -1) error("Listening falied\n");
  while (1) {
    if ((clientSock = accept(sockfd, (struct sockaddr*)NULL, NULL)) < 0)
      error("accept failed  \n");

    struct arg_struct args;
    args.mutex = mutex;
    args.clients = clients;
    args.n = n;
    args.clientSock = clientSock;
    pthread_mutex_lock(&args.mutex);
    clients[n] = clientSock;
    n++;
    // creating a thread for each client
    pthread_create(&recvt, NULL, (void*)receiveMsg, &args);
    pthread_mutex_unlock(&args.mutex);
  }

  return 0;
}*/


#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <vector>
#include <iostream>

pthread_mutex_t mutex;

typedef struct client{
  int sockfd;
}Client;

std::vector<Client*> clients;

void sendtoall(char *msg, int curr) {
  pthread_mutex_lock(&mutex);
  for (auto it = clients.begin(); it != clients.end(); it++) {
    std::cout << (*it)->sockfd << std::endl;
    int fd = (*it)->sockfd;
    if (fd != curr) {
      if (send(fd, msg, strlen(msg), 0) < 0) {
        perror("send: ");
        continue;
      }
    }
  }
  pthread_mutex_unlock(&mutex);
}

void *recvmg(void *client_sock) {
  int sock = ((Client*)client_sock)->sockfd;
  char msg[4096] = {0};
  int len;

  while ((len = recv(sock, msg, 4095, 0)) > 0) {
    msg[len] = '\0';
    sendtoall(msg, sock);
  }

}

int main(int argc, char *argv[]) {
  struct sockaddr_in ServerIp;
  pthread_t recvt;
  int sock = 0, Client_sock = 0;

  ServerIp.sin_family = AF_INET;
  ServerIp.sin_port = htons(atoi(argv[1]));
  ServerIp.sin_addr.s_addr = inet_addr("127.0.0.1");
  
  sock = socket(AF_INET, SOCK_STREAM, 0);

  if (bind(sock, (struct sockaddr *)&ServerIp, sizeof(ServerIp)) == -1)
    perror("bind: ");
  else
    std::cout << "Server Started" << std::endl;

  if (listen(sock, 20) == -1) 
    perror("listen: ");

  while (1) {
    if ((Client_sock = accept(sock, (struct sockaddr *)NULL, NULL)) < 0)
      perror("accept: ");

    pthread_mutex_lock(&mutex);
    //clients[n] = Client_sock;
    Client* new_client = (Client*) malloc(sizeof(Client));
    new_client->sockfd = Client_sock;
    clients.push_back(new_client);
    // creating a thread for each client
    pthread_create(&recvt, NULL, recvmg, &Client_sock);
    pthread_mutex_unlock(&mutex);
  }
  return 0;
}