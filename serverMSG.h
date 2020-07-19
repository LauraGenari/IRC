#ifndef SERVER_MSG_H
#define SERVER_MSG_H

#define MSG_JOIN(x,y) "\nServer: %s entrou no canal %s\n", x, y 
#define MSG_QUIT(x) "Server: %s has quit\n", x
#define MSG_NOT_ADM "Server: Você não é administrador deste canal!\n"
#define MSG_USER_NOT_FOUND "Server: Usuário não encontrado!\n"
#define MSG_KICK_SUCCESS "Server: Usuário foi expulso do canal\n"
#define MSG_WARN_ABOUT_KICK "Server: Você foi expulso do canal\n"
#define MSG_MUTE "Server: Você está está mutado!\n"
#define MSG_PONG "Server: pong\n"
#define MSG_SELF_KICK "Server: Cannot kick yourself\n"
#define MSG_NOT_CONNECTED_TO_CHANNEL "Server: Use '/join canal' para se conectar a um canal primeiro!\n"
#define MSG_WRONG_CHANNEL_NAME "Server: Por favor, use um nome de canal válido!\n Server: Canais começam com # ou & e têm de 3 a 50 caracteres\n"
#define MSG_WRONG_NICKNAME "Server: Por favor, insira um nickname válido!\n"
#define MSG_MUTE_SUCCESS "Server: Usuário foi mutado com sucesso!\n"
#define MSG_UNMUTE_SUCCESS "Server: Usuário foi desmutado com sucesso!\n"

#define NULL_CHANNEL ""

#endif