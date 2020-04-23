/* A simple server in the internet domain using TCP
The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  
#include <sys/select.h>
#include "irc.h"

int main(int argc, char *argv[])
{
    //server and client socket file descriptors, PORT number and client length
    int sockfd, newsockfd, PORT, client_len, activity, maxsd, clientsockets[2]={0}, serv_len, valread; 
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
    
    if (sockfd <= 0)
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
    if(listen(sockfd, 2) < 0)
        error("ERROR on listening");
    
    client_len = sizeof(client_addr);
    serv_len = sizeof(serv_addr);

    fd_set readfds;

    while(1){
        //clear the socket set  
        FD_ZERO(&readfds);   
        
        //add master socket to set  
        FD_SET(sockfd, &readfds);   
        maxsd = sockfd;   

        for(int i = 0; i < 2; i++){
            //adiciona o socket à
            if(clientsockets[i] > 0){
                FD_SET(clientsockets[i], &readfds);
            }

            if(clientsockets[i] > maxsd){
                maxsd= clientsockets[i];
            }
        }

        activity = select(maxsd+1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }   

        if (FD_ISSET(sockfd, &readfds)){
            newsockfd = accept(sockfd, (struct sockaddr*) &serv_addr, &serv_len);
    
            if (newsockfd < 0)
                error("ERROR on accept");
        
            //add new socket to array of sockets  
            for (int i = 0; i < 2; i++){   
                //if position is empty  
                if( clientsockets[i] == 0 )   
                {   
                    clientsockets[i] = newsockfd;   
                    printf("Adding to list of sockets as %d\n" , i);   
                            
                    break;   
                }       
                
            }
        }//seg fault anrwa sw wnreE NO IF
         if(clientsockets[1] != 0 && clientsockets[2] != 0){
             printf("aqui");
             if ( FD_ISSET( clientsockets[1] , &readfds)){   
                //Input/Output bytes length
                int iosize = 0;
                memset(buffer, 0, BUFFER_SIZE);

                //le a mensagem do socket
                iosize = read(newsockfd, buffer, BUFFER_SIZE - 1);
                
                if (iosize < 0)
                    error("ERROR reading from socket");
                
                printf("1: %s\n", buffer);
                send(clientsockets[2] , buffer , strlen(buffer) , 0 );
            }  
            if (FD_ISSET( clientsockets[2] , &readfds)){   
                    //Input/Output bytes length
                int iosize = 0;
                memset(buffer, 0, BUFFER_SIZE);

                //le a mensagem do socket
                iosize = read(newsockfd, buffer, BUFFER_SIZE - 1);
                
                if (iosize < 0)
                    error("ERROR reading from socket");
                
                printf("2: %s\n", buffer);
                send(clientsockets[1] , buffer , strlen(buffer) , 0 );
            }
        }
         
    }


/*
    
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
        error("ERROR writing to socket");*/
    
    return 0; 
}