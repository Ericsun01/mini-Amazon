#ifndef _FRONTSERVER_
#define _FRONTSERVER_

#include <arpa/inet.h>
#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <sys/time.h>
#include <thread>
#include <queue>
#include <mutex>
#include <fstream>
#include <time.h>

// #include "server.h"

using namespace std;

class FrontServer {

  public:
    struct addrinfo host_info, *host_info_list;
    int newfd;
    int sockfd; 
    int status;

    FrontServer() {}

    void initialize(const char *_port);
    void createSocket();
    int acceptConnection();

    void buildServer(const char *port);
    
    // void recv_order_id() {
    //     int client_fd = acceptConnection();
    //     char buffer[128];
    //     int len = recv(client_fd, buffer, sizeof(buffer), 0);
    //     if (len == -1) {
    //         perror("recv");
    //     }
    //     buffer[len] = 0;
    //     // DEBUG
    //     cout << "Server received: " << buffer << endl;
    // }

    // destructor
    ~FrontServer() {
        // close(sockfd);
    }

};

void FrontServer::initialize(const char *_port) {
    cout << "enter FrontServer initialize" << endl;
    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;

    cout << "Front server start getting addrinfo" << endl;
    status = getaddrinfo(NULL, _port, &host_info, &host_info_list);
    if (status != 0) {
        std::cerr << "Error: FrontServer cannot get address info for host" << std::endl;
        exit(EXIT_FAILURE);
    }
    cout << "addrinfo get" << endl;
}


void FrontServer::createSocket() {
    cout << "Front server enter createsocket" << endl;
    sockfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                        host_info_list->ai_protocol);
    if (sockfd == -1) {
        std::cerr << "FrontServer cannot create socket" << std::endl;
        exit(EXIT_FAILURE);
    } 

    int yes = 1;
    status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(sockfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        std::cerr << "FrontServer cannot bind socket" << std::endl;
        exit(EXIT_FAILURE);
    } 

    status = listen(sockfd, 100);
    if (status == -1) {
        std::cerr << "FrontServer cannot listen on socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    // debug
    cout << "Front server Waiting for connection..." << endl; 

    freeaddrinfo(host_info_list);
}

int FrontServer::acceptConnection() {
    //int newfd;
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    newfd = accept(sockfd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (newfd == -1) {
        std::cerr << "Error: FrontServer cannot accept connection on socket" << std::endl;
        exit(EXIT_FAILURE);
    } // if

    return newfd;
}

void FrontServer::buildServer(const char *port) {
    // debug
    cout << "enter FrontServer" << endl;
    initialize(port);
    createSocket();
}


#endif
