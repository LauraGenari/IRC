#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <vector>
#include <iostream>

pthread_mutex_t mutex;

typedef struct client{
  int sockfd;
}Client;

std::vector<Client*> clients;

//function that sends a message received by server to all clients
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

void add_client(Client* new_client);
void remove_client(int sockfd);

void *recvmg(void *client_sock);

int main(int argc, char *argv[]) {
  struct sockaddr_in ServerIp;
  pthread_t recvt;
  int sock = 0, client_fd = 0;

  //socket configuration
  ServerIp.sin_family = AF_INET;
  ServerIp.sin_port = htons(atoi(argv[1]));
  ServerIp.sin_addr.s_addr = inet_addr("127.0.0.1");
  
  sock = socket(AF_INET, SOCK_STREAM, 0);

  if (bind(sock, (struct sockaddr *)&ServerIp, sizeof(ServerIp)) == -1){
    perror("bind: ");
    exit(EXIT_FAILURE);
  }
  else
    std::cout << "Server Started" << std::endl;

  //set socket in a passive mode that waits a client conection
  if (listen(sock, 20) == -1){
    perror("listen: ");
    exit(EXIT_FAILURE);
  }

  //adds a client and waits a message
  while (1) {
    if ((client_fd = accept(sock, (struct sockaddr *)NULL, NULL)) < 0){
      perror("accept: ");
    }

    Client* new_client = (Client*) malloc(sizeof(Client));
    new_client->sockfd = client_fd;
    
    add_client(new_client);

    pthread_create(&recvt, NULL, &recvmg, (void*)new_client);
  }
  return 0;
}

//function that adds client
void add_client(Client* new_client){
    pthread_mutex_lock(&mutex);
    clients.push_back(new_client);
    pthread_mutex_unlock(&mutex);
}

//funciton that removes client
void remove_client(int sockfd){
    pthread_mutex_lock(&mutex);
    //Find client to be removed, swap with the end, remove from the end.
    for(int i = 0; i < clients.size(); i++){
      if(clients[i]->sockfd == sockfd){
        Client* c = clients[i];
        clients[i] = clients.back();
        clients.pop_back();
        break;
      }
    }
    pthread_mutex_unlock(&mutex);
}
//function that receives a messagem from client
void *recvmg(void *client_sock) {
  int sock = ((Client*)client_sock)->sockfd;
  char msg[4096] = {0};
  int len;

  while ((len = recv(sock, msg, 4095, 0)) > 0) {
    msg[len] = '\0';
    //hancles /quit
    sendtoall(msg, sock);
  }

  remove_client(sock);
  close(sock);
  return NULL;
}