#include <stdio.h>
#include <iostream>
#include <math.h>
#include <string>
#include <algorithm>

#include <AP_Arming/AP_Arming.h>
#include <AP_Vehicle/AP_Vehicle.h>

#include "RemoteIDManager.h"
#include "Messages/Messages.h"
#include "Messages/Enums.h"
#include "Messages/AuthIDMessage.h"
#include "Messages/AuthLocMessage.h"
#include "auth/keysys.h"
#include "auth/signer.h"
#include "auth/verifier.h"
#include "matrix.h"

void RemoteIDManager::setup() {
    logFile.open ("/var/log/remoteid/-" + std::to_string(std::time(nullptr) - 1546300800) + ".txt");

    gps = AP_GPS::get_singleton();
    baro = AP_Baro::get_singleton();
    
    serial_manager.init();
    gps->init(serial_manager);

    sock.reuseaddress();

    auth = true;
    authparams_setup();
}

/**
 * This is for simulation only. It passes the true vehicle location and time to the broadcast module
 * return the length of msg
 */
int RemoteIDManager::setRSimMsgHeader(RSimMessage& msg, Sim_Msg_Type type) {
    msg.header.type = type;
    memcpy(msg.header.id, multicopter->get_remoteid(), 20);
    msg.header.time = current_time;
    msg.header.lat = current_real_location.lat;
    msg.header.lng = current_real_location.lng;
    msg.header.alt = current_real_location.alt;
    return sizeof(RSimMessage_Header);
}

/**
 * msg header contains info for broadcast simulation only.
 * msg paylaod contains header and body of remote id packet.
 * return the length of msg
 */
int RemoteIDManager::setRSimMsg(RSimMessage& msg, const MessageHeader& header, const MessageBody& body) {
    // set header of msg_out
    int hl = setRSimMsgHeader(msg, RSim_Broadcast);
    msg.header.seq = ++seq;  // only increase seq for broadcast
    // set payload of msg_out
    memcpy(msg.payload, header.data, header.data_len);
    memcpy(msg.payload+header.data_len, body.data, body.data_len);
    return hl + header.data_len + body.data_len;
}

// should only called from update()
bool RemoteIDManager::initBroadcast() {
    // already broadcasting, no need to re-init
    if (broadcasting) return true;

    /*AP_GPS::GPS_Status gs = gps->status();
    if ( !(gs==AP_GPS::GPS_OK_FIX_3D_RTK_FIXED || gs==AP_GPS::GPS_OK_FIX_3D_RTK_FLOAT || gs==AP_GPS::GPS_OK_FIX_3D_DGPS || gs==AP_GPS::GPS_OK_FIX_3D) )
        return false;
    LOG("gps ready, start broadcasting");*/

    msg_out_len = setRSimMsgHeader(msg_out, RSim_Update);
    sock.sendto(&msg_out, msg_out_len, BRCHANNEL, multicopter->get_broadcast_port());
    LOG("init");

    broadcasting = true;
    nextStaticUpdate = current_time;
    nextDynamicUpdate = current_time;

    // extract private key
    // method1
    extract_key1(&Us, multicopter->get_remoteid(), RIDLEN);
    // method2
    //extract_key2(&Us, multicopter->get_remoteid(), RIDLEN);

    return true;
}

