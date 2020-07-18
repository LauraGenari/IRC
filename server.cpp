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
#include "serverMSG.h"

using namespace std;

pthread_mutex_t mutex;

typedef struct client {
  int sockfd;
  string nick;
  pthread_t tid;
  bool isMuted;
  bool isConnected;
  string currChannelName;
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
void change_client_channel(Client* client, string channelName);
void sendtoall(char* msg, int curr, string channelName);
void* send_client_msg(void* sockfd_msg);
void* recvmg(void* client_sock);
Client* searchClientByName(string channelName, string clientName);
void muteOrUnmute(Client* client, bool mute, string destinationName);
void kickClient(Client* client, string destinationName);

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
  auto channel = channels.find(new_client->currChannelName);
  if (channel == channels.end()) 
  {
    // Creates new channel if it doesn't exist
    Channel* new_channel = new Channel;
    new_channel->clients[new_client->sockfd] = new_client;

    // First client is marked as adm
    new_channel->adm = new_client->sockfd;
    new_channel->name = new_client->currChannelName;
    channels[new_client->currChannelName] = new_channel;
  }
  else 
  {
    channel->second->clients[new_client->sockfd] = new_client;
  }
  clients[new_client->sockfd] = new_client;  // Keeps track of all online clients
  pthread_mutex_unlock(&mutex);
}

// funciton that removes client from map
void remove_client(int sockfd) {
  pthread_mutex_lock(&mutex);
  // Find client to be removed, swap with the end, remove from the end.
  auto c = clients.find(sockfd);
  if (c != clients.end()) {
    delete channels[c->second->currChannelName]->clients[sockfd];
    channels[c->second->currChannelName]->clients.erase(sockfd);
    // removes the channel if empty
    if (channels[c->second->currChannelName]->clients.size() == 0) {
      delete channels[c->second->currChannelName];
      channels.erase(c->second->currChannelName);
    }
    clients.erase(c);
    close(sockfd);
  }
  pthread_mutex_unlock(&mutex);
}

void change_client_channel(Client* client, string channelName)
{
  if(client->isConnected)
  {
    return;
  }
  pthread_mutex_lock(&mutex);
  client->currChannelName = "#" + channelName;
  auto channel = channels.find(client->currChannelName);
  if (channel == channels.end()) 
  {
    // Creates new channel if it doesn't exist
    Channel* new_channel = new Channel;
    new_channel->clients[client->sockfd] = client;

    // First client is marked as adm
    new_channel->adm = client->sockfd;
    new_channel->name = client->currChannelName;
    channels[client->currChannelName] = new_channel;
  }
  else 
  {
    channel->second->clients[client->sockfd] = client;
  }
  client->isConnected = true;
  pthread_mutex_unlock(&mutex);
}

void disconnect_client(int fd)
{
  pthread_t tid;

  // Get client's thread id
  pthread_mutex_lock(&mutex);
  auto c = clients.find(fd);
  if (c != clients.end())
  {
    tid = c->second->tid;
  }
  else
  {
    if (DEBUG_MODE) printf("disconnect_client: client fd not found %d ", fd);
    pthread_mutex_unlock(&mutex);
    return;
  }
  pthread_mutex_unlock(&mutex);

  // End client's receive message thread and remove from data structure
  if (!pthread_cancel(tid)) 
  {
    remove_client(fd);
  }
  else 
  {
    IRC::error("disconnect_client:");
  }
}

/*===== Server Commons =====*/

// function that sends a message received by server to all clients
void sendtoall(char* msg, int curr, string channelName) {
  pthread_mutex_lock(&mutex);
  cout << "Entering sendtoall sock fd:"  << curr << "channel:" << channelName << endl;
  pthread_t send_msgt;
  auto foundChannel = channels.find(channelName);
  if(foundChannel == channels.end())
  {
    cout << "Channel name {"<< channelName <<"} doesnt exist\n";
    pthread_mutex_unlock(&mutex);
    return;
  }
  unordered_map<int, Client*> clientsInChannel = channels[channelName]->clients;
  // Iterate through all clients
  for (auto it = clientsInChannel.begin(); it != clientsInChannel.end(); it++) {
    if (DEBUG_MODE) cout << it->first << endl;
    int fd = it->first;

    // Send message to client
    struct tinfo* info = new struct tinfo;
    info->fd = fd;
    info->msg = msg;
    pthread_create(&send_msgt, NULL, &send_client_msg, (void*)info);
    // delete info;
  }
  pthread_mutex_unlock(&mutex);
}

