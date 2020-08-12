#include <arpa/inet.h>
#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <thread>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <functional>
#include <sys/time.h>
#include <pqxx/pqxx>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "world_amazon.pb.h"
#include "amazon_ups.pb.h"
#include "frontserver.h"
#include "UPSServer.h"
#include "WorldSocket.h"
#include "parseMsg.h"

using namespace google::protobuf::io;
using namespace std;
using namespace pqxx;

/**** helper functions ****/

/*
*** update the order status in database
*/
void update_status(connection *C, int ship_id, string status) {
    stringstream sbuffer;
    string sql;
    work W(*C);

    sbuffer << "UPDATE amazon_orders ";
    sbuffer << "SET status = " << W.quote(status) << " ";
    sbuffer << "WHERE status = " << ship_id << ";";

    sql = sbuffer.str();

    W.exec(sql);
    W.commit();

}
/*
*** set truckid according to shipid in database
*/
void setTruckid(connection *C, int ship_id, int truck_id) {

    stringstream sbuffer;
    string sql;
    work W(*C);

    sbuffer << "UPDATE amazon_orders ";
    sbuffer << "SET truck_id = " << truck_id << " ";
    sbuffer << "WHERE ship_id = " << ship_id << ";";

    sql = sbuffer.str();

    W.exec(sql);

}
/*
*** set shipid in database
*/
void update_shipid(int order_id, int ship_id) {

    stringstream sbuffer;
    string sql;
    work W(*C);

    sbuffer << "UPDATE amazon_orders ";
    sbuffer << "SET ship_id = " << ship_id << " ";
    sbuffer << "WHERE id = " << order_id << ";";

    sql = sbuffer.str();

    W.exec(sql);   
    
}
/*
*** get order information based on order id
*/
std::tuple<int, string, int> getOrderInfo(connection *C, int order_id) {

    nontransaction N(*C);
    string sql;
    stringstream sbuffer;

    sbuffer << "SELECT whnum, description, count ";
    sbuffer << "FROM amazon_orders, amazon_products ";
    sbuffer << "WHERE amazon_orders.description = amazon_products.name ";
    sbuffer << "AND amazon_orders.id = " << order_id << ";";

    sql = sbuffer.str();

    /* Execute SQL query */
    result R(N.exec(sql));

    int whnum;
    string description;
    int count;
    // print our headers
    cout << "whnum " << "description" << "count" << endl;
    // print out results
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        whnum = c[0].as<int>();
        description = c[1].as<string>();
        count = c[2].as<int>();

        cout << "whnum = " << whnum << endl;
        cout << "description = " << description << endl;
        cout << "count = " << count << endl;
    }
    std::tuple<int, string, int> order_info;
    order_info = make_tuple(whnum, description, count);

    return order_info;
}
/*
*** get truck id by order id
*/
int getTruckidByShipid(connection *C, int ship_id) {

    nontransaction N(*C);
    string sql;
    stringstream sbuffer;

    sbuffer << "SELECT truck_id ";
    sbuffer << "FROM amazon_orders ";
    sbuffer << "WHERE ship_id = " << ship_id << ";";

    sql = sbuffer.str();
    result R(N.exec(sql));

    int truck_id;
    // print out results
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        truck_id = c[0].as<int>();
        cout << "truck_id = " << truck_id << endl;
    }    
    return truck_id;
}
/*
*** get shipid based on primary key id
*/
vector<int> getShipidById(connection *C, int id) {

    nontransaction N(*C);
    string sql;
    stringstream sbuffer;

    sbuffer << "SELECT ship_id, ship_addr_x, ship_addr_y ";
    sbuffer << "FROM amazon_orders ";
    sbuffer << "WHERE id = " << id << ";";

    sql = sbuffer.str();
    result R(N.exec(sql));

    int ship_id;
    int ship_addr_x;
    int ship_addr_y;
    // print out results
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        ship_id = c[0].as<int>();
        ship_addr_x = c[1].as<int>();
        ship_addr_y = c[2].as<int>();
        cout << "ship_id = " << ship_id << endl;
        cout << "ship_addr_x = " << ship_addr_x << endl;
        cout << "ship_addr_y = " << ship_addr_y << endl;

    }  
    vector<int> order_info;
    order_info.push_back(ship_id);
    order_info.push_back(ship_addr_x);
    order_info.push_back(ship_addr_y);

    return order_info;
}
/*
*** get upsusername based on ship id
*/
string getUPSusername(connection *C, int ship_id) {

    nontransaction N(*C);
    string sql;
    stringstream sbuffer;

    sbuffer << "SELECT amazon_amazonuser.ups_username ";
    sbuffer << "FROM amazon_orders, amazon_amazonuser ";
    sbuffer << "WHERE amazon_orders.ship_id = " << ship_id << ";";

    sql = sbuffer.str();
    result R(N.exec(sql));

    string ups_username;
    // print out results
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        ups_username = c[0].as<string>();
        cout << "ship_id = " << ups_username << endl;
    }  
    return ups_username;
}


