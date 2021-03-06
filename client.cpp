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
  size_t pos = 0;

  string command = "";
  string ip = "";
  string port = "";

  cout
      << "Bem vindo ao IRC!\nPara iniciar digite o comando /connect [IP] [PORT]"
      << endl;

  while (true) {
    // handles ctrl+d
    if (cin.eof()) {
      cout << "Obrigado por usar nosso IRC, espero que tenha se divertido!\n";
      exit(0);
    }
    cin >> command >> ip >> port;
    // NOTA: cin ignora espaco antes e depois, entao o cara pode usar " /connect
    // ip port" ou "/connectttt ip port" se nao acrescentar o ' ' dps.
    // std::cout << command;
    // if(IRC::VerifyCommand(command+" ", pos) != IRC::CONNECT){
    // acho que nesse caso nao precisa do .find() pq a gente sabe q o comando
    // inteiro vai ta na string command.
    if (command != "/connect") {
      cout << "Erro!\nDigite /connect [IP] [PORT]" << endl;
    } else
      break;
  }

  // --- Conexao com servidor --- //
  cout << "Conectando ....." << endl;
  // socket configuration
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  currentSock = sock;
  // ServerIp.sin_port = htons(atoi(argv[2]));
  ServerIp.sin_port = htons(atoi(port.c_str()));
  ServerIp.sin_family = AF_INET;
  ServerIp.sin_addr.s_addr = inet_addr(ip.c_str());

  if ((connect(sock, (struct sockaddr *)&ServerIp, sizeof(ServerIp))) == -1) {
    IRC::error("connect");
  }

  // -- Comando nickname -- //
  cout << "Utilize o comando /nickname seguido de seu apelido desejado\n";
  cin >> command >> *client_name;
  while (command != "/nickname" || !IRC::checkNick(client_name->c_str())) {
    cout << "Erro!\nUtilize o comando /nickname seguido de seu apelido "
            "desejado\n";
    cin >> command >> *client_name;
  }

  // -- Comando join -- //
  string canal = "";

  cout << "Utilize o comando /join seguido do nome do canal para entrar em um "
          "canal!\n";
  cin >> command >> canal;
  while (command != "/join" || !IRC::checkChannel(canal.c_str())) {
    cout << "Erro!\nUtilize o comando /join seguido do nome do canal para "
            "entrar em um canal!\n";
    cin >> command >> canal;
  }

  string nickAndChannel = (*client_name);
  nickAndChannel.append(canal);

  // Sends name to server to register nick
  int len = write(sock, nickAndChannel.c_str(), nickAndChannel.length());
  if (len < 0) {
    cout << "\nWarning: Mensagem nao enviada!\n";
    exit(0);
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
  size_t pos;
  int len;
  while (fgets(msg, BUFFER_SIZE, stdin) != NULL) {
    // Format and send message to server
    if (msg[0] != '\n') {
      len = write(sock, msg, BUFFER_SIZE);
      if (len < 0) {
        cout << "\nWarning: Message not sent!\n";
      }
    }
    string command = msg;
    // Compare with commands and execute if true
    switch (IRC::VerifyCommand(command, pos))  // TODO: SE PA PODE TROCAR POR 2 IF's
    {
      case IRC::QUIT:
        flag = STOP_FLAG;
        break;
      case IRC::NICKNAME:
        // TODO: Verificar se tem nome valido
        break;
      default:
        break;
    }

    // Test flag
    if (flag == STOP_FLAG) {
      break;
    }
    msg[0] = '\0';
  }

  cout << "Obrigado por usar nosso IRC, espero que tenha se divertido!\n";
  // Close socket fd
  close(sock);
  return 0;
}