/*===== Command Funcs ====*/

Client* searchClientByName(string channelName, string clientName) {
  pthread_mutex_lock(&mutex);
  unordered_map<int, Client*> clientsInChannel = channels[channelName]->clients;
  auto it = clientsInChannel.begin();
  while (it != clientsInChannel.end() && it->second->nick != clientName) it++;
  pthread_mutex_unlock(&mutex);

  return it == clientsInChannel.end() ? NULL : it->second;
}

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

void muteOrUnmute(Client* client, bool mute, string destinationName) {
  if (client->sockfd == channels[client->currChannelName]->adm) {
    Client* dest = searchClientByName(client->currChannelName, destinationName);
    if(dest)
      dest->isMuted = mute;
    else{
      char message[BUFFER_SIZE];
      sprintf(message, MSG_USER_NOT_FOUND);
      struct tinfo* errMsg = new struct tinfo;
      errMsg->msg = message;
      errMsg->fd = client->sockfd;
      send_client_msg(errMsg);
      delete errMsg;
    }
  }else{
    char message[BUFFER_SIZE];
    sprintf(message, MSG_NOT_ADM);
    struct tinfo* errMsg = new struct tinfo;
    errMsg->msg = message;
    errMsg->fd = client->sockfd;
    send_client_msg(errMsg);
    delete errMsg;
  }
}

void whoIs(Client* client, string destinationName) {
  if (client->sockfd == channels[client->currChannelName]->adm) {
    // searches for destinationName in channel
    Client* destClient =
        searchClientByName(client->currChannelName, destinationName);
    char message[BUFFER_SIZE];
    struct tinfo* ipMsg = new struct tinfo;
    ipMsg->fd = client->sockfd;
    if (destClient) {
      char* ip = new char[16];
      IRC::GetIPAddress(destClient->sockfd, ip);
      sprintf(message, "Server: %s ip is %s\n", destinationName.c_str(), ip);
      ipMsg->msg = message;
    } else {
      sprintf(message, MSG_USER_NOT_FOUND);
      ipMsg->msg = message;
    }
    send_client_msg(ipMsg);
    delete ipMsg;
  }else{
    char message[BUFFER_SIZE];
    sprintf(message, MSG_NOT_ADM);
    struct tinfo* errMsg = new struct tinfo;
    errMsg->msg = message;
    errMsg->fd = client->sockfd;
    send_client_msg(errMsg);
    delete errMsg;
  }
}