/**** handler functions ****/

/*
*** connect to the world
*/
void connectToWorld(int world_id, int initwh_id, FileOutputStream * out) {
    AConnect a_connect;
    // a_connect.set_worldid(world_id);
    cout << "world_id set!" << endl;

    AInitWarehouse * initwh = a_connect.add_initwh();
    cout << "initwh init" << endl;
    initwh->set_id(initwh_id);
    cout << "initwh id set:" << initwh->id() << endl;
    initwh->set_x(2);
    cout << "initwh x set: " << initwh->x() << endl;
    initwh->set_y(3);
    cout << "initwh y set: " << initwh->y() << endl;

    a_connect.set_isamazon(true);
    cout << "isamazon set: " << a_connect.isamazon() << endl;

    // send message
    bool sent = sendMesgTo(a_connect, out);
    if ( sent == false) {
        cout << "cannot send message!" << endl;
    }
 
    google::protobuf::ShutdownProtobufLibrary();
}
/*
*** recv AConnected from the world
*/
bool connectedToWorld(FileInputStream * in) {
    AConnected a_connected;
    while (recvMesgFrom(a_connected, in) == false) {
        // wait
    }

    std::cout << "world_id = " << a_connected.worldid() << std::endl;
    std::cout << a_connected.result() << std::endl;
    
    if (a_connected.result().compare("connected!")) {
        return true;
    }
    else {
        return false;
    }    
}
/*
*** handle new orders: send APurchaseMore
*/
void handleNewOrders(int order_id, FileOutputStream * out) {

    std::tuple<int, string, int> order_info = getOrderInfo(C, order_id);
    int whnum;
    string description;
    int count;
    tie (whnum, description, count) = order_info;
    // pack to APurchaseMore and acommands
    ACommands acommand;   
    APurchaseMore * ap = acommand.add_buy();
    ap->set_whnum(whnum); // set whnum
    ap->set_seqnum(seqnum); // set seqnum
    // set products
    AProduct * product = ap->add_things();
    product->set_id(order_id);
    product->set_description(description);
    product->set_count(count);

    if (sendMesgTo(acommand, out) == false) { // send ack
        cout << "cannot send APurchaseMore!" << endl;
    }
    else {
        cout << "APurchaseMore sent!" << endl;
    }

    // send to world
}
/*
*** handle APurchaseMore: send APack to world, send AUReqtruck to ups
*/
void handleAPurchaseMore(APurchaseMore arrived, FileOutputStream * worldout, FileOutputStream * upsout) {
    // goRequestTruck.push(arrived); // enqueue for ups socket to handle
    ::google::protobuf::int32 whnum = arrived.whnum();
    vector<AProduct> things;
    for (int j = 0; j < arrived.things_size(); j++) {
        things.push_back(arrived.things(j));
    }    
    // send Apack to world
    ACommands acommand;
    // set APack
    APack* apack = acommand.add_topack();
    apack->set_whnum(whnum); // set whnum
    AProduct * thing = apack->add_things();
    thing->set_id(things[0].id());
    thing->set_description(things[0].description());
    thing->set_count(things[0].count());            
    /* assign shipid */
    apack->set_shipid(shipid); 
    shipid++;
    update_shipid(thing->id(), shipid); // in database
    apack->set_seqnum(seqnum); // set seqnum
    seqnum++;
    // set the seq of arrived as ack
    acommand.add_acks(arrived.seqnum());

    if (sendMesgTo(acommand, worldout) == false) { // send ack
        cout << "cannot send APack!" << endl;
    }
    else {
        cout << "APack sent!" << endl;
    }

    // send AUReqTruck to ups
    // APurchaseMore ap = goRequestTruck.front();
    // goRequestTruck.pop();
    // get whnum
    AProduct product = arrived.things(0);
    int id = product.id();
    string description = product.description();
    // get shipid from database
    vector<int> order_info = getShipidById(C, id);
    int ship_id = order_info[0];
    int ship_addr_x = order_info[1];
    int ship_addr_y = order_info[2];
    string ups_username = getUPSusername(C, ship_id);

    AUCommands aucommand;
    AUReqTruck * req_truck = aucommand.add_requests();
    req_truck->set_warehouseid(whnum); // set whnum
    req_truck->set_shipid(ship_id); // set shipid
    req_truck->set_seqnum(seqnum); // set seqnum
    seqnum++;
    // set orders
    AUOrder * orders;
    orders->set_description(description);
    orders->set_locationx(ship_addr_x);
    orders->set_locationy(ship_addr_y);
    orders->set_username(ups_username);
    req_truck->set_allocated_orders(orders);

    if (sendMesgTo(aucommand, upsout) == false) { // send ack
    cout << "cannot send AUReqTruck!" << endl;
    }
    else {
        cout << "AUReqTruck sent!" << endl;
    } 

}
/*
*** handle APacked: send ack
*/
void handleAPacked(APacked ready, FileOutputStream * out) {
    ACommands acommand;

    int ship_id = ready.shipid();
    orderPackedStatus[ship_id] = 1; // set flag to 1
    // set the seq of ready as ack
    acommand.add_acks(ready.seqnum());

    if (sendMesgTo(acommand, out) == false) { // send ack
        cout << "cannot send ack for APacked!" << endl;
    }
    else {
        cout << "Ack for APacked sent!" << endl;
    }
}
/*
*** handle ALoaded: send ack to world, send AUTruckloaded to ups
*/
void handleALoaded(ALoaded loaded, FileOutputStream * worldout, FileOutputStream * upsout) {
    ACommands acommand;
    // truckLoaded.push(loaded); // enqueue for ups socket to handle
    // set the seq of ready as ack
    acommand.add_acks(loaded.seqnum());

    if (sendMesgTo(acommand, worldout) == false) { // send ack
        cout << "cannot send ack for ALoaded!" << endl;
    }
    else {
        cout << "Ack for ALoaded sent!" << endl;
    }

    // send AUTruckloaded to ups
    int ship_id = loaded.shipid();
    int truck_id = getTruckidByShipid(C, ship_id);

    AUCommands aucommand;
    AUTruckLoaded * autruck_loaded = aucommand.add_truckloaded();
    autruck_loaded->set_truckid(truck_id);
    autruck_loaded->set_shipid(shipid);
    autruck_loaded->set_seqnum(seqnum);
    seqnum++;

    if (sendMesgTo(aucommand, upsout) == false) { // send ack
    cout << "cannot send AUTruckLoaded!" << endl;
    }
    else {
        cout << "AUTruckLoaded sent!" << endl;
    }
}
/*
*** handle ALoaded: send APutOnTruck to world
*/
void handleAPutOnTruck(FileOutputStream * out) {
    UATruckArrived truck_arrived = readyToLoad.front(); // get the first item in queue
    readyToLoad.pop();
    int ship_id = truck_arrived.shipid(); // get ship id
    if (orderPackedStatus[ship_id] == 1) { // if already packed
        ACommands acommand;
        // set APutOnTruck
        APutOnTruck* load = acommand.add_load();
        load->set_whnum(1); // set whnum to 1
        load->set_truckid(truck_arrived.truckid()); // set shipid
        load->set_shipid(ship_id);
        load->set_seqnum(seqnum); // set seqnum
        seqnum++;
        // set the seq of arrived as ack
        acommand.add_acks(truck_arrived.seqnum());

        if (sendMesgTo(acommand, out) == false) { // send ack
            cout << "cannot send APutOnTruck!" << endl;
        }
        else {
            cout << "APutOnTruck sent!" << endl;
        }
    }
    else { // if haven't packed yet, put it back and keep waiting
        readyToLoad.push(truck_arrived);
    }
}