void RemoteIDManager::update() {
    // update private status variables
    current_time = multicopter->get_time_now_us();
    current_real_location = multicopter->get_location();

    if (AP::arming().is_armed()) {
        if (!armed) {
            armed = true;
            LOG("armed");
        }
        if (!broadcasting) initBroadcast();
    } else {
        if (armed) {
            armed = false;
            LOG("disarmed");
            end();
        }
        broadcasting = false;
    }

    uint8_t newmode = AP::vehicle()->get_mode();
    if (mode != newmode) {
        mode = newmode;
        LOG("mode " + std::to_string(mode));
    }

    // quit on RTL, at the end of mission
    if (mode == 6) end(); // refer to ArduCopter/mode.h

    gps->update();

    if (broadcasting) {
        // update the channel of the position
        msg_out_len = setRSimMsgHeader(msg_out, RSim_Update);
        sock.sendto(&msg_out, msg_out_len, BRCHANNEL, multicopter->get_broadcast_port());

        // first process all received messages
        ssize_t n;
        while ( (n = sock.recv(msg_in, BRBUFSIZE-1, 1)) >= 0) {
            msg_in[n] = '\0';
            recvBroadcast(msg_in, n);
        }
        
        /**
         * A Standard Remote ID Drone, remote ID capability is built into the drone, must broadcas:
         * A unique identifier for the drone: drone's serial number or a session ID
         * An indication of the drone's latitude, longitude, geometric altitude, and velocity;
         * An indication of the control station's latitude, longitude, and geometric altitude;
         * A time mark
         * An emergency status indication.
         * 
         * A drone with a remote ID broadcast module, remote ID capability through module attached to drone, must broadcast
         * The serial number of the broadcast module;
         * An indication of the drone's latitude, longitude, geometric altitude, and velocity;
         * An indication of the latitude, longitude, and geometric altitude of the drone's take-off location;
         * A time mark.
         * 
         * BCMinUasLocRefreshRate 1 Seconds BUR0010 5.4.4
         * BCMinStaticRefreshRate 3 Seconds BUR0020 5.4.4
         */
        // then, broadcast remote id on a schedule
        if (current_time >= nextStaticUpdate) {
            broadcast(RID_BasicID);

            nextStaticUpdate += updateStaticInterval;
        }

        if (current_time >= nextDynamicUpdate) {
            broadcast(RID_Location);

            nextDynamicUpdate += updateDynamicInterval;
        }

    }

    return;

}

void RemoteIDManager::recvBroadcast(uint8_t* m, size_t n) {
    MessageHeader h(m);
    const uint8_t* d = m + h.data_len;
    switch (h.header.MessageType) {
        case RID_BasicID :
        //LOG("recv basicid");
        //processBasicID(d);  // TODO
        break;

        case RID_Location :
        //LOG("recv location");
        //processLocationVector(d); // TODO
        break;

        case RID_Auth :
        break;

        case RID_SelfID :
        break;

        case RID_System :
        break;

        case RID_OperatorID :
        break;

        case RID_MsgPack :
        LOG("recv msgpack " + std::to_string(n));
        processMessagePack(d);
        break;

        default:
        ;
    }
}

void RemoteIDManager::broadcast(RID_Msg_Type rmt) {

    bool ready = true;

    switch (rmt) {
        case RID_BasicID : {
            if (auth) msg_out_len = makeAuthBasicID(msg_out);
            else msg_out_len = makeBasicID(msg_out);
            break;
        }

        case RID_Location : {
            if (auth) msg_out_len = makeAuthLocationVector(msg_out);
            else msg_out_len = makeLocationVector(msg_out);
            break;
        }

        /*case RID_SelfID : {
            uint8_t descriptionType = 0;
            char description[23] = "DronesRus:Survey";
            MessageHeader selfIDMessageHeader(RID_SelfID);
            SelfIDMessage selfIDMessage(descriptionType, description);
            setRSimMsg(selfIDMessageHeader, selfIDMessage);
            break;
        }*/

        /*case RID_System : {
            uint8_t systemFlags = 0;
            int32_t operatorLatitude = multicopter->get_home().lat;
            int32_t operatorLongitude = multicopter->get_home().lng;
            uint16_t areaCount = 1;
            uint8_t areaRadius = 0;
            uint16_t AreaCeiling = 0;
            uint16_t AreaFloor = 0;
            MessageHeader systemMessageHeader(RID_System);
            SystemMessage systemMessage(            
                systemFlags,
                operatorLatitude,
                operatorLongitude,
                areaCount,
                areaRadius,
                AreaCeiling,
                AreaFloor
            );
            setRSimMsg(systemMessageHeader, systemMessage);
            break;
        }*/

        /*case RID_OperatorID : {
            uint8_t operatorIdType = 0;
            char operatorId[20] = "Damen Hannah #123";
            MessageHeader operatorIDMessageHeader(RID_OperatorID);
            OperatorIDMessage operatorIDMessage(operatorIdType, operatorId);
            setRSimMsg(operatorIDMessageHeader, operatorIDMessage);
            break;
        }*/
        
        default :
            ready = false;
    }

    // no packet to send
    if (!ready) return;

    sock.sendto(&msg_out, msg_out_len, BRCHANNEL, multicopter->get_broadcast_port());
    LOG("bcst " + std::to_string(msg_out_len - sizeof(RSimMessage_Header)) \
    + " lat " + std::to_string(current_real_location.lat/1.0e7) \
    + " lon " + std::to_string(current_real_location.lng/1.0e7) \
    + " alt " + std::to_string(current_real_location.alt/100.0) );

    // for self testing only. comment out if not in self testing
    // recvBroadcast(msg_out.payload, msg_out_len - sizeof(RSimMessage_Header));

    //logFile << messagePack->toJson().dump(4) << std::endl;
    
}

