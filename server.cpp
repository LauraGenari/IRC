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