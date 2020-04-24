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

void *recvmg(void *my_sock) {
  int sock = *((int *)my_sock);
  int len;
  char msg[4096];
  // client thread always ready to receive message
  while ((len = recv(sock, msg, 4096, 0)) > 0) {
    msg[len] = '\0';
    fputs(msg, stdout);
    //std::cout << msg;
  }
}
int main(int argc, char *argv[]) {
  pthread_t recvt;
  int len;
  int sock;
  struct sockaddr_in ServerIp;
  std::string client_name;

  if(argc < 3){
    printf("insira seu username e a PORT");
  }

  client_name = argv[1];
  //strcpy(client_name, argv[1]);
  
  sock = socket(AF_INET, SOCK_STREAM, 0);
  
  ServerIp.sin_port = htons(atoi(argv[2]));
  ServerIp.sin_family = AF_INET;
  ServerIp.sin_addr.s_addr = inet_addr("127.0.0.1");
  
  if ((connect(sock, (struct sockaddr *)&ServerIp, sizeof(ServerIp))) == -1){
    perror("connect: ");
    exit(EXIT_FAILURE);
  }

  // creating a client thread which is always waiting for a message
  pthread_create(&recvt, NULL, recvmg, &sock);

  // ready to read a message from console
    char msg[4096];
    std::string send_msg;
    while (fgets(msg,4096 - client_name.size(), stdin) > 0) {
      send_msg = '\n'+ client_name + ": " + msg;
      len = write(sock, send_msg.c_str(), send_msg.length());
      if (len < 0){
        std::cout << "\nWarning: Message not sent!\n";
      }
    }


  // thread is closed
  pthread_join(recvt, NULL);
  close(sock);
  
  system("pause");
  return 0;
}


/*
EU TROXA CODANDO NA MAO SENDO Q O FEGTS JA FAZ O TRABALHO MAS N 
VO APAGAR PQ DEU UM PUTA TRABALHO, MSM NAO FUNCIONANDO
char msg[4096];
  std::string send_msg;
  int i;
  char buffer = fgetc(stdin);
  while(buffer != EOF){
    i = 0;
    while ( buffer != '\n') {
      msg[i] = buffer;
      buffer = fgetc(stdin);
      i++;
      if(i==4095){
        msg[i]='\0';
        send_msg = client_name + ": " + msg;
        len = write(sock, send_msg.c_str(), send_msg.length());
        if (len < 0){
          std::cout << "\nWarning: Message not sent!\n";
        }
        i=0;
      }
    }
    msg[i]='\0';
    send_msg = client_name + ": " + msg;
    len = write(sock, send_msg.c_str(), send_msg.length());
    if (len < 0){
      std::cout << "\nWarning: Message not sent!\n";
    }
    fflush(stdin);
    buffer = fgetc(stdin);
  }

*/
