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

typedef struct channel {
  unordered_map<int, Client*> clients;
  int id;
  string name;
  int adm;  // admin's sockfd
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
void sendtoall(char* msg, int curr, string channelName);
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
    Client* new_client = new Client;
    new_client->sockfd = client_fd;
    new_client->isMuted = false;

    // Create thread for receiving client's messages
    pthread_create(&recvt, NULL, &recvmg, (void*)new_client);

    // Assign thread_id to client
    new_client->tid = recvt;

    // Send connection message to all clients on server
  }
  return 0;
}

/*===== Map Operations =====*/

// function that adds client to map of its channel
void add_client(Client* new_client) {
  pthread_mutex_lock(&mutex);
  auto channel = channels.find(new_client->currChanelName);
  if (channel == channels.end()) {
    // Creates new channel if it doesn't exist
    Channel* new_channel = new Channel;
    new_channel->clients[new_client->sockfd] = new_client;

    // First client is marked as adm
    new_channel->adm = new_client->sockfd;
    new_channel->name = new_client->currChanelName;
    channels[new_client->currChanelName] = new_channel;
  } else {
    channel->second->clients[new_client->sockfd] = new_client;
  }
  clients[new_client->sockfd] =
      new_client;  // Keeps track of all online clients
  pthread_mutex_unlock(&mutex);
}

// funciton that removes client from map
void remove_client(int sockfd) {
  pthread_mutex_lock(&mutex);
  // Find client to be removed, swap with the end, remove from the end.
  auto c = clients.find(sockfd);
  if (c != clients.end()) {
    delete channels[c->second->currChanelName]->clients[sockfd];
    channels[c->second->currChanelName]->clients.erase(sockfd);
    // removes the channel if empty
    if (channels[c->second->currChanelName]->clients.size() == 0) {
      delete channels[c->second->currChanelName];
      channels.erase(c->second->currChanelName);
    }
    clients.erase(c);
    close(sockfd);
  }
  pthread_mutex_unlock(&mutex);
}

/*===== Server Commons =====*/

// function that sends a message received by server to all clients
void sendtoall(char* msg, int curr, string channelName) {
  pthread_mutex_lock(&mutex);

  pthread_t send_msgt;
  unordered_map<int, Client*> clientsInChannel = channels[channelName]->clients;
  // Iterate through all clients
  for (auto it = clientsInChannel.begin(); it != clientsInChannel.end(); it++) {
    if (DEBUG_MODE) cout << it->first << endl;
    int fd = it->first;
    // Send message to client if its not the one who wrote it

    struct tinfo* info = new struct tinfo;
    info->fd = fd;
    info->msg = msg;
    pthread_create(&send_msgt, NULL, &send_client_msg, (void*)info);
    // delete info;
  }
  pthread_mutex_unlock(&mutex);
}

/*===== Command Funcs ====*/

void sendPong(int sock) {
  // Handle ping message
  struct tinfo* pongMsg = new struct tinfo;
  pongMsg->fd = sock;
  char pong[] = "Server: pong\n";
  pongMsg->msg = pong;
  // Sends message only to client who sent ping
  send_client_msg((void*)pongMsg);
  delete pongMsg;
}

void whoIs(Client* client, string destinationName) {
  if (client->sockfd == channels[client->currChanelName]->adm) {
    // searches for destinationName in channel
    pthread_mutex_lock(&mutex);
    unordered_map<int, Client*> clientsInChannel =
        channels[client->currChanelName]->clients;
    auto it = clientsInChannel.begin();
    while (it != clientsInChannel.end() && it->second->nick != destinationName)
      it++;
    pthread_mutex_unlock(&mutex);
    char message[BUFFER_SIZE];
    struct tinfo* ipMsg = new struct tinfo;
    ipMsg->fd = client->sockfd;
    if (it != clientsInChannel.end()) {
      char* ip = new char[16];
      IRC::GetIPAddress(it->first, ip);
      sprintf(message, "Server: %s ip is %s\n", destinationName.c_str(), ip);
      ipMsg->msg = message;
    } else {
      sprintf(message, "Server: Nome não encontrado no canal!\n");
      ipMsg->msg = message;
    }
    send_client_msg(ipMsg);
    delete ipMsg;
  }
}

