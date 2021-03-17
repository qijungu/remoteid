#ifndef BASICID_MESSAGE_H
#define BASICID_MESSAGE_H

#include <stdint.h>

// 5.4.5.5 Basic ID Message Type: 0x0, Static Periodicity, Mandatory
struct BasicMessageData {
    unsigned idType : 4;  // enum RID_ID_Type
    unsigned uaType : 4;  // enum RID_UA_Type
    char uasId[RIDLEN];   // 20 bytes
    uint8_t reserve[3];   // reserved
} __attribute__((packed));

class BasicIDMessage: public MessageBody {
    public:

        struct BasicMessageData basicIDMessage;

        BasicIDMessage(
           uint8_t idType,
           uint8_t uaType,
           const uint8_t uasId[20]
        ) {
            basicIDMessage.idType = idType;
            basicIDMessage.uaType = uaType;
            memcpy(basicIDMessage.uasId, uasId, RIDLEN);
            uint8_t reserve[3] = {0,0,0};
            memcpy(basicIDMessage.reserve, reserve, 3);
            data_len = sizeof(struct BasicMessageData);
            data = (uint8_t*)&basicIDMessage;
        };

        json toJson() override {
            json j;
            j["ID Type"] = std::to_string(basicIDMessage.idType);
            j["UA Type"] = std::to_string(basicIDMessage.uaType);
            j["UAS ID"] = basicIDMessage.uasId;
            return j;
        }
};

#endif // BASICID_MESSAGE_H