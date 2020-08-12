/************ functionality test codes *************/
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
#include "parseMsg.h"

using namespace google::protobuf::io;
using namespace std;
using namespace pqxx;

/*
**** AConnect test codes: send AConnect and receive AConnected ****

    connectToWorld(3, 1, worldclient.out); 
    if (connectedToWorld(worldclient.in) == true) {
        cout << "Connected to world!" << endl;
    }

*/

/*
****  APurchaseMore test codes: send APurchaseMore, receive AResponse ****
****  containing APurchaseMore, and send APack ****

    ACommands acommand;   
    APurchaseMore * ap = acommand.add_buy();
    ap->set_whnum(1); // set whnum
    ap->set_seqnum(2); // set seqnum
    // set products
    AProduct * product = ap->add_things();
    product->set_id(15);
    product->set_description("juice");
    product->set_count(3);

    if (sendMesgTo(acommand, worldclient.out) == false) { // send ack
        cout << "cannot send APurchaseMore!" << endl;
    }
    else {
        cout << "APurchaseMore sent!" << endl;
    }

    AResponses aresponse;
    if (recvMesgFrom(aresponse, worldclient.in) == false) {
        cout << "Failed to recv aresponse!" << endl;
    }
    else {
        for (int i = 0; i < aresponse.arrived_size(); i++) {
            APurchaseMore arrived = aresponse.arrived(i);
            // thread (handleAPurchaseMore, arrived, worldclient.out, upsServer.out).detach();

            ::google::protobuf::int32 whnum = arrived.whnum();
            cout << "whnum = " << whnum << endl;
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
            cout << "things id set: " << things[0].id() << endl;
            thing->set_description(things[0].description());
            cout << "things description set: " << things[0].description() << endl;
            thing->set_count(things[0].count());
            cout << "things count set: " << things[0].count() << endl;            

            apack->set_shipid(shipid);
            cout << "things shipid set: " << shipid << endl; 
            // update_shipid(C, thing->id(), shipid); // in database
            shipid++;
            cout << "things shipid updated" << endl;
            apack->set_seqnum(seqnum); // set seqnum
            cout << "things seqnum set: " << seqnum << endl;
            seqnum++;
            // set the seq of arrived as ack
            acommand.add_acks(arrived.seqnum());
            cout << "ack set: " << seqnum << endl;

            if (sendMesgTo(acommand, worldclient.out) == false) { // send ack
                cout << "cannot send APack!" << endl;
            }
            else {
                cout << "APack sent!" << endl;
            }


            // doesn't work
            // recv APacked
            AResponses aresponse;
            if (recvMesgFrom(aresponse, worldclient.in) == false) {
                cout << "Failed to recv aresponse!" << endl;
            }
            for (int i = 0; i < aresponse.ready_size(); i++) {
                APacked ready = aresponse.ready(i);
                // thread (handleAPacked, ready, worldclient.out).detach();

                ACommands acommand;
                int ship_id = ready.shipid();

                // set the seq of ready as ack
                acommand.add_acks(ready.seqnum());

                if (sendMesgTo(acommand, worldclient.out) == false) { // send ack
                    cout << "cannot send ack for APacked!" << endl;
                }
                else {
                    cout << "Ack for APacked sent!" << endl;
                }
            }
        }
    }

*/

/*
****  APutOnTruck test codes: send APutOnTruck, receive ALoaded ****

    ACommands acommand;
    // set APutOnTruck
    APutOnTruck* load = acommand.add_load();
    load->set_whnum(1); 
    load->set_truckid(2); 
    load->set_shipid(3); // set shipid
    load->set_seqnum(5); // set seqnum
    seqnum++;

    if (sendMesgTo(acommand, out) == false) { // send ack
        cout << "cannot send APutOnTruck!" << endl;
    }
    else {
        cout << "APutOnTruck sent!" << endl;
    }

    AResponses aresponse;
    if (recvMesgFrom(aresponse, worldclient.in) == false) {
        cout << "Failed to recv aresponse!" << endl;
    }
    else {
        for (int i = 0; i < aresponse.loaded_size(); i++) {
            ALoaded loaded = aresponse.loaded(i);

            int ship_id = loaded.shipid();
            cout << "ship_id: " << ship_id << endl;
            int seqnum = loaded.seqnum();
            cout << "seq_num: " << seqnum << endl;

        }
    }

*/

