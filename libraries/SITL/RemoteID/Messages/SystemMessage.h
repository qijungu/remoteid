#ifndef SYSTEM_MESSAGE_H
#define SYSTEM_MESSAGE_H

#include <stdint.h>

// 5.4.5.18 System Message Type: 0x4, Static Periodicity, Optional
struct SystemMessageData {
    unsigned operatorLocationType: 2; // enum RID_OP_Location_Type
    unsigned flags : 6;
    int32_t operatorLatitude;   // Int signed deg*10^7 (LE)
    int32_t operatorLongitude;  // Int signed deg*10^7 (11 mm precision) (LE)
    uint16_t areaCount;  // Number of aircraft in Area, group or formation (default 1)
    uint8_t areaRadius;  // Radius of cylindrical area of group or formation * 10 m (default 0) centered on Location/Vector Message position
    uint16_t areaCeiling;  // Group operations ceiling WGS-84 HAE (Altitude + 1000 m)/0.5
    uint16_t areaFloor;    // Group operations floor WGS-84 HAE (Altitude + 1000 m)/0.5
    uint8_t reserved[8];
} __attribute__((packed));

class SystemMessage: public MessageBody {
    public:

        struct SystemMessageData system;

        SystemMessage(
            uint8_t operatorLocationType,
            int32_t operatorLatitude,
            int32_t operatorLongitude,
            uint16_t areaCount,
            uint8_t areaRadius,
            uint16_t areaCeiling,
            uint16_t areaFloor
        ) {
            assert(sizeof(struct SystemMessageData)==24);
            memset(&system, 0, sizeof(struct SystemMessageData));
            system.operatorLocationType = operatorLocationType;
            system.areaRadius = areaRadius;
            system.areaCount = areaCount;
            system.operatorLatitude = operatorLatitude;
            system.operatorLongitude = operatorLongitude;
            system.areaCount = areaCount;
            system.areaCeiling = areaCeiling;
            system.areaFloor = areaFloor;
            data_len = sizeof(struct SystemMessageData);
            data = (uint8_t*)&system;
        };

        SystemMessage(uint8_t* d) {
            assert(sizeof(struct SystemMessageData)==24);
            memset(&system, 0, sizeof(struct SystemMessageData));
            memcpy(&system, d, sizeof(struct SystemMessageData));
            data_len = sizeof(struct SystemMessageData);
            data = (uint8_t*)&system;
        }

        json toJson() override {
            json j;
            j["Flags"] = std::to_string(system.flags);
            j["Operator Latitude"] = std::to_string(system.operatorLatitude);
            j["Operator Longitude"] = std::to_string(system.operatorLongitude);
            j["Area Count"] = std::to_string(system.areaCount);
            j["Area Radius"] = std::to_string(system.areaRadius);
            j["Area Ceiling"] = std::to_string(system.areaRadius);
            j["Area Floor"] = std::to_string(system.areaFloor);;
            return j;
        }
};

#endif // #ifndef SYSTEM_MESSAGE_H