#ifndef SERVER_H
#define SERVER_H

#include<sys/epoll.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<fcntl.h>
#include<unistd.h>
#include<cstring>
#include<iostream>
#include<vector>
#include<unordered_map>

#include"IO.h"

class epoll_server{
    private:
      int listen_fd;
      int epoll_fd;
      struct epoll_event events_[Max_events];  //建立输出缓冲区（内核->用户态）

      //存客户端信息，fd->ip
      std::unordered_map<int,std::string>client_s;

      void handle_accept();
      void handle_read(int fd);
      void close_client(int fd, const char* reason);

    public:
      epoll_server(int port);
      void run();
      ~epoll_server();
};

#endif
