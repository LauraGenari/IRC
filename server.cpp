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

#include "irc.h"

pthread_mutex_t mutex;

typedef struct client{
  int sockfd;
}Client;

std::vector<Client*> clients;

//function that sends a message received by server to all clients
void sendtoall(char *msg, int curr) {
  pthread_mutex_lock(&mutex);
  //Iterate through all clients
  for (auto it = clients.begin(); it != clients.end(); it++) {
    if(DEBUG_MODE) std::cout << (*it)->sockfd << std::endl; 
    int fd = (*it)->sockfd;
    //Send message to client if its not the one who wrote it
    if (fd != curr) {
      if (send(fd, msg, strlen(msg), 0) < 0) {
        perror("send");
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
    IRC::error("bind");
  } 
  else
    std::cout << "Server Started" << std::endl;

  //set socket in a passive mode that waits a client connection
  if (listen(sock, 20) == -1){
    IRC::error("listen");
  }

  //Accept new client and create thread for receiving messages
  while (1) {
    if ((client_fd = accept(sock, (struct sockaddr *)NULL, NULL)) < 0){
      perror("accept");
    }

    Client* new_client = (Client*) malloc(sizeof(Client));
    new_client->sockfd = client_fd;
    
    add_client(new_client);
    pthread_create(&recvt, NULL, &recvmg, (void*)new_client);
    char connection_message[] = "\nServer: A client has joined\n";
    sendtoall(connection_message, client_fd);
  }
  return 0;
}

//function that adds client to list
void add_client(Client* new_client){
    pthread_mutex_lock(&mutex);
    clients.push_back(new_client);
    pthread_mutex_unlock(&mutex);
}

//funciton that removes client from list
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

//function that receives a message from client
void *recvmg(void *client_sock) {
  int sock = ((Client*)client_sock)->sockfd;
  char msg[BUFFER_SIZE] = {0};
  int len;

  //Receive message
  while ((len = recv(sock, msg, BUFFER_SIZE, 0)) > 0) {
    msg[len] = '\0';
    //Handle /quit command
    std::string command = msg;
    if(command.find("/quit") == std::string::npos){
      sendtoall(msg, sock);
    }
    else{
      //Send quit message to all and break while
      if(DEBUG_MODE) sprintf(msg, "Server: %d has quit\n", sock);
      else sprintf(msg, "Server: A client has quit\n");
      sendtoall(msg, sock);
      break;
    }
  }

  //Remove Client, close connection and end thread
  remove_client(sock);
  close(sock);
  return NULL;
}