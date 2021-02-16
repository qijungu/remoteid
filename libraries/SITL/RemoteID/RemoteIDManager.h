#pragma once

#include <AP_GPS/AP_GPS.h>
#include <AP_SerialManager/AP_SerialManager.h>

#include "json.h"
using json = nlohmann::json;

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <chrono>
#include <AP_HAL/utility/Socket.h>

#include "../SIM_Multicopter.h"
#include "Messages/Messages.h"
#include "generic/types.h"

namespace SITL { class MultiCopter; }

#define BRBUFSIZE  (2048)
#define BRPLSIZE   (BRBUFSIZE - sizeof(RSimMessage_Header))
#define BRCHANNEL  "127.0.0.1"

// simulation message type
enum Sim_Msg_Type {
    RSim_Update = 1,       // add or update the vehicle's real location, no payload
    RSim_Broadcast = 2,    // remote id broadcast, the payload is remote id broadcast
};

// Remote ID message type
enum RID_Msg_Type {
    RID_Basic = 0,
    RID_Location = 1,
    RID_Auth = 2,
    RID_AuthPages = 7,   // check again?? TODO
    RID_SelfID = 3,
    RID_System = 4, 
    RID_OperatorID = 5,
};

typedef struct _message_header {
    uint8_t type;   // message type, MSG_TYPE
    uint8_t id[RIDLEN]; // vehicle id
    uint64_t time;  // message time (assume sending and receiving at the same time, because the transmission time is way smaller than the simulation clock tick, 1/1200=833us.
    int32_t lat;    // real lat
    int32_t lng;    // real lng
    int32_t alt;    // real alt
    uint64_t seq;   // sequence, only increase for broadcast
} __attribute__((packed)) RSimMessage_Header;

typedef struct _message {
    RSimMessage_Header header;  // header is only for simulation to collect information
    uint8_t payload[BRPLSIZE]; // payload carries remote id broadcast according to FAA specs
} __attribute__((packed)) RSimMessage;

class RemoteIDManager {
public:

    SITL::MultiCopter* multicopter;
    AP_GPS *gps;
    AP_SerialManager serial_manager;
    json log;
    std::ofstream logFile;

    RemoteIDManager(SITL::MultiCopter* _multicopter) :
        multicopter(_multicopter), sock(true), broadcasting(false), seq(0)
    {
        setup();
    }

    ~RemoteIDManager() {
        logFile.close();
    }

    void setup();
    void update();

protected:

    SocketAPM sock;
    bool broadcasting;  // status, if is broadcasting remoteid

    // const uint64_t LegacyBTStatic = 3000000;
    const uint64_t updateRate = 1000000;
    // const uint64_t LegacyBTStatic = 1000000 * 10;
    // const uint64_t LegacyBTDynamic = 1000000 * 10;
    // const uint64_t BTExtended = 333333;
    // const uint64_t WiFiStatic = 3000000;
    // const uint64_t WiFiDynamic = 333333;

    uint64_t nextUpdate = 0;
    // uint64_t nextDynamicUpdate = 0;

    bool initBroadcast();
    void broadcast(RID_Msg_Type rmt);
    // set remote id simulation message sim_header
    void setRSimMsgHeader(Sim_Msg_Type type);
    // set remote id simmulation message, including sim_header and sim_payload
    // sim_playload is the remote id packet, includeing header and body
    void setRSimMsg(const MessageHeader& header, const MessageBody& body);
    // process received broadcast message
    void recvBroadcast(uint8_t* rm, size_t n);

private:

    RSimMessage msg_out;  // remote message out to the broadcast channel
    size_t msg_out_len;
    uint64_t seq;
    
    // private signer's key
    ecpoint_fp Us;

    uint8_t msg_in[BRBUFSIZE];    // receive message in from broadcast

    // private status variables, to update each time within update() to make simultaion a bit faster
    uint64_t current_time; // current time
    Location current_real_location; // current location

    //void genSignature(uint8_t msg, int len);
    //bool verifySignature();

    void LOG(char* m) {
        printf("%s\n", m);
    }

    void LOG(const std::string& m) {
        std::cout << m << std::endl;
    }

};
