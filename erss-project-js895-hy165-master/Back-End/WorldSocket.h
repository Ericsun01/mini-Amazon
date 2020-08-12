#ifndef _WORLDSOCKET_
#define _WORLDSOCKET_

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
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace std;
using namespace google::protobuf::io;

class WorldSocket {

  public:

    struct addrinfo host_info, *host_info_list;
    int socket_fd; 
    int status;
    FileOutputStream * out;
    FileInputStream * in;


    WorldSocket() {

    }

  public:
    int connect_server(const char *hostname, const char *port);
    void send_init_msg();

    // destructor
    ~WorldSocket() {
        delete out;
        delete in;
        close(socket_fd);
    }
  
};

int WorldSocket::connect_server(const char *hostname, const char *port) {

    memset(&host_info, 0, sizeof(struct addrinfo));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        std::cerr << "cannot get address info for host" << std::endl;
        std::cout << "  (" << hostname << "," << port << ")" << std::endl;
        exit(EXIT_FAILURE);
    } // if

    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                    host_info_list->ai_protocol);
    if (socket_fd == -1) {
        std::cerr << "cannot create socket" << std::endl;
        std::cout << "  (" << hostname << "," << port << ")" << std::endl;
        exit(EXIT_FAILURE);
    } // if

    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        std::cerr << "cannot connect to socket" << std::endl;
        std::cout << "  (" << hostname << "," << port << ")" << std::endl;
        exit(EXIT_FAILURE);
    } // if

    freeaddrinfo(host_info_list);

    // init gpb stream
    out = new FileOutputStream(socket_fd);
    in = new FileInputStream(socket_fd);

    // cout << "world connected!" << endl;

    return socket_fd;
}

void WorldSocket::send_init_msg() {
    const char *message = "hi there!";
    send(socket_fd, message, strlen(message), 0);
    cout << "init msg sent" << endl;
}

#endif