void kickClient(Client* client, string destinationName)
{
  char message[BUFFER_SIZE];
  char destMessage[BUFFER_SIZE];
  pthread_t sendt;

  //Check if client is Admin of channel
  if(client->sockfd == channels[client->currChannelName]->adm)
  {
    // Check if client is trying to remove himself
    if (!client->nick.compare(destinationName))
    {
      sprintf(message, "Server: cannot kick yourself");
    }
    else
    {
      // Check if destination name exists in channel
      Client* destClient = searchClientByName(client->currChannelName, destinationName);
      if(destClient != NULL)
      {
        pthread_mutex_lock(&mutex);
        destClient->isMuted = false;
        auto destChannel = channels.find(destClient->currChannelName);
        if(destChannel != channels.end())
        { 
          // Remove client from channel
          destChannel->second->clients.erase(destClient->sockfd);
          //destClient->currChannelName = NULL_CHANNEL;
          destClient->isConnected = false;
        }
        pthread_mutex_unlock(&mutex);
        
        // Send warning message to kicked client
        sprintf(destMessage, MSG_WARN_ABOUT_KICK);
        struct tinfo* warnMessage = new struct tinfo;
        warnMessage->msg = destMessage;
        warnMessage->fd = destClient->sockfd;
        pthread_create(&sendt, NULL, &send_client_msg, (void*) warnMessage);

        sprintf(message, MSG_KICK_SUCCESS);
      }
      else
      {
        // Send client not found message
        sprintf(message, MSG_USER_NOT_FOUND);
      }
    }
  }
  else
  {
    sprintf(message, MSG_NOT_ADM);
  }
  struct tinfo* reportMsg = new struct tinfo;
  reportMsg->msg = message;
  reportMsg->fd = client->sockfd;

  pthread_create(&sendt, NULL, &send_client_msg, (void*)reportMsg);
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
    pos = temp.find("#");
    if (pos != std::string::npos) {
      client->nick = temp.substr(0, pos).c_str();
      client->currChannelName = temp.substr(pos, temp.length()).c_str();
      client->isConnected = true;
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
  //sprintf(connection_message, "\nServer: %s has joined at %s\n",
  //        client->nick.c_str(), client->currChanelName.c_str());
  sprintf(connection_message, MSG_JOIN( client->nick.c_str(), client->currChannelName.c_str()));
  sendtoall(connection_message, sock, client->currChannelName);
  //delete connection_message;
  

  // Receive Loop
  int isRunning = 1;
  string command = msg;
  size_t pos;
  string name;
  int namePos;

  pthread_t send_tid;
  
  while ((len = recv(sock, msg, BUFFER_SIZE, 0)) > 0) 
  {
    // Break if connection ended
    if (!isRunning) break;
    msg[len] = '\0';
    command = msg;
    
    IRC::CommandType commandType = IRC::VerifyCommand(command, pos);
  
    // Check channel commands
    
    switch (commandType) 
    {
      case IRC::NONE:
        if(!client->isConnected)
        {
          char joinCH[] = "Server: Use '/join canal' para se conectar a um canal primeiro!\n";
          struct tinfo* joinMsg = new struct tinfo;
          joinMsg->fd = sock;
          joinMsg->msg = joinCH;
          pthread_create(&send_tid, NULL, &send_client_msg, (void*) joinMsg);
        }
        else 
        {
          if (!client->isMuted) 
          {
            char* temp = new char[BUFFER_SIZE + 3]; //OBS: Nao sei se e' uma boa deixar esse +3 pro tamanho do buffer
            sprintf(temp, "%s: %s", client->nick.c_str(), msg);
            sendtoall(temp, sock, client->currChannelName);
            // delete temp;
          }
          else
          {
            struct tinfo* muteMsg = new struct tinfo;
            muteMsg->fd = sock;
            char pong[] = "Server: Você está mutado!\n";
            muteMsg->msg = pong;
            // Sends message only to client who sent ping
            pthread_create(&send_tid, NULL, &send_client_msg, (void*) muteMsg);
            //delete muteMsg;
          }
        }
        break;
      
      case IRC::QUIT:
        isRunning = 0;
        break;

      case IRC::PING:
        sendPong(sock);
        break;

      case IRC::JOIN:
        namePos = command.find(" ");
        name = command.substr(namePos + 1, command.size());
        name = name.substr(0, name.size() - 1);
        change_client_channel(client, name);
        break;

      case IRC::WHOIS:
        namePos = command.find(" ");
        name = command.substr(namePos + 1, command.size());
        name = name.substr(0, name.size() - 1);
        whoIs(client, name);
        break;

      case IRC::MUTE:
        namePos = command.find(" ");
        name = command.substr(namePos + 1, command.size());
        name = name.substr(0, name.size() - 1);
        muteOrUnmute(client, true, name);
        break;

      case IRC::UNMUTE:
        namePos = command.find(" ");
        name = command.substr(namePos + 1, command.size());
        name = name.substr(0, name.size() - 1);
        muteOrUnmute(client, false, name);
        break;

      case IRC::KICK:
        namePos = command.find(" ");
        name = command.substr(namePos + 1, command.size());
        name = name.substr(0, name.size()-1); //OBS: isso e' pra remover o \n? se for da pra usar replace no lugar eu acho
        kickClient(client, name);
        break;
    }
  }
  // Send disconnect message to all
  char* disconnect_msg = new char[32];
  sprintf(disconnect_msg, "Server: %s has quit\n", client->nick.c_str());
  sendtoall(disconnect_msg, sock, client->currChannelName);
  // Remove Client, close connection and end thread
  // delete disconnect_msg;
  remove_client(sock);
  return NULL;
}

/**
 * Function that sends message to client
 * Warning: Must be called via Thread
 * */ 

void* send_client_msg(void* sockfd_msg) {
  // Get params
  int fd = ((struct tinfo*)sockfd_msg)->fd;
  string msg = ((struct tinfo*)sockfd_msg)->msg;

  int num_fails = 0;

  // Send message and wait for confirmation
  while (num_fails < 5 && send(fd, msg.c_str(), msg.size(), 0) < 0) 
  {
    num_fails++;
    // ? apagar
    if (DEBUG_MODE) cout << "send_client_msg: failed" << num_fails << endl;
  }

  // Disconnect client if more than 5 failed tries
  if (num_fails >= 5) 
  {
    disconnect_client(fd);
  }

  // End thread
  return NULL;
}