#ifndef SYSTEM_MESSAGE_H
#define SYSTEM_MESSAGE_H

#include <stdint.h>

struct systemMessageData {
    uint8_t flags;
    int32_t operatorLatitude;
    int32_t operatorLongitude;
    uint16_t areaCount;
    uint8_t areaRadius;
    uint16_t areaCeiling;
    uint16_t areaFloor;
    uint8_t reserved[8];
} __attribute__((packed));

class SystemMessage: public MessageBody {
    public:

        struct systemMessageData system;

        SystemMessage(
            uint8_t flags,
            int32_t operatorLatitude,
            int32_t operatorLongitude,
            uint16_t areaCount,
            uint8_t areaRadius,
            uint16_t areaCeiling,
            uint16_t areaFloor
        ) {
            system.flags = flags;
            system.areaRadius = areaRadius;
            system.areaCount = areaCount;
            system.operatorLatitude = operatorLatitude;
            system.operatorLongitude = operatorLongitude;
            system.areaCount = areaCount;
            system.areaCeiling = areaCeiling;
            system.areaFloor = areaFloor;
            data_len = sizeof(struct systemMessageData);
            data = (uint8_t*)&system;
        };

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