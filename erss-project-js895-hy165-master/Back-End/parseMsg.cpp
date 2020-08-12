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
#include <tuple>
#include <mutex>
#include <fstream>
#include <sstream>
#include <time.h>
#include <pqxx/pqxx>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>

#include "world_amazon.pb.h"
#include "amazon_ups.pb.h"
#include "parseMsg.h"


using namespace std;
using namespace google::protobuf::io;
using namespace pqxx;

// // global variables
// connection *C; // postgresql database connection
// std::atomic <int> seqnum(0); // increment by 1
// std::atomic <int> shipid(0); // increment by 1
// map<int, int> ackRecv;

// queue <int> newOrders;
// queue <APurchaseMore> goRequestTruck;
// queue <ALoaded> truckLoaded;
// queue <UATruckArrived> readyToLoad;
// map<int, int> orderPackedStatus;

/**** helper functions ****/

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

void connectToWorld(int world_id, int initwh_id, FileOutputStream * out) {
    AConnect a_connect;
    a_connect.set_worldid(world_id);
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



// product
// AProduct* setAProduct(int id, string description, int count) {

//     AProduct* thing;
//     thing->set_id(id);
//     thing->set_description(description);
//     thing->set_count(count);

//     return thing;
// }

// purchaseMore
void handleNewOrders(FileOutputStream * out) {
    int order_id = newOrders.front();
    newOrders.pop();
    // TODO: find info in database
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

void handleAPurchaseMore(APurchaseMore arrived, FileOutputStream * out) {
    goRequestTruck.push(arrived); // enqueue for ups socket to handle
    ::google::protobuf::int32 whnum = arrived.whnum();
    vector<AProduct> things;
    for (int j = 0; j < arrived.things_size(); j++) {
        things.push_back(arrived.things(j));
    }    
    // put into acommand
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

    if (sendMesgTo(acommand, out) == false) { // send ack
        cout << "cannot send APack!" << endl;
    }
    else {
        cout << "APack sent!" << endl;
    }
}

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

void handleALoaded(ALoaded loaded, FileOutputStream * out) {
    ACommands acommand;
    truckLoaded.push(loaded); // enqueue for ups socket to handle
    // set the seq of ready as ack
    acommand.add_acks(loaded.seqnum());

    if (sendMesgTo(acommand, out) == false) { // send ack
        cout << "cannot send ack for ALoaded!" << endl;
    }
    else {
        cout << "Ack for ALoaded sent!" << endl;
    }
}

void handleAPutOnTruck(FileOutputStream * out) {
    UATruckArrived truck_arrived = readyToLoad.front(); // get the first item in queue
    readyToLoad.pop();
    int ship_id = truck_arrived.shipid(); // get ship id
    if (orderPackedStatus[ship_id] == 1) { // if already packed
        ACommands acommand;
        // set APack
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

// acks
void handleAcks(::google::protobuf::int64 ack, FileOutputStream * out) {
    map<int, int>::iterator it;
    if ((it = ackRecv.find(ack)) != ackRecv.end()) { // if found seqnum
        it->second = 1; // set flag to 1
    }
    else {
        cerr << "seqnum does not exist!" << endl;
    }
}

// ACommands
// void handleAResponses(AResponses aresponse, FileOutputStream * out) {
    // handle acks
    // for (int i = 0; i < aresponse.acks_size(); i++) {
    //     ::google::protobuf::int64 ack = aresponse.acks(i);

    //     std::thread (handleAcks, ack, out).detach();
    //     // handleAcks(ack, out);
    // }
    // // handle APurchaseMore
    // for (int i = 0; i < aresponse.arrived_size(); i++) {
    //     APurchaseMore arrived = aresponse.arrived(i);
    //     std::thread (handleAPurchaseMore, arrived, out).detach();
    //     // handleAPurchaseMore(arrived, out);
    // }
    // // handle APacked
    // for (int i = 0; i < aresponse.ready_size(); i++) {
    //     APacked ready = aresponse.ready(i);
    //     handleAPacked(ready, out);
    // }
    // // handle ALoaded
    // for (int i = 0; i < aresponse.loaded_size(); i++) {
    //     ALoaded loaded = aresponse.loaded(i);
    //     handleALoaded(loaded, out);
    // }
    // // handle APackage
    // for (int i = 0; i < aresponse.packagestatus_size(); i++) {
    //     APackage packagestatus = aresponse.packagestatus(i);
    //     handleAPackage(packagestatus, out);
    // }
    // // handle AErr
    // for (int i = 0; i < aresponse.error_size(); i++) {
    //     AErr error = aresponse.error(i);
    //     handleAErr(error, out);
    // }
// }

/* Amazon - UPS */

// AUOrder, AUReqTruck
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

// UATruckArrived
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

// AUTruckLoaded
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

// UAPackageArrived
void handleUAPackageArrived(UAPackageArrived package_arrived, FileOutputStream * out) {
    int ship_id = package_arrived.shipid();
    update_status(C, ship_id, "delivered");
    // TODO: set truckArrived to 1 (can't remember what it was..)
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

// Errors
// UAErr setError(string error, int org_seqnum, int seqnum);
// void handleError(UAErr uaerror, FileOutputStream * out) {

// }

// UACommands
// void handleUACommands(UACommands uacommand) {
    // handle acks
    // for (int i = 0; i < uacommand.acks_size(); i++) {
    //     ::google::protobuf::int64 ack = uacommand.acks(i);
    //     handleAcks(ack, ups.in);
    // }
    // // handle UATruckArrived
    // for (int i = 0; i < uacommand.truckarrived_size(); i++) {
    //     UATruckArrived truckarrived = uacommand.truckarrived(i);
    //     handleUATruckArrived(truckarrived, ups.in);
    // }
    // // handle UAPackageArrived
    // for (int i = 0; i < uacommand.packagearrived_size(); i++) {
    //     UAPackageArrived packagearrived = uacommand.packagearrived(i);
    //     handleUAPackageArrived(packagearrived, ups.in);
    // }
    // // handle AErr
    // for (int i = 0; i < uacommand.uaerror_size(); i++) {
    //     UAErr error = uacommand.uaerror(i);
    //     handleError(error, ups.in);
    // }
// }