void RemoteIDManager::processBasicID(const uint8_t* data, std::array<uint8_t, RIDLEN> id, bool verified) {
    BasicIDMessage p(data);

    ReceivedIDMsg m;
    m.id = p.basicIDMessage;
    m.verified = verified;
    m.receivedTime = current_time;

    if (uavs.count(id) == 0) {
        UAVStatus s;
        s.ids.push_front(m);
        uavs[id] = s;
    } else {
        UAVStatus& s = uavs[id];
        if ( (m.receivedTime - s.ids.front().receivedTime) > MinMsgInterval ) {
            s.ids.push_front(m);
            purgeStatus(id);
        }
    }

}

void RemoteIDManager::processLocationVector(const uint8_t* data, std::array<uint8_t, RIDLEN> id, bool verified) {
    LocationVectorMessage p(data);
 
    ReceivedLocMsg m;
    m.location = p.locationVector;
    m.lat = p.locationVector.latitude/1.0e7;
    m.lon = p.locationVector.longitude/1.0e7;
    m.alt = p.locationVector.geodeticAltitude/2.0-1000.0;
    m.verified = verified;
    m.receivedTime = current_time;

    if (uavs.count(id) == 0) {
        UAVStatus s;
        s.locs.push_front(m);
        uavs[id] = s;
    } else {
        UAVStatus& s = uavs[id];
        if ( (m.receivedTime - s.locs.front().receivedTime) > MinMsgInterval ) {
            s.locs.push_front(m);
            purgeStatus(id);
        }
    }

}

void RemoteIDManager::purgeStatus(std::array<uint8_t, RIDLEN> id) {
    // nothing to purge
    if (uavs.count(id) == 0) return;

    UAVStatus& s = uavs[id];

    // remove over-que messages
    while( s.ids.size()>QUESIZE ) s.ids.pop_back();
    // remove expire messeages
    while( s.ids.size()>0 && (current_time - s.ids.back().receivedTime) > MsgExpire ) s.ids.pop_back(); 
    // remove over-que messages
    while( s.locs.size()>QUESIZE ) s.locs.pop_back(); 
    // remove expire messeages
    while( s.ids.size()>0 && (current_time - s.locs.back().receivedTime) > MsgExpire ) s.locs.pop_back();

    // no message left for uav id, so remove id from uavs
    if (s.ids.size() == 0 && s.locs.size() == 0) uavs.erase(id);
}

bool RemoteIDManager::verifySignature(const MessagePack& p) {

        // must have two auth msgs and at leat one id msg
        assert(p.pack.numberMessages >= 3);
        // process authentication
        // get the signature
        fp_t sig;
        uint8_t* sigptr = (uint8_t*)&sig;
        // page 0
        MessageHeader h_a0(p.pack.messages[0]);
        AuthenticationMessage a0(&(p.pack.messages[0][1]));
        // validate page 0
        assert(a0.authentication.authType == RID_Auth_Type_UasID || a0.authentication.authType == RID_Auth_Type_Location);
        assert(a0.authentication.pageCount == 2);
        assert(a0.authentication.pageNumber == 0);
        assert(a0.authentication.length == 32);
        // page 1
        MessageHeader h_a1(p.pack.messages[1]);
        AuthenticationMessagePage a1(&(p.pack.messages[1][1]));
        // validate page 1
        assert(a1.authenticationPage.authType == a0.authentication.authType);
        assert(a1.authenticationPage.pageNumber == 1);

        // TODO: a0.authentication.timestamp

        // copy the signature from page 0 and page 1
        memcpy(sigptr, a0.authentication.authenticationData, 17);
        memcpy(sigptr+17, a1.authenticationPage.authenticationData, 15);

        // get id from message[2]
        // message[2] must be basic id
        assert(p.pack.messages[2][0] == RID_BasicID);
        BasicIDMessage id(&(p.pack.messages[2][1]));
        uint8_t rid[RIDLEN];
        memcpy(rid, id.basicIDMessage.uasId, RIDLEN);

        // from message[2], all data, but need to be concatenated
        const uint8_t* d = &(p.pack.messages[2][0]);
        uint8_t l = (p.pack.numberMessages - 2) * 25;
        
        // verify
        int v;
        // method 1
        v = verify1(sig, rid, RIDLEN, d, l);
        // method 2
        //v = verify2(sig, rid, RIDLEN, d, l);
        LOG("verify " + std::to_string(v));

        return v==0 ? true : false;
}

