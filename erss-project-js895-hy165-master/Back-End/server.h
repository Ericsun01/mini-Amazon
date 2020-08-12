#ifndef _SERVER_
#define _SERVER_

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

using namespace std;

class Server {

  public:

    struct addrinfo host_info, *host_info_list;
    int newfd;
    int sockfd; 
    int status;

    Server() {
        cout << "super server constructed" << endl;
    }

    Server (const char * port) {
        // // initialize
        // memset(&host_info, 0, sizeof(host_info));
        // host_info.ai_family = AF_UNSPEC;
        // host_info.ai_socktype = SOCK_STREAM;
        // host_info.ai_flags = AI_PASSIVE;

        // cout << "start getting addrinfo" << endl;
        // status = getaddrinfo(NULL, port, &host_info, &host_info_list);
        // if (status != 0) {
        //     std::cerr << "Error: cannot get address info for host" << std::endl;
        //     exit(EXIT_FAILURE);
        // }
        // // create socket
        // cout << "enter createsocket" << endl;
        // sockfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
        //                     host_info_list->ai_protocol);
        // if (sockfd == -1) {
        //     std::cerr << "cannot create socket" << std::endl;
        //     exit(EXIT_FAILURE);
        // } 

        // int yes = 1;
        // status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        // status = bind(sockfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
        // if (status == -1) {
        //     std::cerr << "cannot bind socket" << std::endl;
        //     exit(EXIT_FAILURE);
        // } 

        // status = listen(sockfd, 100);
        // if (status == -1) {
        //     std::cerr << "cannot listen on socket" << std::endl;
        //     exit(EXIT_FAILURE);
        // }
        // // debug
        // cout << "Waiting for connection..." << endl; 

        // freeaddrinfo(host_info_list);

    }

    void initialize(const char *_port);
    void createSocket();
    int acceptConnection();

    void buildServer(const char *port);

    ~Server() {
        close(sockfd);
    }

};

void Server::initialize(const char *_port) {
    cout << "enter server initialize" << endl;
    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;

    cout << "start getting addrinfo" << endl;
    status = getaddrinfo(NULL, _port, &host_info, &host_info_list);
    if (status != 0) {
        std::cerr << "Error: cannot get address info for host" << std::endl;
        exit(EXIT_FAILURE);
    }
    cout << "addrinfo get" << endl;
}


void Server::createSocket() {
    cout << "enter createsocket" << endl;
    sockfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                        host_info_list->ai_protocol);
    if (sockfd == -1) {
        std::cout << "cannot create socket" << std::endl;
        exit(EXIT_FAILURE);
    } 

    int yes = 1;
    status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(sockfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        std::cout << "cannot bind socket" << std::endl;
        exit(EXIT_FAILURE);
    } 

    status = listen(sockfd, 100);
    if (status == -1) {
        std::cout << "cannot listen on socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    // debug
    cout << "Waiting for connection..." << endl; 

    freeaddrinfo(host_info_list);
}

int Server::acceptConnection() {
    //int newfd;
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    newfd = accept(sockfd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (newfd == -1) {
        std::cerr << "Error: cannot accept connection on socket" << std::endl;
        exit(EXIT_FAILURE);
    } // if

    return newfd;
}

void Server::buildServer(const char *port) {
    // debug
    cout << "enter buildserver" << endl;
    initialize(port);
    createSocket();
}


#endif