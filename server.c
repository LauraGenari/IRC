/* A simple server in the internet domain using TCP
The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "irc.h"

int main(int argc, char *argv[])
{
    //server and client socket file descriptors, PORT number and client length
    int sockfd, newsockfd, PORT, client_len; 
    //Buffer for reading and writing messages
    char buffer[BUFFER_SIZE]; 
    //sockets
    struct sockaddr_in serv_addr, client_addr;
    
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    //cria um socket(ligação)
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0)
        error("socket: ");
    
    //zera o socket do servidor
    memset((char*) &serv_addr, 0, sizeof(serv_addr));

    //seta as devidas informações do servidor que se conectará
    PORT = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    
    //faz a ligação do servidor ao socket
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    
    //coloca o socket no modo passivo, aguardando uma coneção
    if(listen(sockfd, 5) < 0)
        error("ERROR on listening");
    
    client_len = sizeof(client_addr);
    
    //liga o socket do cliente no socket do servidor
    newsockfd = accept(sockfd, (struct sockaddr*) &client_addr, &client_len);
    
    if (newsockfd < 0)
        error("ERROR on accept");
    
    //Input/Output bytes length
    int iosize = 0;
    memset(buffer, 0, BUFFER_SIZE);

    //le a mensagem do socket
    iosize = read(newsockfd, buffer, BUFFER_SIZE - 1);
    
    if (iosize < 0)
        error("ERROR reading from socket");
    
    printf("Here is the message: %s\n",buffer);
    
    //escreve retorno no socket
    iosize = write(newsockfd,"I got your message",18);
    
    if (iosize < 0)
        error("ERROR writing to socket");
    
    return 0; 
}