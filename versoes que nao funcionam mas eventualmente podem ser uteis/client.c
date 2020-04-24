#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "irc.h"

int main(int argc, char *argv[])
{
    int sockfd, PORT, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFFER_SIZE];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    //guarda o PORT 
    PORT = atoi(argv[2]);

    //cria  socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) 
        error("ERROR opening socket");

    //salva o nome do host na estrutura do host
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    //zera o a variavel do servido e seta pra IPv4
    memset((char*) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    //copia o endereço do host para a estrutura de socket do server
    memcpy((char*) &serv_addr.sin_addr.s_addr, (char*)server->h_addr_list[0], server->h_length);
   
    //copia o PORT para a estrutura de socket do server
    serv_addr.sin_port = htons(PORT);
    printf("%s", server->h_addr_list[0]);
    
    //conecta o socket ao server
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    
    //Conexao sucedida, comecar troca de mensagens
    printf("Comece a se falar\n");
    while(1){
        memset(buffer, 0, BUFFER_SIZE);
        //guarda a mensagem no buffer
        fgets(buffer, BUFFER_SIZE-1, stdin);
        
        //escreve a mensagem no socket
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) 
            error("ERROR writing to socket");
        
        memset(buffer, 0, BUFFER_SIZE);
        //le a mensagem do socket
        n = read(sockfd, buffer, BUFFER_SIZE-1);
        if (n < 0) 
            error("ERROR reading from socket");
    }
    return 0;
}