void RemoteIDManager::processMessagePack(const uint8_t* data) {
    MessagePack p(data);
    int i;

    // auth must be the first packet in message pack.
    MessageHeader f(p.pack.messages[0]);

    if (f.header.MessageType == RID_Auth) {

        AuthenticationMessage a0(&(p.pack.messages[0][1]));

        // get id from message[2]
        // message[2] must be basic id
        assert(p.pack.messages[2][0] == RID_BasicID);
        BasicIDMessage idmsg(&(p.pack.messages[2][1]));
        std::array<uint8_t, RIDLEN> id;
        for (i=0; i<RIDLEN; i++) id[i] = idmsg.basicIDMessage.uasId[i];

        bool v = false;
        if (uavs.count(id) == 0) {
            // a new nearby uav
            LOG("reason new_nearby_uav");
            v = verifySignature(p);
            if ( !v ) return; // ToDo

        } else {
            // an exisiting nearby uav
            const UAVStatus& s = uavs[id];
            std::size_t loccnt = s.locs.size();
            if (loccnt < MinLocRegression) {
                // not enough past locations for regression
                LOG("reason not_enough_past_locations_for_regression " + std::to_string(loccnt));
                v = verifySignature(p);
                if ( !v ) return;

            } else {
                // enough past locations
                
                if ( (current_time - s.lastLocVerifyTime) > MaxLocVerifyInterval ) {
                    // too long not verify a location
                    LOG("reason too_long_not_verify_a_location " + std::to_string(current_time - s.lastLocVerifyTime));
                    v = verifySignature(p);
                    if ( !v ) return;

                } else {
                    // check if the last verified location is in the past MinLocVerify.

                    bool pv = false;
                    for (i=0; i<=MinLocVerify; i++) {
                        if (s.locs[i].verified) {
                            pv = true;
                            break;
                        }
                    }
                    if (!pv) {
                        // no early verification in the past MinLocVerify
                        LOG("reason no_early_verification_in_the_past_MinLocVerify " + std::to_string(s.locs.size()));
                        v = verifySignature(p);
                        if ( !v ) return;

                    } else {
                        // one location was verified recently within a time limit and a count limit
                        if (a0.authentication.authType == RID_Auth_Type_Location) {
                            // so, we can regress on the past locations
                            std::deque<double> lats;
                            std::deque<double> lons;
                            std::deque<double> alts;
                            std::deque<double> dts;
                            double dt;
                            uint64_t starttime = s.locs[0].receivedTime;
                            for (i=0; ((std::size_t)i)<loccnt; i++) {
                                const ReceivedLocMsg& l = s.locs[i];
                                dt = ((double)l.receivedTime) - ((double)starttime);
                                if ( -dt  > RegWindow ) break;
                                lats.push_front(l.lat);
                                lons.push_front(l.lon);
                                alts.push_front(l.alt);
                                dts.push_front(dt/1.0e6);
                            }

                            if (lats.size() < MinLocRegression) {
                                LOG("reason not_enough_past_locations_within_regression_window " + std::to_string(lats.size()));
                                v = verifySignature(p);
                                if ( !v ) return;

                            } else {
                                // enough regression locations

                                double rlat[N], rlon[N], ralt[N];
                                if (!qregress(dts, lats, rlat)) LOG("cannot predict lat");
                                if (!qregress(dts, lons, rlon)) LOG("cannot predict lon");
                                if (!qregress(dts, alts, ralt)) LOG("cannotpredict alt");

                                // y = r[0]*x2 + r[1]*x + r[2];
                                double plat, plon, palt, dt2;
                                dt = (current_time - starttime)/1.0e6;
                                dt2 = dt*dt;
                                plat = rlat[0]*dt2 + rlat[1]*dt + rlat[2];
                                plon = rlon[0]*dt2 + rlon[1]*dt + rlon[2];
                                palt = ralt[0]*dt2 + ralt[1]*dt + ralt[2];

                                // broadcast location
                                double blat, blon, balt; 
                                LocationVectorMessage lm(&(p.pack.messages[3][1]));
                                blat = lm.locationVector.latitude/1.0e7;
                                blon = lm.locationVector.longitude/1.0e7;
                                balt = (lm.locationVector.geodeticAltitude/2.0-1000.0);
                                
                                // error
                                double elat, elon, ealt;
                                elat = abs(plat - blat);
                                elon = abs(plon - blon);
                                ealt = abs(palt - balt);

                                LOG("predict");
                                printf("broad : lat %.8f lon %.8f alt %.3f\n", blat, blon, balt);
                                printf("predi : lat %.8f lon %.8f alt %.3f\n", plat, plon, palt);
                                printf("error : lat %.8f lon %.8f alt %.3f\n", elat, elon, ealt);

                                if (elat>2e-5 || elon>2e-5 || ealt>2.0) {
                                    // error is too large
                                    LOG("reason error_too_large");
                                    v = verifySignature(p);
                                    if ( !v ) return;

                                }
                            }

                        }
                    }
                }
            }
        }
        
        if (a0.authentication.authType == RID_Auth_Type_UasID) {
            // message[2] must be id
            assert(p.pack.messages[2][0] == RID_BasicID);
            processBasicID(&(p.pack.messages[2][1]), id, v);
            if (v && uavs.count(id)>0) uavs[id].lastIdVerifyTime = current_time;
        } else if (a0.authentication.authType == RID_Auth_Type_Location) {
            // message[2] must be id
            assert(p.pack.messages[2][0] == RID_BasicID);
            // message[3] must be location
            assert(p.pack.messages[3][0] == RID_Location);
            processLocationVector(&(p.pack.messages[3][1]), id, v);
            if (v && uavs.count(id)>0) uavs[id].lastLocVerifyTime = current_time;
        }
        
    } else {
        // not auth messages
        for (i=0; i<p.pack.numberMessages; i++) {
            MessageHeader h(p.pack.messages[i][0]);
            if (h.header.MessageType == RID_BasicID) {
                BasicIDMessage m(&(p.pack.messages[i][1]));
                // processBasicID(m);  // TODO
            } else if (h.header.MessageType == RID_Location) {
                LocationVectorMessage m(&(p.pack.messages[i][1]));
                // processLocationVector(m); // TODO
            }
        }
    }

    return;
}

