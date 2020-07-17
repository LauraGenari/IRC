#ifndef _IRC_H_
#define _IRC_H_

#define BUFFER_SIZE 4096
#define DEBUG_MODE true

namespace IRC{

    typedef enum
    {
        NONE,
        CONNECT,
        NICKNAME,
        JOIN,
        QUIT,
        KICK,
        MUTE,
        UNMUTE,
        WHOIS,
        PING
    } CommandType;

    void error(std::string msg){
        perror(msg.c_str());
        exit(EXIT_FAILURE);
    }

    /**
     * VerifyCommand
     * Verifies if the string command received contains any valid command. If true, it sets pos to the found position
     * and returns the respective CommandType.
     * @return respective CommandType, NONE if no command was found.
     * @param string command -> the string to find command from, size_t &pos setted to found position or std::string::npos.
    */
    CommandType VerifyCommand(std::string command, size_t &pos)
    {
        //Verifica somente se a mensagem comecar com '/'
        if(command.c_str()[0] != '/')
        {
            pos = std::string::npos;
            return NONE;
        }

        if((pos = command.find("/connect ")) != std::string::npos)
        {
            return CONNECT;
        }
        else if((pos = command.find("/nickname ")) != std::string::npos)
        {
            return NICKNAME;
        }
        else if((pos = command.find("/join ")) != std::string::npos)
        {
            return JOIN;
        }
        else if((pos = command.find("/quit\n")) != std::string::npos)
        {
            return QUIT;
        }
        else if((pos = command.find("/kick ")) != std::string::npos)
        {
            return KICK;
        }
        else if((pos = command.find("/mute ")) != std::string::npos)
        {
            return MUTE;
        }
        else if((pos = command.find("/unmute ")) != std::string::npos)
        {
            return UNMUTE;
        }
        else if((pos = command.find("/whois ")) != std::string::npos)
        {
            return WHOIS;
        }
        else if((pos = command.find("/ping\n")) != std::string::npos)
        {
            return PING;
        }

        pos = std::string::npos;
        return NONE;
    }


    /**
     * GetIPAddress
     * Copies to (char*) ip the value of sockfd's ip address in number and dot notation.
     * @return true in success and false in failure. Sets errno message.
     * @param int sockfd -> fd to get the ip address from, char* ip -> setted to the ip value if valid. 
    */
    bool GetIPAddress(int sockfd, char* ip)
    {
        struct sockaddr_in addr;
        socklen_t addr_size = sizeof(struct sockaddr_in);

        // Fills sockaddr_in struct with the connection's info, returns 0 on success.
        if(!getpeername(sockfd, (struct sockaddr *)&addr, &addr_size))
        {
            strcpy(ip, inet_ntoa(addr.sin_addr));
            return true;
        }
        return false;
    }

    bool checkNick(const char* str){
        int size = strlen(str);

        if(size < 2 || size > 50){
            return false;
        }

        for (int i = 0; i < size; i++){
            if(str[i] == ' '|| str[i] == '#' || str[i] == '&' || str[i] == ':'){
                return false;
            }
        }
        
        return true;
    }

    bool checkChannel(const char* str){
        int size = strlen(str);
        if(str[0] != '#' && str[0] != '&' && size < 3){
            return false;
        }

        for (int i = 1; i < size; i++){
            if(str[i] == ' ' || str[i] == ':'){
                return false;
            }
        }
        
        return true;
    }


}
#endif