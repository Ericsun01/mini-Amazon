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
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "world_amazon.pb.h"
#include "amazon_ups.pb.h"
#include "interact.h"
#include "frontserver.h"
#include "UPSServer.h"
#include "WorldSocket.h"
// #include "gpb_test.h"
#include "parseMsg.h"

using namespace google::protobuf::io;
using namespace std;

int main(void) {

    WorldSocket worldclient;

    worldclient.connect_server("vcm-13673.vm.duke.edu", "23456");
    cout << "fd: " << worldclient.socket_fd << endl;
    worldclient.out = new FileOutputStream(worldclient.socket_fd); // send
    worldclient.in = new FileInputStream(worldclient.socket_fd);  // 开头设定 recv

    // send gpb message for test
    AConnect a_connect;
    // a_connect.set_worldid(1);
    // cout << "world_id set!" << endl;

    AInitWarehouse * initwh = a_connect.add_initwh();
    initwh->set_id(2);
    cout << "initwh id:" << initwh->id() << endl;
    initwh->set_x(2);
    cout << "initwh x: " << initwh->x() << endl;
    initwh->set_y(3);
    cout << "initwh y: " << initwh->y() << endl;

    a_connect.set_isamazon(true);
    cout << "isamazon: " << a_connect.isamazon() << endl;

    // cout << "outstream defined" << endl;
    bool sent = sendMesgTo(a_connect, worldclient.out);
    if ( sent == false) {
        cout << "cannot send message!" << endl;
    }

    AConnected a_connected;
    if (recvMesgFrom(a_connected, worldclient.in) == false) {
        cout << "Failed to recv message!" << endl;
    }
    std::cout << a_connected.worldid() << std::endl;
    std::cout << a_connected.result() << std::endl;

    google::protobuf::ShutdownProtobufLibrary();

    return EXIT_SUCCESS;
    
}

