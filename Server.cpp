#include"Server.h"
#include <arpa/inet.h>  // 是inet_ntop 所需要的

epoll_server::epoll_server(int port){
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
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(  bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) ==-1  ){
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if(  listen(listen_fd, SOMAXCONN)==-1  ){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    set_nonblocking(listen_fd);  //  AI改的错(根因是 accept 循环与 socket 阻塞模式不匹配：),有点迷糊了

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
void epoll_server::run(){
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
          else if(ev & EPOLLOUT){ //可写事件(服务器给客户端发信息)
            //暂时不要
          }
          else if(ev & (EPOLLERR|EPOLLHUP)){
            close_client(fd,"Error/Hangup");
          }
        }
    }
}

void epoll_server::handle_accept() {
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    
    // LT模式下循环accept直到返回-1且errno==EAGAIN
    while (true) {
        int conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &len);
        if (conn_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // 所有连接处理完毕
            } 
            else {
                perror("accept");
                break;
            }
        }
        
        // 设置为非阻塞并加入epoll
        set_nonblocking(conn_fd);
        struct epoll_event ev;
        ev.events = EPOLLIN|EPOLLET ; // LT模式给客户端(本来想用ET的)
        ev.data.fd = conn_fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev);
        
        // 记录客户端信息
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
        client_s[conn_fd] = std::string(ip) + ":" + std::to_string(ntohs(client_addr.sin_port));
        
        std::cout << "[Connect] " << client_s[conn_fd] 
                  << " (fd=" << conn_fd << ")" << std::endl;
    }
}

void epoll_server::handle_read(int fd) {
    std::cout << "[DEBUG handle_read] fd=" << fd << " client_s=" << (client_s.count(fd) ? client_s[fd] : "unknown") << std::endl;   //测试BUG
    char buf[Buf_size];
    
    ssize_t n = read(fd, buf, sizeof(buf) - 1);  // LT模式下读,等效于send()
    std::cout << "[DEBUG read] n=" << n << " errno=" << errno << std::endl;

    if (n > 0) {
        buf[n] = '\0';
        std::cout << "[Recv] fd=" << fd << " (" << client_s[fd] 
                  << "): " << buf << std::endl << std::flush;
        
        // Echo回写（简单处理：假设一次write能写完）
        ssize_t sent = write(fd, buf, n);
        if (sent == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                close_client(fd, "Write error");
                return;
            }
        }
        std::cout << "[Send] fd=" << fd << " echoed " << sent << " bytes" << std::endl;
        
    } 
    else if (n == 0) {
        // 客户端主动关闭（发送FIN）
        close_client(fd, "Client closed");
    } 
    else { // n == -1
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            close_client(fd, "Read error");
        }
    }
}

void epoll_server::close_client(int fd, const char* reason) {
    std::cout << "[Disconnect] fd=" << fd 
              << " (" << client_s[fd] << ") Reason: " 
              << reason << std::endl;
    
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
    client_s.erase(fd);
}

epoll_server::~epoll_server() {
    close(epoll_fd);
    close(listen_fd);
}
