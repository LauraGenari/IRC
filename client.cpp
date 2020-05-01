#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <signal.h>

#define STOP_FLAG 0x0001
#define EMPTY_FLAG 0x0000

int flag;
//function that receives a messagem from server
void *recvmg(void *my_sock) {
  int sock = *((int *)my_sock);
  int len;
  char msg[4096];

  //client thread always ready to receive message
  while ((len = recv(sock, msg, 4096, 0)) > 0) {
    msg[len] = '\0';
    fputs(msg, stdout);
  }
  return NULL;
}
//function that advise the correct manner to quit
void interrupt_handler(int signo, siginfo_t *sinfo, void *context){
  std::cout << "\nPlease use the '/quit' command to leave\n";
}

int main(int argc, char *argv[]) {
  pthread_t recvt;
  int len;
  int sock;
  struct sockaddr_in ServerIp;
  std::string client_name;

  //Signal handling
  struct sigaction act;
  memset(&act, 0, sizeof(struct sigaction));
  act.sa_sigaction = interrupt_handler;
  act.sa_flags = SA_SIGINFO;
  sigaction(SIGINT, &act, NULL);

  if(argc < 3){
    std::cout << "Modo de uso: " << argv[0] << " nome PORT" << std::endl;
  }

  client_name = argv[1];
  //socket configuration
  sock = socket(AF_INET, SOCK_STREAM, 0);
  
  ServerIp.sin_port = htons(atoi(argv[2]));
  ServerIp.sin_family = AF_INET;
  ServerIp.sin_addr.s_addr = inet_addr("127.0.0.1");
  
  if ((connect(sock, (struct sockaddr *)&ServerIp, sizeof(ServerIp))) == -1){
    perror("connect: ");
    exit(EXIT_FAILURE);
  }
  flag = EMPTY_FLAG;
  // creating a client thread which is always waiting for a message
  pthread_create(&recvt, NULL, recvmg, &sock);

  // ready to read a message from console
  char msg[4096];
  std::string send_msg;
  while (fgets(msg,4096 - client_name.size(), stdin) > 0) {
    //Format and send message to server
    send_msg = '\n'+ client_name + ": " + msg;
    len = write(sock, send_msg.c_str(), send_msg.length());
    if (len < 0){
      std::cout << "\nWarning: Message not sent!\n";
    }
    //Compare with commands and execute if true
    if(!strncmp(msg, "/quit\n", 6)){
      flag = STOP_FLAG;
    }
    //Test flag
    if(flag == STOP_FLAG){
      std::cout << "Thank you for using our IRC, hope you enjoyed your experience!\n";
      break;
    }
  }
  printf("BROKEN\n");

  close(sock);
  printf("CLOSED\n");
  
  return 0;
}
