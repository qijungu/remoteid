#ifndef LOCATION_VECTOR_MESSAGE_H
#define LOCATION_VECTOR_MESSAGE_H

#include <stdint.h>

// 5.4.5.7 Location/Vector Message Type: 0x1, Dynamic Periodicity, Mandatory
struct LocationVectorMessageData {
    unsigned flag_speed_multiplier : 1;  // enum RID_Speed_Multiplier
    unsigned flag_ew_direction : 1;      // enum RID_EW_Direction
    unsigned flag_height_type : 1;       // enum RID_Height_Type
    unsigned flag_reserved : 1;
    unsigned status : 4;                 // enum RID_OP_Status
    uint8_t trackDirection;              // Direction expressed as the route course measured clockwise from true north. Encode as 0–179.
                                         // If E/W Direction bit is set, then 180 should be added to the value.
    uint8_t speed;                       // If value <= 225*0.25, encodedValue = value/0.25, set multiplier flag to 0
                                         // Else if value > 225*0.25 and value <254.25, encodedValue = (value – (225*0.25))/0.75, set multiplier flag to 1
                                         // Else value >= 254.25 m/s, encodedValue = 254, set multiplier flag to 1
    int8_t verticalSpeed;                // EncodedValue = Value(m/s) / 0.5, up to ±63.5 m/s (12.5k ft/min)
    int32_t latitude;                    // EncodedValue = Value(UA) * 10^7
    int32_t longitude;                   // EncodedValue = Value(UA) * 10^7
    uint16_t pressureAltitude;           // EncodedValue = (Value(m) + 1000) / 0.5
    uint16_t geodeticAltitude;           // EncodedValue = (Value(m) + 1000) / 0.5, WGS-84 HAE
    uint16_t height;                     // EncodedValue = (Value(m) + 1000) / 0.5, above takeoff or ground to flag height type
    unsigned horizontalAccuracy : 4;     // enum RID_Horizontal_Accuracy
    unsigned verticalAccuracy : 4;       // enum RID_Vertical_Accuracy
    unsigned speedAccuracy : 4;          // enum RID_Speed_Accuracy
    unsigned baroAltitudeAccuracy : 4;   // enum RID_Vertical_Accuracy
    uint16_t timestamp;                  // Time of applicability expressed in 1/10ths of seconds since the last hour
    unsigned timestampAccuracy : 4;      // Timestamp accuracy: Bits [3..0], 0.1 s stepping resolutions, 0.1 s – 1.5 s, 0=unknown
    unsigned reserved23 : 4;
    uint8_t reserved24;
} __attribute__((packed));

class LocationVectorMessage: public MessageBody {
    public:
        struct LocationVectorMessageData locationVector;

        LocationVectorMessage(
            uint8_t status,
            uint8_t flag_height_type,       // enum RID_Height_Type
            uint8_t flag_ew_direction,      // enum RID_EW_Direction
            uint8_t flag_speed_multiplier,  // enum RID_Speed_Multiplier
            uint8_t trackDirection,
            uint8_t speed,
            int8_t verticalSpeed,
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
            assert(sizeof(struct LocationVectorMessageData)==24);
            memset(&locationVector, 0, sizeof(struct LocationVectorMessageData));
            locationVector.status = status;
            locationVector.flag_height_type = flag_height_type;
            locationVector.flag_ew_direction = flag_ew_direction;
            locationVector.flag_speed_multiplier = flag_speed_multiplier;
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
            data_len = sizeof(struct LocationVectorMessageData);
            data = (uint8_t*)&locationVector;
        };

        LocationVectorMessage(const uint8_t* d) {
            assert(sizeof(struct LocationVectorMessageData)==24);
            memset(&locationVector, 0, sizeof(struct LocationVectorMessageData));
            memcpy(&locationVector, d, sizeof(struct LocationVectorMessageData));
            data_len = sizeof(struct LocationVectorMessageData);
            data = (uint8_t*)&locationVector;
        }

        json toJson() override {
            json j;
            j["Status"] = std::to_string(locationVector.status);
            j["Flag_height_type"] = std::to_string(locationVector.flag_height_type);
            j["Flag_ew_direction"] = std::to_string(locationVector.flag_ew_direction);
            j["Flag_speed_multiplier"] = std::to_string(locationVector.flag_speed_multiplier);
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