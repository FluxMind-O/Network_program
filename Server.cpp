#include"IO.cpp"

class epoll_server{
    private:
      int listen_fd;
      int epoll_fd;
      struct epoll_event events_[Max_events];  //建立输出缓冲区（内核->用户态）

      //存客户端信息，fd->ip
      std::unordered_map<int,std::string>client_s;

    public:
      epoll_server(int port){
        listen_fd = socket(AF_INET,SOCK_STREAM,0);   //创建监听socket
        if(listen_fd==-1){
           perror("socket");
           exit(EXIT_FAILURE);
        }

        //设置端口复用
        int reuse = 1;
        setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));

        //绑定地址的bind(),监听listen()
        struct sockaddr_in addr;
        memset(&addr,0,sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(port);

        if(  bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) ==-1  ){
            perror("bind");
            exit(EXIT_FAILURE);
        }

        if(  listen(listen_fd, SOMAXCONN)==-1  ){
            perror("listen");
            exit(EXIT_FAILURE);
        }

        // 1.创建epoll实例
        epoll_fd = epoll_create1(0);
        if (epoll_fd==-1){
            perror("epoll_createl");
            exit(EXIT_FAILURE);
        }

        // 2.将监听fd加入epoll(LT)
        struct epoll_event ev; 
        ev.events = EPOLLIN; //LT模式监听可读事件
        ev.data.fd = listen_fd;  //ev.data.fd存储socket, epoll_wait返回时通过这个就知道是哪个socket就绪
        epoll_ctl(epoll_fd,EPOLL_CTL_ADD,listen_fd,&ev);  //加进去

        std::cout<<"[Server] start on port "<<port<<std::endl;
      }

      //运行事件循环
      void run(){
        while(true){
            int nfds = epoll_wait(epoll_fd,events_,Max_events,-1);
            
            for (int i = 0; i < nfds; i++){
              int fd = events_[i].data.fd;
              uint32_t ev = events_[i].events;
              
              if(fd==listen_fd){ //新连接
                handle_accept();
              }
              else if(ev & EPOLLIN){ //可读事件(客户端在发信息)
                handle_read(fd);
              }
              else if(ev & EPOLLOUT){ //可写时间(服务器给客户端发信息)
                //暂时不要
              }
              else if(ev & (EPOLLERR|EPOLLHUP)){
                close_client(fd,"Error/Hangup");
              }
            }
        }
      }
       

      private:
      
        void handle_accept(){}
        void handle_read(int fd){}
        void close_client(int fd,const char* reason){}



};//