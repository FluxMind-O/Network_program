#include "Server.h"



int main(int argc,char* argv[]){
    int port = (argc > 1) ? atoi(argv[1]) :8080;

    epoll_server server (port);
    server.run();

    return 0;

}