int RemoteIDManager::makeBasicID(RSimMessage& msg) {
    uint8_t idType = RID_ID_Type_Serial;
    uint8_t uaType = RID_UA_Type_Helicopter;
    MessageHeader basicMessageHeader(RID_BasicID);
    BasicIDMessage basicIdMessage(idType, uaType, multicopter->get_remoteid());
    return setRSimMsg(msg, basicMessageHeader, basicIdMessage);
}

int RemoteIDManager::makeLocationVector(RSimMessage& msg) {
    uint8_t status = RID_OP_Status_Airborne;
    uint8_t flag_height_type = RID_Height_Type_AGL;
    uint8_t flag_ew_direction;
    uint8_t trackDirection;
    encodeDirection(gps->ground_course(), flag_ew_direction, trackDirection);
    uint8_t flag_speed_multiplier = RID_Speed_Multiplier_1Q;
    uint8_t speed;
    encodeSpeed(gps->ground_speed(), flag_speed_multiplier, speed);
    int8_t verticalSpeed;
    encodeVerticalSpeed(gps->velocity().z, verticalSpeed);
    int32_t latitude = gps->location().lat;
    int32_t longitude = gps->location().lng;
    uint16_t pressureAltitude;
    encodePressureAltitude(baro->get_altitude(), pressureAltitude);
    uint16_t geodeticAltitude;
    encodePressureAltitude(gps->location().alt/100, geodeticAltitude);
    uint16_t height = geodeticAltitude;
    uint8_t verticalAccuracy;
    float accuracy;
    if (gps->vertical_accuracy(accuracy)) encodeVerticalAccuracy(accuracy, verticalAccuracy);
    else verticalAccuracy = RID_Vertical_Accuracy_Unknown;
    uint8_t horizontalAccuracy;
    if (gps->horizontal_accuracy(accuracy)) encodeHorizontalAccuracy(accuracy, horizontalAccuracy);
    else horizontalAccuracy = RID_Horizontal_Accuracy_Unknown;
    uint8_t baroAltitudeAccuracy = RID_Vertical_Accuracy_3m;
    uint8_t speedAccuracy;
    if (gps->speed_accuracy(accuracy)) ;
    else speedAccuracy = RID_Speed_Accuracy_Unknown;
    uint16_t timestamp = (current_time/100000)%36000; // current_time in us, timestamp in 0.1s
    uint8_t timestampAccuracy = 1;
    MessageHeader locationVectorMessageHeader(RID_Location);
    LocationVectorMessage locationVectorMessage(
        status,
        flag_height_type,
        flag_ew_direction,
        flag_speed_multiplier,
        trackDirection,
        speed,
        verticalSpeed,
        latitude,
        longitude,
        pressureAltitude,
        geodeticAltitude,
        height,
        verticalAccuracy,
        horizontalAccuracy,
        baroAltitudeAccuracy,
        speedAccuracy,
        timestamp,
        timestampAccuracy
    );
    return setRSimMsg(msg, locationVectorMessageHeader, locationVectorMessage);
}

