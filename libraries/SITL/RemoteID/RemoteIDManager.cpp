#include <stdio.h>
#include <iostream>
#include <math.h>
#include <chrono>
#include <string>

#include "RemoteIDManager.h"
#include "Messages/Messages.h"
#include "Messages/Enums.h"
#include "auth/keysys.h"
#include "auth/signer.h"
#include "auth/verifier.h"

void RemoteIDManager::setup() {
    logFile.open ("/var/log/remoteid/-" + std::to_string(std::time(nullptr) - 1546300800) + ".txt");

    gps = AP_GPS::get_singleton();
    
    serial_manager.init();
    gps->init(serial_manager);

    sock.reuseaddress();

    authparams_setup();
    // extract private key
    // method1
    extract_key1(&Us, multicopter->get_remoteid(), RIDLEN);
    // method2
    extract_key2(&Us, multicopter->get_remoteid(), RIDLEN);
}

void RemoteIDManager::setRSimMsgHeader(Sim_Msg_Type type) {
    msg_out.header.type = type;
    memcpy(msg_out.header.id, multicopter->get_remoteid(), 20);
    msg_out.header.time = current_time;
    msg_out.header.lat = current_real_location.lat;
    msg_out.header.lng = current_real_location.lng;
    msg_out.header.alt = current_real_location.alt;
    msg_out_len = sizeof(RSimMessage_Header);
}

void RemoteIDManager::setRSimMsg(const MessageHeader& header, const MessageBody& body) {
    setRSimMsgHeader(RSim_Broadcast);
    msg_out.header.seq = ++seq;  // only increase seq for broadcast
    memcpy(msg_out.payload, header.data, header.data_len);
    memcpy(msg_out.payload+header.data_len, body.data, body.data_len);
    msg_out_len = sizeof(RSimMessage_Header) + header.data_len + body.data_len;
}

// should only called from update()
bool RemoteIDManager::initBroadcast() {
    // already broading, no need to re-init
    if (broadcasting) return true;

    setRSimMsgHeader(RSim_Update);
    sock.sendto(&msg_out, msg_out_len, BRCHANNEL, multicopter->get_broadcast_port());
    printf("init %lu\n", current_time);

    broadcasting = true;
    nextUpdate = current_time + 1000000;

    return true;
}