// // query
// AQuery setAQuery(int packageid, int seqnum) {

// }

/*
*** handle APackage: update status in database
*/
void handleAPackage(APackage packagestatus, FileOutputStream * out) {
    int ship_id = packagestatus.packageid(); // get shipid
    string status = packagestatus.status(); // get status
    // renew database
    update_status(C, ship_id, status);
    ACommands acommand;
    acommand.add_acks(packagestatus.seqnum());

    if (sendMesgTo(acommand, out) == false) { // send ack
        cout << "cannot send ack for APackage!" << endl;
    }
    else {
        cout << "Ack for APackage sent!" << endl;
    }
}

// AErr
// ACommands handleAErr(AErr aerror, FileOutputStream * out) {

// }

/*
*** handle acks: set flag to 1, indicating the message has been recv by world
*/
void handleAcks(::google::protobuf::int64 ack, FileOutputStream * out) {
    map<int, int>::iterator it;
    if ((it = ackRecv.find(ack)) != ackRecv.end()) { // if found seqnum
        it->second = 1; // set flag to 1
    }
    else {
        cerr << "seqnum does not exist!" << endl;
    }
}


/************ Amazon - UPS *************/

/*
*** handle UATruckArrived: send ack to the world, push into "readyToLoad" queue
*/
void handleUATruckArrived(UATruckArrived truck_arrived, FileOutputStream * out) {
    int ship_id = truck_arrived.shipid();
    int truck_id = truck_arrived.truckid();
    setTruckid(C, ship_id, truck_id); // set truckid in database
    readyToLoad.push(truck_arrived);

    AUCommands aucommand;
    // set the seq as ack
    aucommand.add_acks(truck_arrived.seqnum());

    if (sendMesgTo(aucommand, out) == false) { // send ack
        cout << "cannot send ack for UATruckArrived!" << endl;
    }
    else {
        cout << "Ack for UATruckArrived sent!" << endl;
    }
}
/*
*** handle UAPackageArrived: send ack to the world, update order status to 1
*/
void handleUAPackageArrived(UAPackageArrived package_arrived, FileOutputStream * out) {
    int ship_id = package_arrived.shipid();
    update_status(C, ship_id, "delivered");
    AUCommands aucommand;
    // set the seq as ack
    aucommand.add_acks(package_arrived.seqnum());

    if (sendMesgTo(aucommand, out) == false) { // send ack
    cout << "cannot send ack for UAPackageArrived!" << endl;
    }
    else {
        cout << "Ack for UAPackageArrived sent!" << endl;
    }
}

