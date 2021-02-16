#ifndef LOCATION_VECTOR_MESSAGE_H
#define LOCATION_VECTOR_MESSAGE_H

#include <stdint.h>

struct locationVectorMessageData {
    unsigned status : 4;
    unsigned flags : 4;
    uint8_t trackDirection;
    uint8_t speed;
    uint8_t verticalSpeed;
    int32_t latitude;
    int32_t longitude;
    uint16_t pressureAltitude;
    uint16_t geodeticAltitude;
    uint16_t height;
    unsigned verticalAccuracy : 4;
    unsigned horizontalAccuracy : 4;
    unsigned baroAltitudeAccuracy : 3;
    unsigned speedAccuracy : 3;
    uint16_t timestamp;
    unsigned reserved23 : 4;
    unsigned timestampAccuracy : 4;
    uint8_t reserved24;
} __attribute__((packed));

class LocationVectorMessage: public MessageBody {
    public:
        struct locationVectorMessageData locationVector;

        LocationVectorMessage(
            uint8_t status,
            uint8_t flags,
            uint8_t trackDirection,
            uint8_t speed,
            uint8_t verticalSpeed,
            int32_t latitude,
            int32_t longitude,
            uint16_t pressureAltitude,
            uint16_t geodeticAltitude,
            uint16_t height,
            uint8_t verticalAccuracy,
            uint8_t horizontalAccuracy,
            uint8_t baroAltitudeAccuracy,
            uint8_t speedAccuracy,
            uint16_t timestamp,
            uint8_t timestampAccuracy
        ) {
            locationVector.status = status;
            locationVector.flags = flags;
            locationVector.trackDirection = trackDirection;
            locationVector.speed = speed;
            locationVector.verticalSpeed = verticalSpeed;
            locationVector.verticalAccuracy = verticalAccuracy;
            locationVector.horizontalAccuracy = horizontalAccuracy;
            locationVector.baroAltitudeAccuracy = baroAltitudeAccuracy;
            locationVector.speedAccuracy = speedAccuracy;
            locationVector.timestampAccuracy = timestampAccuracy;
            locationVector.latitude = latitude;
            locationVector.longitude = longitude;
            locationVector.pressureAltitude = pressureAltitude;
            locationVector.geodeticAltitude = geodeticAltitude;
            locationVector.height = height;
            locationVector.timestamp = timestamp;
            data_len = sizeof(struct locationVectorMessageData);
            data = (uint8_t*)&locationVector;
        };

        json toJson() override {
            json j;
            j["Status"] = std::to_string(locationVector.status);
            j["Flags"] = std::to_string(locationVector.flags);
            j["Track Direction"] = std::to_string(locationVector.trackDirection);
            j["Speed"] = std::to_string(locationVector.speed);
            j["Vertical Speed"] = std::to_string(locationVector.verticalSpeed);
            j["Latitude"] = std::to_string(locationVector.latitude);
            j["Longitude"] = std::to_string(locationVector.longitude);
            j["Pressure Altitude"] = std::to_string(locationVector.pressureAltitude);
            j["Geodetic Altitude"] = std::to_string(locationVector.geodeticAltitude);
            j["Height"] = std::to_string(locationVector.height);
            j["Vertical Accuracy"] = std::to_string(locationVector.verticalAccuracy);
            j["Horizontal Accuracy"] = std::to_string(locationVector.horizontalAccuracy);
            j["Baro Accuracy"] = std::to_string(locationVector.baroAltitudeAccuracy);
            j["Speed Accuracy"] = std::to_string(locationVector.speedAccuracy);
            j["Timestamp"] = std::to_string(locationVector.timestamp);
            j["Timestamp Accuracy"] =std::to_string(locationVector.timestampAccuracy);
            return j;
        }

};

#endif // LOCATION_VECTOR_MESSAGE_H