int RemoteIDManager::makeAuthBasicID(RSimMessage& msg) {
    uint8_t idMsg[25];
    RSimMessage _m;
    makeBasicID(_m);
    memcpy(idMsg, _m.payload, 25);

    MessageHeader authPackHeader(RID_MsgPack);
    AuthIDMessage authPack;

    addSignature(authPack, RID_Auth_Type_UasID, idMsg, 25);

    // add id
    authPack.addMessage(idMsg);

    return setRSimMsg(msg, authPackHeader, authPack);
}

int RemoteIDManager::makeAuthLocationVector(RSimMessage& msg) {
    uint8_t idMsg[25], locMsg[25], data[50];
    RSimMessage _m;
    makeBasicID(_m);
    memcpy(idMsg, _m.payload, 25);
    makeLocationVector(_m);
    memcpy(locMsg, _m.payload, 25);

    memcpy(data, idMsg, 25);
    memcpy(data+25, locMsg, 25);
 
    MessageHeader authPackHeader(RID_MsgPack);
    AuthLocMessage authPack;

    addSignature(authPack, RID_Auth_Type_Location, data, 50);

    // add data
    authPack.addMessage(idMsg);
    authPack.addMessage(locMsg);

    return setRSimMsg(msg, authPackHeader, authPack);
    
}

void RemoteIDManager::addSignature(MessagePack& authPack, RID_Auth_Type authType, uint8_t* data, int len) {
    // sign
    fp_t sig;
    // method 1
    sign1(sig, data, len);
    // method 2
    //sign2(sig, data, len);

    // auth page 0
    uint8_t pageNumber = 0;
    uint8_t pageCount = 2;
    uint8_t length = 32;
    uint32_t timestamp = current_time;  // TODO, to fix according to spec
    uint8_t* signature = (uint8_t*)sig;
    MessageHeader authenticationMessageHeader(RID_Auth);
    AuthenticationMessage authenticationMessage(
        authType, 
        pageNumber,
        pageCount,
        length,
        timestamp,
        17,
        signature
    );
    RSimMessage _m;
    setRSimMsg(_m, authenticationMessageHeader, authenticationMessage);
    authPack.addMessage(_m.payload);

    // auth page 1
    pageNumber = 1;
    AuthenticationMessagePage authenticationMessagePage(
        authType, 
        pageNumber,
        sizeof(fp_t)-17,
        signature+17
    );
    setRSimMsg(_m, authenticationMessageHeader, authenticationMessagePage);
    authPack.addMessage(_m.payload);
}

// Direction expressed as the route course measured clockwise from true north. Encode as 0–179.
// If E/W Direction bit is set, then 180 should be added to the value.
void RemoteIDManager::encodeDirection(float value, uint8_t& flag_ew_direction, uint8_t& trackDirection) {
    int16_t direction = (int16_t)round(value);
    direction = direction % 360;
    if (0 <= direction && direction < 180) {
        flag_ew_direction = RID_EW_Direction_East;
        trackDirection = direction;
    } else {
        flag_ew_direction = RID_EW_Direction_West;
        trackDirection = direction-180;
    }
}

