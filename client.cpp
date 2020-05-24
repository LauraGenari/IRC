#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <csignal>
#include <iostream>
#include <string>

#include "irc.h"

#define STOP_FLAG 0x0001
#define EMPTY_FLAG 0x0000

using namespace std;

int currentSock;
int flag;
// function that receives a messagem from server
void *recvmg(void *my_sock) {
  int sock = *((int *)my_sock);
  int len;
  char msg[BUFFER_SIZE];

  // client thread always ready to receive message
  while ((len = recv(sock, msg, BUFFER_SIZE, 0)) > 0) {
    msg[len] = '\0';
    fputs(msg, stdout);
  }
  return NULL;
}
// function that advise the correct manner to quit -- WIP
void interrupt_handler(int signo) {
  cout << "\nPlease use the '/quit' command to leave\n";
}


// Sets client_name and connects client to server, returns socket Id
int connectUser(string *client_name) {
  struct sockaddr_in ServerIp;
  string command = "";
  cout << "Bem vindo ao IRC!\nPara iniciar digite o comando /connect seguido de seu usuário!" << endl;
  cin >> command >> *client_name;
  while (command != "/connect") {
    cout << "Erro!\nDigite /connect seguido de seu usuário para iniciar!" << endl;
    cin >> command >> *client_name;
  }

  cout << "Conectando ....." << endl;
  // socket configuration
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  currentSock = sock;
  // ServerIp.sin_port = htons(atoi(argv[2]));
  ServerIp.sin_port = htons(2000);
  ServerIp.sin_family = AF_INET;
  ServerIp.sin_addr.s_addr = inet_addr("127.0.0.1");

  if ((connect(sock, (struct sockaddr *)&ServerIp, sizeof(ServerIp))) == -1) {
    IRC::error("connect");
  }
  system("clear");
  return sock;
}

int main(int argc, char *argv[]) {
  pthread_t recvt;

  string client_name;

  // Signal handling
  /*struct sigaction act;
  memset(&act, 0, sizeof(struct sigaction));
  act.sa_sigaction = interrupt_handler;
  act.sa_flags = SA_SIGINFO;
  sigaction(SIGINT, &act, NULL);
  signal(SIGINT, SIG_IGN);*/

  signal(SIGINT, interrupt_handler);

  /*if (argc < 3) {
    cout << "Modo de uso: " << argv[0] << " nome PORT" << endl;
  }*/

  int sock = connectUser(&client_name);

  cout << "Bem vindo(a) " << client_name << endl;
  flag = EMPTY_FLAG;
  // creating a client thread which is always waiting for a message
  pthread_create(&recvt, NULL, recvmg, &sock);

  // ready to read a message from console
  char msg[BUFFER_SIZE];
  string send_msg;
  int len;
  while (fgets(msg, BUFFER_SIZE - client_name.size(), stdin) > 0) {
    // Format and send message to server
    send_msg = '\n' + client_name + ": " + msg;
    len = write(sock, send_msg.c_str(), send_msg.length());
    if (len < 0) {
      cout << "\nWarning: Message not sent!\n";
    }
    // Compare with commands and execute if true
    if (!strncmp(msg, "/quit\n", 6)) {
      flag = STOP_FLAG;
    }
    // Test flag
    if (flag == STOP_FLAG) {
      cout << "Thank you for using our IRC, hope you enjoyed your experience!\n";
      break;
    }
  }

  close(sock);
  return 0;
}
