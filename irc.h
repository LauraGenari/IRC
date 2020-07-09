#ifndef _IRC_H_
#define _IRC_H_

#define BUFFER_SIZE 4096
#define DEBUG_MODE true

namespace IRC{

void error(std::string msg){
    perror(msg.c_str());
    exit(EXIT_FAILURE);
}

}
#endif