void handleAUReqTruck(FileOutputStream * out) {
    APurchaseMore ap = goRequestTruck.front();
    goRequestTruck.pop();
    // get whnum
    int whnum = ap.whnum();
    AProduct thing = ap.things(0);
    int id = thing.id();
    string description = thing.description();
    // get shipid from database
    vector<int> order_info = getShipidById(C, id);
    int ship_id = order_info[0];
    int ship_addr_x = order_info[1];
    int ship_addr_y = order_info[2];
    string ups_username = getUPSusername(C, ship_id);

    AUCommands aucommand;
    AUReqTruck * req_truck = aucommand.add_requests();
    req_truck->set_warehouseid(whnum); // set whnum
    req_truck->set_shipid(ship_id); // set shipid
    req_truck->set_seqnum(seqnum); // set seqnum
    seqnum++;
    // set orders
    AUOrder * orders;
    orders->set_description(description);
    orders->set_locationx(ship_addr_x);
    orders->set_locationy(ship_addr_y);
    orders->set_username(ups_username);
    req_truck->set_allocated_orders(orders);

    if (sendMesgTo(aucommand, out) == false) { // send ack
    cout << "cannot send AUReqTruck!" << endl;
    }
    else {
        cout << "AUReqTruck sent!" << endl;
    } 
}

