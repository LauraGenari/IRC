#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <unordered_map>
#include <vector>

#include "irc.h"

using namespace std;

pthread_mutex_t mutex;

typedef struct client {
  int sockfd;
  string nick;
  pthread_t tid;
  bool isMuted;
  string currChanelName;
} Client;

typedef struct channel{
  unordered_map<int, Client*> clients;
  int id;
  string name;
  int adm; //admin's sockfd 
} Channel;

struct tinfo {
  int fd;
  char* msg;
};

// Hashmap for finding clients via its socket file descriptor
// Key: client->sockfd | Value: client
unordered_map<int, Client*> clients = unordered_map<int, Client*>();

// Channel Hashmap
// Key: client->currChanelName | Value: channel
unordered_map<string, Channel*> channels = unordered_map<string, Channel*>();

void add_client(Client* new_client);
void remove_client(int sockfd);
void sendtoall(char* msg, int curr);
void* send_client_msg(void* sockfd_msg);
void* recvmg(void* client_sock);

int main(int argc, char* argv[]) {
  struct sockaddr_in ServerIp;
  pthread_t recvt;

  int sock = 0, client_fd = 0;

  // socket configuration
  ServerIp.sin_family = AF_INET;
  ServerIp.sin_port = htons(atoi(argv[1]));
  ServerIp.sin_addr.s_addr = inet_addr("127.0.0.1");

  sock = socket(AF_INET, SOCK_STREAM, 0);
  int option = 1;
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  if (bind(sock, (struct sockaddr*)&ServerIp, sizeof(ServerIp)) == -1) {
    IRC::error("bind");
  } else
    cout << "Server Started" << endl;

  // set socket in a passive mode that waits a client connection
  if (listen(sock, 20) == -1) {
    IRC::error("listen");
  }

  // Accept new client and create thread for receiving messages
  while (1) {
    if ((client_fd = accept(sock, (struct sockaddr*)NULL, NULL)) < 0) {
      perror("accept");
    }
    // Create client struct in memory
    Client* new_client = (Client*)malloc(sizeof(Client));
    new_client->sockfd = client_fd;
    new_client->isMuted = false;
    

    // Create thread for receiving client's messages
    pthread_create(&recvt, NULL, &recvmg, (void*)new_client);

    // Add to hashmap and vector
    add_client(new_client);

    // Assign thread_id to client
    new_client->tid = recvt;

    // Send connection message to all clients on server
    
  }
  return 0;
}

/*===== Map Operations =====*/

// function that adds client to map
void add_client(Client* new_client) {
  pthread_mutex_lock(&mutex);
  clients[new_client->sockfd] = new_client;
  pthread_mutex_unlock(&mutex);
}

// funciton that removes client from map
void remove_client(int sockfd) {
  pthread_mutex_lock(&mutex);
  // Find client to be removed, swap with the end, remove from the end.
  auto c = clients.find(sockfd);
  if (c != clients.end()) {
    clients.erase(c);
    close(sockfd);
  }
  pthread_mutex_unlock(&mutex);
}

/*===== Server Commons =====*/

// function that sends a message received by server to all clients
void sendtoall(char* msg, int curr) {
  pthread_mutex_lock(&mutex);

  pthread_t send_msgt;

  // Iterate through all clients
  for (auto it = clients.begin(); it != clients.end(); it++) {
    if (DEBUG_MODE) cout << it->first << endl;
    int fd = it->first;
    // Send message to client if its not the one who wrote it
    struct tinfo* info = new struct tinfo;
    info->fd = fd;
    info->msg = msg;
    pthread_create(&send_msgt, NULL, &send_client_msg, (void*)info);
  }
  pthread_mutex_unlock(&mutex);
}

/*===== Thread Funcs =====*/

// function that receives a message from client
void* recvmg(void* client_sock) {
  Client* client = (Client*)client_sock;
  int sock = client->sockfd;
  char msg[BUFFER_SIZE] = {0};
  int len;

  // Receive client name and client channel
  if (recv(sock, msg, BUFFER_SIZE, 0) > 0){
    int pos = 0;
    string temp = msg;
    pos = temp.find("$");
    if(pos != std::string::npos){
      client->nick = temp.substr(0, pos).c_str();
      client->currChanelName = temp.substr(pos, temp.length()).c_str();
    }
    
  }
  else {
    // Remove Client, close connection and end thread
    remove_client(sock);
    return NULL;
  }
  
  //Send join message to all clients
  char* connection_message = new char[32];
  sprintf(connection_message, "\nServer: %s has joined\n", client->nick.c_str());
  sendtoall(connection_message, sock);

  // Receive message
  while ((len = recv(sock, msg, BUFFER_SIZE, 0)) > 0) {
    msg[len] = '\0';
    // Handle /quit command
    string command = msg;
    if (command.find("/quit\n") == string::npos &&
        command.find("/ping\n") == string::npos) {
      sendtoall(msg, sock);
    } else if (command.find("/quit\n") != string::npos) {
      // break while
      break;
    } else if (command.find("/ping\n") != string::npos) {
      // Handle ping message
      struct tinfo* pongMsg = new struct tinfo;
      pongMsg->fd = sock;
      char pong[] = "Server: pong\n";
      pongMsg->msg = pong;
      // Sends message only to client who sent ping
      send_client_msg((void*)pongMsg);
    }
  }
  // Send disconnect message to all
  char* disconnect_msg = new char[32];
  sprintf(disconnect_msg, "Server: %s has quit\n", client->nick.c_str());
  sendtoall(disconnect_msg, sock);
  // Remove Client, close connection and end thread
  remove_client(sock);
  return NULL;
}

//Function that sends message to client
void* send_client_msg(void* sockfd_msg) {
  // Get params
  int fd = ((struct tinfo*)sockfd_msg)->fd;
  char* msg = ((struct tinfo*)sockfd_msg)->msg;
  pthread_t tid;

  // Get client's thread id
  pthread_mutex_lock(&mutex);
  auto c = clients.find(fd);
  if (c != clients.end()) {
    tid = c->second->tid;
  } else {
    if(DEBUG_MODE) 
      printf("send_client_msg: client fd not found");
    pthread_mutex_unlock(&mutex);
    return NULL;
  }
  pthread_mutex_unlock(&mutex);

  int num_fails = 0;

  // Send message and wait for confirmation
  while (num_fails < 5 && send(fd, msg, strlen(msg), 0) < 0) {
    num_fails++;
    // ? apagar
    if (DEBUG_MODE) cout << "send_client_msg: failed" << num_fails << endl;
  }

  // Disconnect client if more than 5 failed tries
  if (num_fails >= 5) {
    if (!pthread_cancel(tid)) {
      remove_client(fd);
    } else {
      IRC::error("send_client_msg:");
    }
  }

  // End thread
  return NULL;
}