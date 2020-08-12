#ifndef _PARSEMSG_
#define _PARSEMSG_

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
#include <tuple>
#include <queue>
#include <mutex>
#include <fstream>
#include <time.h>
#include <pqxx/pqxx>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <atomic>

#include "world_amazon.pb.h"
#include "amazon_ups.pb.h"
 #include "frontserver.h"
 #include "UPSServer.h"
 #include "WorldSocket.h"

using namespace std;
using namespace google::protobuf::io;
using namespace pqxx;

// global variables
// sockets
WorldSocket worldclient;
UPSServer upsServer;
FrontServer frontserver;

connection *C; // postgresql database connection
std::atomic <int> seqnum(0); // increment by 1
std::atomic <int> shipid(0); // increment by 1
map<int, int> ackRecv;

queue <int> newOrders;
queue <APurchaseMore> goRequestTruck;
queue <ALoaded> truckLoaded;
queue <UATruckArrived> readyToLoad;
map<int, int> orderPackedStatus;

/**** helper functions ****/
void update_status(connection *C, int ship_id, string status);
void setTruckid(connection *C, int ship_id, int truck_id);
void update_shipid(int order_id, int ship_id);
std::tuple<int, string, int> getOrderInfo(connection *C, int order_id);
int getTruckidByShipid(connection *C, int ship_id);
vector<int> getShipidById(connection *C, int id);
string getUPSusername(connection *C, int ship_id);


// send and recv
template<typename T>
bool sendMesgTo(const T & message, google::protobuf::io::FileOutputStream * out) 
{
    { 
        //extra scope: make output go away before out->Flush()
        // We create a new coded stream for each message.
        // Don’t worry, this is fast.
        google::protobuf::io::CodedOutputStream output(out);
        // Write the size.
        const int size = message.ByteSize();
        output.WriteVarint32(size);
        uint8_t* buffer = output.GetDirectBufferForNBytesAndAdvance(size);
        if (buffer != NULL) 
        {
            // Optimization: The message fits in one buffer, so use the faster direct-to-array serialization path.
            message.SerializeWithCachedSizesToArray(buffer);
        } 
        else 
        {
            // Slightly-slower path when the message is multiple buffers.
            message.SerializeWithCachedSizes(&output);
            if (output.HadError()) 
            {
                return false;
            }
        }
    }
    out->Flush();
    return true;
}

//this is adpated from code that a Google engineer posted online template<typename T>
template<typename T>
bool recvMesgFrom(T & message, google::protobuf::io::FileInputStream * in)
{
    google::protobuf::io::CodedInputStream input(in);
    uint32_t size;
    if (!input.ReadVarint32(&size)) 
    {
        cout << "failed to read varint32!" << endl;
        return false;
    }
    // Tell the stream not to read beyond that size.
    google::protobuf::io::CodedInputStream::Limit limit = input.PushLimit(size);
    // Parse the message.
    if (!message.MergeFromCodedStream(&input)) 
    {
        cout << "failed to merge!" << endl;
        return false;
    }
    if (!input.ConsumedEntireMessage()) 
    {
        cout << "failed to consume entire merge!" << endl;
        return false;
    }
    // Release the limit.
    input.PopLimit(limit);
    return true;
}


/* Amazon - World */

// connect
void connectToWorld(int world_id, int initwh_id, FileOutputStream * out);
bool connectedToWorld(FileInputStream * in);

// product
AProduct* setAProduct(int id, string description, int count);
void handleNewOrders(int order_id, FileOutputStream * out);

// purchaseMore
void handleAPurchaseMore(APurchaseMore arrived, FileOutputStream * worldout, FileOutputStream * upsout);

// pack
void handleAPacked(APacked ready, FileOutputStream * out);

// put on truck(load)
void handleALoaded(ALoaded loaded, FileOutputStream * worldout, FileOutputStream * upsout);
void handleAPutOnTruck(FileOutputStream * out);

// query
AQuery setAQuery(int packageid, int seqnum);
void handleAPackage(APackage packagestatus, FileOutputStream * out);

// AErr
ACommands handleAErr(AErr aerror, FileOutputStream * out);

void handleAcks(::google::protobuf::int64 ack, FileOutputStream * out);
// void handleAResponses(AResponses aresponse);

/* Amazon - UPS */

// AUOrder, AUReqTruck
 void handleAUReqTruck(FileOutputStream * out); // mark1

// UATruckArrived
void handleUATruckArrived(UATruckArrived truck_arrived, FileOutputStream * out);

// AUTruckLoaded
 void handleAUTruckLoaded(FileOutputStream * out); //mark2

// UAPackageArrived
void handleUAPackageArrived(UAPackageArrived package_arrived, FileOutputStream * out);

// Errors
UAErr setError(string error, int org_seqnum, int seqnum);
void handleError(UAErr uaerror, FileOutputStream * out);

// UACommand
// void handleUACommands(UACommands uacommand);


#endif