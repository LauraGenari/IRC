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
} Client;

struct tinfo {
  int fd;
  char* msg;
};

vector<Client*> clients;
// Hashmap for finding clients via its socket file descriptor
// Key: client->sockfd | Value: client
unordered_map<int, Client*> clients_hash =
    unordered_map<int, Client*>();

void add_client(Client* new_client);
void remove_client(int sockfd);
void* send_client_msg(void* sockfd_msg);
void* recvmg(void* client_sock);

// function that sends a message received by server to all clients
void sendtoall(char* msg, int curr) {
  pthread_mutex_lock(&mutex);

  pthread_t send_msgt;

  // Iterate through all clients
  for (auto it = clients.begin(); it != clients.end(); it++) {
    if (DEBUG_MODE) cout << (*it)->sockfd << endl;
    int fd = (*it)->sockfd;
    // Send message to client if its not the one who wrote it
    if (fd != curr) {
      struct tinfo* info = new struct tinfo;
      info->fd = fd;
      info->msg = msg;
      pthread_create(&send_msgt, NULL, &send_client_msg, (void*)info);
    }
  }
  pthread_mutex_unlock(&mutex);
}

int main(int argc, char* argv[]) {
  struct sockaddr_in ServerIp;
  pthread_t recvt;

  int sock = 0, client_fd = 0;

  // socket configuration
  ServerIp.sin_family = AF_INET;
  ServerIp.sin_port = htons(atoi(argv[1]));
  ServerIp.sin_addr.s_addr = inet_addr("127.0.0.1");

  sock = socket(AF_INET, SOCK_STREAM, 0);

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

    // Add to hashmap and vector
    add_client(new_client);

    // Create thread for receiving client's messages
    pthread_create(&recvt, NULL, &recvmg, (void*)new_client);
    // Assign thread_id to client
    new_client->tid = recvt;

    // Send connection message to all clients on server
    char connection_message[] = "\nServer: A client has joined\n";
    sendtoall(connection_message, client_fd);
  }
  return 0;
}

// function that adds client to list
void add_client(Client* new_client) {
  pthread_mutex_lock(&mutex);
  clients.push_back(new_client);
  clients_hash[new_client->sockfd] = new_client;
  pthread_mutex_unlock(&mutex);
}

// funciton that removes client from list
void remove_client(int sockfd) {
  pthread_mutex_lock(&mutex);
  // Find client to be removed, swap with the end, remove from the end.
  for (int i = 0; i < clients.size(); i++) {
    if (clients[i]->sockfd == sockfd) {
      Client* c = clients[i];
      clients[i] = clients.back();
      clients.pop_back();
      // close client's file descriptor
      close(sockfd);
      break;
    }
  }
  pthread_mutex_unlock(&mutex);
}

// function that receives a message from client
void* recvmg(void* client_sock) {
  int sock = ((Client*)client_sock)->sockfd;
  char msg[BUFFER_SIZE] = {0};
  int len;

  // Receive message
  while ((len = recv(sock, msg, BUFFER_SIZE, 0)) > 0) {
    msg[len] = '\0';
    // Handle /quit command
    string command = msg;
    if (command.find("/quit\n") == string::npos &&
        command.find("/ping\n") == string::npos) {
      sendtoall(msg, sock);
    } else if (command.find("/quit\n") != string::npos) {
      // Send quit message to all and break while
      if (DEBUG_MODE)
        sprintf(msg, "Server: %d has quit\n", sock);
      else
        sprintf(msg, "Server: A client has quit\n");
      sendtoall(msg, sock);
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
  printf("Server: %d has quit\n", sock);
  // Remove Client, close connection and end thread
  remove_client(sock);
  return NULL;
}

void* send_client_msg(void* sockfd_msg) {
  // Get params
  int fd = ((struct tinfo*)sockfd_msg)->fd;
  char* msg = ((struct tinfo*)sockfd_msg)->msg;

  // ? apagar
  if (DEBUG_MODE)
    cout << "send_client_msg: sending to " << fd << " " << msg
              << endl;
  // Get client's thread id
  // ? e' operacao de leitura, entao nao e' pra da deadlock, neh ?
  pthread_mutex_lock(&mutex);
  pthread_t tid;
  auto c = clients_hash.find(fd);
  if (c == clients_hash.end()) {
    // ? apagar
    if (DEBUG_MODE)
      cout << "send_client_msg: no such client with fd " << fd
                << endl;
  } else {
    tid = c->second->tid;
  }
  pthread_mutex_unlock(&mutex);

  int num_fails = 0;

  // Send message and wait for confirmation
  while (num_fails < 5 && send(fd, msg, strlen(msg), 0) < 0) {
    num_fails++;
    // ? apagar
    if (DEBUG_MODE)
      cout << "send_client_msg: failed" << num_fails << endl;
  }

  // Disconnect client if more than 5 failed tries
  if (num_fails >= 5) {
    if (!pthread_cancel(tid)) {
      remove_client(fd);
    } else {
      perror("send_client_msg: no thread with this id found");
    }
  }

  // End thread
  return NULL;
}