void handleAUTruckLoaded(FileOutputStream * out) {
    ALoaded loaded = truckLoaded.front();
    truckLoaded.pop();
    int ship_id = loaded.shipid();
    int truck_id = getTruckidByShipid(C, ship_id);

    AUCommands aucommand;
    AUTruckLoaded * autruck_loaded = aucommand.add_truckloaded();
    autruck_loaded->set_truckid(truck_id);
    autruck_loaded->set_shipid(shipid);
    autruck_loaded->set_seqnum(seqnum);
    seqnum++;

    if (sendMesgTo(aucommand, out) == false) { // send ack
    cout << "cannot send AUTruckLoaded!" << endl;
    }
    else {
        cout << "AUTruckLoaded sent!" << endl;
    }

}

// Errors
// UAErr setError(string error, int org_seqnum, int seqnum);
// void handleError(UAErr uaerror, FileOutputStream * out) {
// }

/*
*** connect to database
*/
void connectDB(connection *C) {
    try {
        //Establish a connection to the database
        //Parameters: database name, user name, user password
        C = new connection("dbname=erss_amazon user=postgres password=ece568");
        if (C->is_open()) {
            cout << "Opened database successfully: " << C->dbname() << endl;
        } else {
            cout << "Can't open database" << endl;
        }
    } catch (const std::exception &e){
        cerr << e.what() << std::endl;
    }
}
/*
*** frontsocket thread
*/
void front() {
// void front(FrontServer & frontserver) {
    // debug
    cout << "Entered front thread" << endl;
    int fd;
    int order_id;
    std::mutex mtx;

    // fd = frontserver.acceptConnection();
   
    // string buff;
    while (true) {
        while (true) {
            fd = frontserver.acceptConnection();
            // debug
            cout << "listening to front..." << endl;
            char buffer[128];
            int len = recv(fd, buffer, sizeof(buffer), 0);
            if (len == -1) {
                perror("recv");
            }
            if (len > 0) {
                buffer[len] = 0;
                // DEBUG
                cout << "Server received: " << buffer << endl;
                order_id = stoi(buffer);
                mtx.lock();
                newOrders.push(order_id);
                // DEBUG
                cout << "Push into queue: " << newOrders.back() << endl;                
                mtx.unlock();
                break;
            }            
        }
    }
}
/*
*** the thread listening to world and ups
*/               
void back() {

    // debug
    cout << "Entered back thread" << endl;

    int world_id;
    int initwh_id = 2;

    int ups_fd = upsServer.acceptConnection();
    upsServer.out = new FileOutputStream(ups_fd);
    upsServer.in = new FileInputStream(ups_fd);


    // /*
    //  1. recv worldbuilt 
    // */
     UAWorldBuilt world_built;
     while (recvMesgFrom(world_built, upsServer.in) == false) {
         // wait    
     }
     world_id = world_built.worldid();
     // debug
     cout << "recv world_id = " << world_id << endl;

     AUCommands aucommand;
     // set the seq as ack
     aucommand.add_acks(world_built.seqnum());
     if (sendMesgTo(aucommand, upsServer.out) == false) { // send ack
         cout << "cannot send Aconnect!" << endl;
     }

    /*
     2. connect to world 
    */
    connectToWorld(world_id, 1, worldclient.out);
    //connectToWorld(3, 1, worldclient.out); // test; remove later
    if (connectedToWorld(worldclient.in) == true) {
        cout << "Connected to world!" << endl;
    }


    /*
     3. general responses 
    */

    while(true) { 
        cout << "enter Aresponse loop" << endl;
        /* 
          communication with world 
        */
        AResponses aresponse; 
        cout << "Aresponse init" << endl;       
        // if new order comes in
        while (!newOrders.empty()) {
            cout << "enter new order loop" << endl;  
            int order_id = newOrders.front();
            newOrders.pop();
            // debug
            cout << "pop from queue: " << order_id << endl;
            cout << "" << endl;
            thread (handleNewOrders, order_id, worldclient.out).detach();         
        }    
        cout << "currently no new orders" << endl;  

        if (recvMesgFrom(aresponse, worldclient.in) == true) { // if recv aresponse
            cout << "new aresponse" << endl; 
            for (int i = 0; i < aresponse.acks_size(); i++) {
                ::google::protobuf::int64 ack = aresponse.acks(i);
                thread (handleAcks, ack, worldclient.out).detach();
            }
            // handle APurchaseMore
            for (int i = 0; i < aresponse.arrived_size(); i++) {
                APurchaseMore arrived = aresponse.arrived(i);
                thread (handleAPurchaseMore, arrived, worldclient.out, upsServer.out).detach();
            }
            // handle APacked
            for (int i = 0; i < aresponse.ready_size(); i++) {
                APacked ready = aresponse.ready(i);
                thread (handleAPacked, ready, worldclient.out).detach();

            }
            // handle ALoaded
            for (int i = 0; i < aresponse.loaded_size(); i++) {
                ALoaded loaded = aresponse.loaded(i);
                thread (handleALoaded, loaded, worldclient.out, upsServer.out).detach();
            }
            // handle APackage
            for (int i = 0; i < aresponse.packagestatus_size(); i++) {
                APackage packagestatus = aresponse.packagestatus(i);
                thread (handleAPackage, packagestatus, worldclient.out).detach();
            }
             // handle AErr
            // for (int i = 0; i < aresponse.error_size(); i++) {
            //     AErr error = aresponse.error(i);
            //     thread (handleAErr, error, worldclient.out).detach();
            // }
        }

        cout << "currently no aresponse" << endl;

        while (!readyToLoad.empty()) {
            std::thread (handleAPutOnTruck, worldclient.out).detach(); // send APutOnTruck
        }

        cout << "currently nothing ready to load" << endl;
              
        /* 
         communication with ups 
        */
        UACommands uacommand;
        while (recvMesgFrom(uacommand, worldclient.in) == true) { // if received from ups

            for (int i = 0; i < uacommand.acks_size(); i++) {
                ::google::protobuf::int64 ack = uacommand.acks(i);
                std::thread (handleAcks, ack, upsServer.out).detach();
                // handleAcks(ack, ups.out);
            }
            // handle UATruckArrived
            for (int i = 0; i < uacommand.truckarrived_size(); i++) {
                UATruckArrived truckarrived = uacommand.truckarrived(i);
                std::thread (handleUATruckArrived, truckarrived, upsServer.out).detach();
                // handleUATruckArrived(truckarrived, ups.out);
            }
            // handle UAPackageArrived
            for (int i = 0; i < uacommand.packagearrived_size(); i++) {
                UAPackageArrived packagearrived = uacommand.packagearrived(i);
                std::thread (handleUAPackageArrived, packagearrived, upsServer.out).detach();
                // handleUAPackageArrived(packagearrived, ups.out);
            }
            //  handle AErr
            // for (int i = 0; i < uacommand.uaerror_size(); i++) {
            //     UAErr error = uacommand.uaerror(i);
            //     handleError(error, upsServer.out);
            // }

        }

        //cout << "currently no uacommand" << endl;
        // while (!goRequestTruck.empty()) {
        //     std::thread (handleAUReqTruck, upsServer.out).detach();
             // std::thread (goRequestT).detach();
        // }
        // while (!truckLoaded.empty()) {
        //     std::thread (handleAUTruckLoaded, upsServer.out).detach();
             // std::thread (truckL).detach();
        // }
    }
}


int main (int argc, char **argv) {

    connectDB(C);

    int fd = worldclient.connect_server("vcm-13673.vm.duke.edu", "23456");
      
    /******** test code area *********/

    /******** test code area *********/
   

    frontserver.buildServer("5678");
    upsServer.buildServer("43210");

    std::thread th1 (front);
    th1.detach();
    std::thread th2 (back);
    th2.detach();

    while (true) {}  
      
    return EXIT_SUCCESS;
 
}