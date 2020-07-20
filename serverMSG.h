#ifndef SERVER_MSG_H
#define SERVER_MSG_H

#define MSG_JOIN(x,y) "\nServer: %s entrou no canal %s\n", x, y 
#define MSG_QUIT(x) "\nServer: %s has quit\n", x
#define MSG_NOT_ADM "\nServer: Você não é administrador deste canal!\n"
#define MSG_USER_NOT_FOUND "\nServer: Usuário não encontrado!\n"
#define MSG_KICK_SUCCESS "\nServer: Usuário foi expulso do canal\n"
#define MSG_WARN_ABOUT_KICK "\nServer: Você foi expulso do canal\n"
#define MSG_MUTE "\nServer: Você está está mutado!\n"
#define MSG_PONG "\nServer: pong\n"
#define MSG_SELF_KICK "\nServer: Cannot kick yourself\n"
#define MSG_NOT_CONNECTED_TO_CHANNEL "\nServer: Use '/join canal' para se conectar a um canal primeiro!\n"
#define MSG_WRONG_CHANNEL_NAME "\nServer: Por favor, use um nome de canal válido!\n Server: Canais começam com # ou & e têm de 3 a 50 caracteres\n"
#define MSG_WRONG_NICKNAME "\nServer: Por favor, insira um nickname válido!\n"
#define MSG_CHANGED_NICKNAME(x) "\nServer: Seu nickname foi mudado para %s!\n", x
#define MSG_CHANGED_NICKNAME_CHANNEl(x, y) "\nServer: %s mudou seu nickname para %s!\n", x, y
#define MSG_MUTE_SUCCESS "\nServer: Usuário foi mutado com sucesso!\n"
#define MSG_UNMUTE_SUCCESS "\nServer: Usuário foi desmutado com sucesso!\n"
#define MSG_ALREADY_CONNECTED "\nServer: Você já está conectado a um canal!\n"

#define NULL_CHANNEL ""

#endif