// If value <= 225*0.25, encodedValue = value/0.25, set multiplier flag to 0
// Else if value > 225*0.25 and value <254.25, encodedValue = (value – (225*0.25))/0.75, set multiplier flag to 1
// Else value >= 254.25 m/s, encodedValue = 254, set multiplier flag to 1
void RemoteIDManager::encodeSpeed(float value, uint8_t& flag_speed_multiplier, uint8_t& speed) {
    if (value < 0) value = 0;
    if (value <= 225*0.25) {
        flag_speed_multiplier = RID_Speed_Multiplier_1Q;
        speed = (uint8_t)value*4;
    } else if (value < 254.25) {
        flag_speed_multiplier = RID_Speed_Multiplier_3Q;
        speed = (uint8_t)((value - 225*0.25) / 0.75);
    } else {
        flag_speed_multiplier = RID_Speed_Multiplier_3Q;
        speed = 254;
    }
}

// EncodedValue = Value(m/s) / 0.5, up to ±63.5 m/s (12.5k ft/min)
void RemoteIDManager::encodeVerticalSpeed(float value, int8_t& verticalSpeed) {
    if (value > 63.5) value = 63.5;
    if (value < -63.5) value = -63.5;
    verticalSpeed = (int8_t)(value * 2);
}             

// EncodedValue = (Value(m) + 1000) / 0.5
void RemoteIDManager::encodePressureAltitude(float value, uint16_t& pressureAltitude) {
    if (value<-1000) value = -1000;
    if (value>31767.5) value = 31767.5;
    pressureAltitude = (uint16_t)((value + 1000) * 2);
}

// enum RID_Vertical_Accuracy
void RemoteIDManager::encodeVerticalAccuracy(float value, uint8_t& verticalAccuracy) {
    if (value < 0) verticalAccuracy = RID_Vertical_Accuracy_Unknown;
    else if (value < 1) verticalAccuracy = RID_Vertical_Accuracy_1m;
    else if (value < 3) verticalAccuracy = RID_Vertical_Accuracy_3m;
    else if (value < 10) verticalAccuracy = RID_Vertical_Accuracy_10m;
    else if (value < 25) verticalAccuracy = RID_Vertical_Accuracy_25m;
    else if (value < 45) verticalAccuracy = RID_Vertical_Accuracy_45m;
    else if (value < 150) verticalAccuracy = RID_Vertical_Accuracy_150m;
    else verticalAccuracy = RID_Vertical_Accuracy_Unknown;
}

// enum RID_Horizontal_Accuracy
void RemoteIDManager::encodeHorizontalAccuracy(float value, uint8_t& horizontalAccuracy) {
    if (value < 0) horizontalAccuracy = RID_Horizontal_Accuracy_Unknown;
    else if (value < 1) horizontalAccuracy = RID_Horizontal_Accuracy_1m;
    else if (value < 3) horizontalAccuracy = RID_Horizontal_Accuracy_3m;
    else if (value < 10) horizontalAccuracy = RID_Horizontal_Accuracy_10m;
    else if (value < 30) horizontalAccuracy = RID_Horizontal_Accuracy_30m;
    else if (value < 92.6) horizontalAccuracy = RID_Horizontal_Accuracy_005NM;
    else if (value < 185.2) horizontalAccuracy = RID_Horizontal_Accuracy_01NM;
    else if (value < 555.6) horizontalAccuracy = RID_Horizontal_Accuracy_03NM;
    else if (value < 926) horizontalAccuracy = RID_Horizontal_Accuracy_05NM;
    else if (value < 1852) horizontalAccuracy = RID_Horizontal_Accuracy_1NM;
    else if (value < 3704) horizontalAccuracy = RID_Horizontal_Accuracy_2NM;
    else if (value < 7408) horizontalAccuracy = RID_Horizontal_Accuracy_4NM;
    else if (value < 18520) horizontalAccuracy = RID_Horizontal_Accuracy_10NM;
    else horizontalAccuracy = RID_Horizontal_Accuracy_Unknown;
}

// enum RID_Speed_Accuracy
void RemoteIDManager::encodeSpeedAccuracy(float value, uint8_t& speedAccuracy) {
    if (value < 0) speedAccuracy = RID_Speed_Accuracy_Unknown;
    else if (value < 0.3) speedAccuracy = RID_Speed_Accuracy_03m;
    else if (value < 1) speedAccuracy = RID_Speed_Accuracy_1m;
    else if (value < 3) speedAccuracy = RID_Speed_Accuracy_3m;
    else if (value < 10) speedAccuracy = RID_Speed_Accuracy_10m;
    else speedAccuracy = RID_Speed_Accuracy_Unknown;
}
