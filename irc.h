#ifndef _IRC_H_
#define _IRC_H_

#define BUFFER_SIZE 4096
#define DEBUG_MODE false

namespace IRC{

void error(std::string msg)
{
    perror(msg.c_str());
    exit(EXIT_FAILURE);
}

void format_message(bool ACK, std::string& msg)
{
    if(ACK){
        msg = "0"+msg;
    }
    else{
        msg = "1"+msg;
    }
    std::cout << msg << std::endl;
}

}
#endif