void RemoteIDManager::update() {
    // update private status variables
    current_time = multicopter->get_time_now_us();
    current_real_location = multicopter->get_location();

    if (!broadcasting) initBroadcast();

    gps->update();

    if (broadcasting) {
        // update the channel of the position
        setRSimMsgHeader(RSim_Update);
        sock.sendto(&msg_out, msg_out_len, BRCHANNEL, multicopter->get_broadcast_port());
        //printf("updt %lu\n", current_time);

        // first process all received messages
        ssize_t n;
        while ( (n = sock.recv(msg_in, BRBUFSIZE-1, 1)) >= 0) {
            msg_in[n] = '\0';
            recvBroadcast(msg_in, n);
            printf("recv %lu : %s\n", current_time, msg_in);
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
        if (current_time >= nextUpdate ) {

            broadcast(RID_Basic);
            printf("brct basic %lu\n", current_time);
            /*const Location& l = multicopter->get_location();
            printf("loc lat %d, lng %d, alt %d\n", l.lat, l.lng, l.alt);
            const Location& gl = gps->location();
            printf("gps lat %d, lng %d, alt %d\n", gl.lat, gl.lng, gl.alt);
            const Vector3f& p = multicopter->get_position();
            printf("pos x %f, y %f, z %f\n", p.x, p.y, p.z);
            const Vector3f& v = multicopter->get_velocity_ef();
            printf("vel x %f, y %f, z %f\n", v.x, v.y, v.z);
            const Vector3f& gv = gps->velocity();
            printf("gve x %f, y %f, z %f\n", gv.x, gv.y, gv.z);*/

            nextUpdate += updateRate;
        }
    }

    return;

}

void RemoteIDManager::recvBroadcast(uint8_t* m, size_t n) {
    MessageHeader h(m);
    //const uint8_t* b = m + h.data_len;
    // verify
    int v;
    fp_t sig;
    // method 1
    v = verify1(sig, multicopter->get_remoteid(), RIDLEN, (byte*)m, n);
    // method 2
    v = verify1(sig, multicopter->get_remoteid(), RIDLEN, m, n);
    if (v) return;
    switch (h.header.MessageType) {
        case RID_Basic :
        LOG("recv,basic," + std::to_string(current_time));
        break;

        case RID_Location :
        break;

        case RID_Auth :
        break;

        case RID_SelfID :
        break;

        case RID_System :
        break;

        case RID_OperatorID :
        break;

        default:
        ;
    }
}

void RemoteIDManager::broadcast(RID_Msg_Type rmt) {

    bool ready = true;

    switch (rmt) {
        case RID_Basic : {
            uint8_t idType = 3;
            uint8_t uaType = 2;
            MessageHeader basicMessageHeader(RID_Basic);
            BasicIDMessage basicIdMessage(idType, uaType, multicopter->get_remoteid());
            setRSimMsg(basicMessageHeader, basicIdMessage);
            break;
        }

        case RID_Location : {
            uint8_t status = 0;
            uint8_t flag_height_type = RID_Height_Type_AGL;
            uint8_t flag_ew_direction = RID_EW_Direction_East;
            uint8_t flag_speed_multiplier = RID_Speed_Multiplier_1Q;
            uint8_t trackDirection = (uint8_t)gps->ground_course();
            uint8_t speed = (uint8_t)gps->ground_speed();
            uint8_t verticalSpeed = 0; // calculate from velocity_ef
            int32_t latitude = gps->location().lat;
            int32_t longitude = gps->location().lng;
            uint16_t pressureAltitude = 584;
            uint16_t geodeticAltitude = 584;
            uint16_t height = 0;
            uint8_t verticalAccuracy = 0;
            uint8_t horizontalAccuracy = 0;
            uint8_t baroAltitudeAccuracy = 0;
            uint8_t speedAccuracy = 0;
            uint16_t locationVectorTimestamp = 0;
            uint8_t timestampAccuracy = 0;
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
                locationVectorTimestamp,
                timestampAccuracy
            );
            setRSimMsg(locationVectorMessageHeader, locationVectorMessage);
            break;
        }

        case RID_Auth : {
            uint8_t authType = 0;
            uint8_t pageNumber = 0;
            uint8_t pageCount = 1;
            uint8_t length = 17;
            time_t timestamp = std::time(nullptr) - 1546300800;
            uint32_t authenticationTimestamp = static_cast<uint32_t> (timestamp);
            // sign
            fp_t sig;
            // method 1
            sign1(sig, msg_out.payload, 10);
            // method 2
            sign2(sig, msg_out.payload, 10);
            uint8_t authenticationData[17] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
            memcpy(authenticationData, sig, 17);
            MessageHeader authenticationMessageHeader(RID_Auth);
            AuthenticationMessage authenticationMessage(
                authType, 
                pageNumber,
                pageCount,
                length,
                authenticationTimestamp,
                authenticationData
            );
            setRSimMsg(authenticationMessageHeader, authenticationMessage);
            break;
        }

        /*case RID_AuthPages: {
            uint8_t authPagesType = 0;
            uint8_t pagesNumber = 1;
            uint8_t authenticationPagesData[23] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
            MessageHeader authenticationMessagePagesHeader(RID_AuthPages);
            AuthenticationMessagePages authenticationMessagePages(authPagesType, pagesNumber, authenticationPagesData);
            setRSimMsg(authenticationMessagePagesHeader, authenticationMessagePages);
            break;
        }*/

        case RID_SelfID : {
            uint8_t descriptionType = 0;
            char description[23] = "DronesRus:Survey";
            MessageHeader selfIDMessageHeader(RID_SelfID);
            SelfIDMessage selfIDMessage(descriptionType, description);
            setRSimMsg(selfIDMessageHeader, selfIDMessage);
            break;
        }

        case RID_System : {
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
        }

        case RID_OperatorID : {
            uint8_t operatorIdType = 0;
            char operatorId[20] = "Damen Hannah #123";
            MessageHeader operatorIDMessageHeader(RID_OperatorID);
            OperatorIDMessage operatorIDMessage(operatorIdType, operatorId);
            setRSimMsg(operatorIDMessageHeader, operatorIDMessage);
            break;
        }
        
        default :
            ready = false;
    }

    // no packet to send
    if (!ready) return;

    sock.sendto(&msg_out, msg_out_len, BRCHANNEL, multicopter->get_broadcast_port());
    
    //logFile << messagePack->toJson().dump(4) << std::endl;
    
}