/*===== Thread Funcs =====*/

// function that receives a message from client
void* recvmg(void* client_sock) {
  Client* client = (Client*)client_sock;
  int sock = client->sockfd;
  char msg[BUFFER_SIZE] = {0};
  int len;

  // Receive client name and client channel
  if (recv(sock, msg, BUFFER_SIZE, 0) > 0) {
    int pos = 0;
    string temp = msg;
    pos = temp.find("$");
    if (pos != std::string::npos) {
      client->nick = temp.substr(0, pos).c_str();
      client->currChanelName = temp.substr(pos, temp.length()).c_str();
    }
    // Add to hashmap and vector
    add_client(client);

  } else {
    // Remove Client, close connection and end thread
    remove_client(sock);
    return NULL;
  }

  // Send join message to all clients
  char* connection_message = new char[BUFFER_SIZE];
  sprintf(connection_message, "\nServer: %s has joined at %s\n",
          client->nick.c_str(), client->currChanelName.c_str());
  sendtoall(connection_message, sock, client->currChanelName);
  delete connection_message;
  // Receive message
  int isRunning = 1;
  while ((len = recv(sock, msg, BUFFER_SIZE, 0)) > 0) {
    if (!isRunning) break;
    msg[len] = '\0';
    string command = msg;
    size_t pos;
    IRC::CommandType commandType = IRC::VerifyCommand(command, pos);

    switch (commandType) {
      case IRC::NONE:
        if (!client->isMuted) {
          char* temp = new char[BUFFER_SIZE + 3];
          sprintf(temp, "%s: %s", client->nick.c_str(), msg);
          sendtoall(temp, sock, client->currChanelName);
          // delete temp;
        } else {
          struct tinfo* muteMsg = new struct tinfo;
          muteMsg->fd = sock;
          char pong[] = "Server: Você está mutado!\n";
          muteMsg->msg = pong;
          // Sends message only to client who sent ping
          send_client_msg((void*)muteMsg);
          delete muteMsg;
        }
        break;
      case IRC::PING:
        sendPong(sock);
        break;
      case IRC::QUIT:
        isRunning = 0;
        break;
      case IRC::WHOIS:
        int pos = command.find(" ");
        string name = command.substr(pos+1, command.size());
        name = name.substr(0, name.size()-1);
        whoIs(client, name);
        break;
    }
  }
  // Send disconnect message to all
  char* disconnect_msg = new char[32];
  sprintf(disconnect_msg, "Server: %s has quit\n", client->nick.c_str());
  sendtoall(disconnect_msg, sock, client->currChanelName);
  // Remove Client, close connection and end thread
  // delete disconnect_msg;
  remove_client(sock);
  return NULL;
}

// Function that sends message to client
void* send_client_msg(void* sockfd_msg) {
  // Get params
  int fd = ((struct tinfo*)sockfd_msg)->fd;
  string msg = ((struct tinfo*)sockfd_msg)->msg;
  pthread_t tid;

  // Get client's thread id
  pthread_mutex_lock(&mutex);
  auto c = clients.find(fd);
  if (c != clients.end()) {
    tid = c->second->tid;
  } else {
    if (DEBUG_MODE) printf("send_client_msg: client fd not found %d ", fd);
    pthread_mutex_unlock(&mutex);
    return NULL;
  }
  pthread_mutex_unlock(&mutex);

  int num_fails = 0;

  // Send message and wait for confirmation
  while (num_fails < 5 && send(fd, msg.c_str(), msg.size(), 0) < 0) {
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