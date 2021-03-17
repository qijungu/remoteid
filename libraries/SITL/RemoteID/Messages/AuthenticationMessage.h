#ifndef AUTHENTICATION_MESSAGE_H
#define AUTHENTICATION_MESSAGE_H

#include <stdint.h>

// 5.4.5.9 Authentication Message Type: 0x2, Static Periodicity, Optional
struct AuthenticationMessageData {
    unsigned authType : 4;   // enum RID_Auth_Type
    unsigned pageNumber : 4;
    unsigned pageReserved : 4;
    unsigned pageCount : 4;  // total page count, up to 5 pages
    uint8_t length : 8;
    uint32_t timestamp;      // 32 bit Unix Timestamp in seconds since 00:00:00 01/01/2019 (to relate back to standard unix timestamp, add 1546300800 to base it on 00:00:00 01/01/1970)
    uint8_t authenticationData[17];
} __attribute__((packed));

class AuthenticationMessage: public MessageBody {
    public:

        struct AuthenticationMessageData authentication;
        
        AuthenticationMessage(
            uint8_t authType, 
            uint8_t pageNumber,
            uint8_t pageCount,
            uint8_t length,
            uint32_t timestamp,
            const uint8_t authenticationData[17]
        ) {
            authentication.authType = authType;
            authentication.pageNumber = pageNumber;
            authentication.pageCount = pageCount;
            authentication.length = length;
            authentication.timestamp = timestamp;
            memcpy(authentication.authenticationData, authenticationData, 17);
            data_len = sizeof(struct AuthenticationMessageData);
            data = (uint8_t*)&authentication;
        }

        json toJson() override {
            json j;
            j["Auth Type"] = std::to_string(authentication.authType);
            j["Page Number"] = std::to_string(authentication.pageNumber);
            j["Page Count"] = std::to_string(authentication.pageCount);
            j["Length"] = std::to_string(authentication.length);
            j["Timestamp"] = std::to_string(authentication.timestamp);
            j["Authentication Data"] = "...";
            return j;
        }
};

#endif // AUTHENTICATION_MESSAGE_H