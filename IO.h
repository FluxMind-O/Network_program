#ifndef IO_H
#define IO_H

#include<sys/epoll.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<fcntl.h>
#include<unistd.h>
#include<cstring>
#include<iostream>
#include<vector>
#include<unordered_map>

#define Max_events 1024
#define Buf_size 1024

//设置非阻塞IO
int set_nonblocking(int fd);

#endif
