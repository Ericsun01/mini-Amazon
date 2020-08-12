#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <pqxx/pqxx>

using namespace std;
using namespace pqxx;


/*
int main(void) {

    connection *C;
    FrontServer frontserver;

    connectDB(C);

    frontserver.buildServer("6666");
    int client_fd = frontserver.acceptConnection();
    char buffer[128];
    int len = recv(client_fd, buffer, sizeof(buffer), 0);
    if (len == -1) {
        perror("recv");
    }
    buffer[len] = 0;
    // DEBUG
    cout << "Server received: " << buffer << endl;

    

    //TODO: Close database connection
    // C->disconnect();


    return EXIT_SUCCESS;
}*/