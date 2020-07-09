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
  cout << "Bem vindo ao IRC!\nPara iniciar digite o comando /connect seguido "
          "de seu usuário!"
       << endl;
  cin >> command >> *client_name;
  // handles ctrl+d
  if (cin.eof()) {
    cout << "Obrigado por usar nosso IRC, espero que tenha se divertido!\n";
    exit(0);
  }
  while (command != "/connect") {
    cout << "Erro!\nDigite /connect seguido de seu usuário para iniciar!"
         << endl;
    cin >> command >> *client_name;
    // handles ctrl+d
    if (cin.eof()) {
      cout << "Obrigado por usar nosso IRC, espero que tenha se divertido!\n";
      exit(0);
    }
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
  cout << "Utilize o comando /join seguido do nome do canal para entrar em um "
          "canal!\n";
  string canal;
  cin >> command >> canal;
  //TODO --> VALIDAR NOME DO CANAL
  while (command != "/join") {
    cout << "Erro!\nUtilize o comando /join seguido do nome do canal para "
            "entrar em um canal!\n";
    cin >> command >> canal;
  }
  string nickAndChannel = (*client_name);
  nickAndChannel.append("$");
  nickAndChannel.append(canal);

  // Sends name to server to register nick
  int len = write(sock, nickAndChannel.c_str(), nickAndChannel.length());
  if (len < 0) {
    cout << "\nWarning: Mensagem nao enviada!\n";
  }

  system("clear");
  return sock;
}

int main(int argc, char *argv[]) {
  pthread_t recvt;

  string client_name;

  // Signal handling
  signal(SIGINT, SIG_IGN);

  int sock = connectUser(&client_name);

  cout << "Bem vindo(a) " << client_name << endl;
  flag = EMPTY_FLAG;
  // creating a client thread which is always waiting for a message
  pthread_create(&recvt, NULL, recvmg, &sock);

  // ready to read a message from console
  char msg[BUFFER_SIZE];
  string send_msg;
  int len;
  while (fgets(msg, BUFFER_SIZE - client_name.size(), stdin) != NULL) {
    // Format and send message to server
    if (msg[0] != '\n') send_msg = '\n' + client_name + ": " + msg;

    len = write(sock, send_msg.c_str(), send_msg.length());
    if (len < 0) {
      cout << "\nWarning: Message not sent!\n";
    }
    // Compare with commands and execute if true
    string command = msg;
    if (command.find("/quit\n") != string::npos) {
      flag = STOP_FLAG;
    }
    // Test flag
    if (flag == STOP_FLAG) {
      break;
    }
    send_msg[0] = '\0';
  }

  cout << "Obrigado por usar nosso IRC, espero que tenha se divertido!\n";
  // Close socket fd
  close(sock);
  return 0;
}
