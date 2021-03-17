#ifndef SELFID_MESSAGE_H
#define SELFID_MESSAGE_H

#include <stdint.h>

// 5.4.5.16 Self ID Message Type: 0x3, Static Periodicity, Optional
struct SelfIDMessageData {
    uint8_t descriptionType;  // 0: text description
    uint8_t description[23];
} __attribute__((packed));

class SelfIDMessage: public MessageBody {
    public:

        struct SelfIDMessageData selfID;

        SelfIDMessage(
            uint8_t descriptionType,
            const char description[23]
        ) {
            assert(sizeof(struct SelfIDMessageData)==24);
            memset(&selfID, 0, sizeof(struct SelfIDMessageData));
            selfID.descriptionType = descriptionType;
            memcpy(selfID.description, description, 23);
            data_len = sizeof(struct SelfIDMessageData);
            data = (uint8_t*)&selfID;
        };

        SelfIDMessage(uint8_t* d) {
            assert(sizeof(struct SelfIDMessageData)==24);
            memset(&selfID, 0, sizeof(struct SelfIDMessageData));
            memcpy(&selfID, d, sizeof(struct SelfIDMessageData));
            data_len = sizeof(struct SelfIDMessageData);
            data = (uint8_t*)&selfID;
        }

        json toJson() override {
            json j;
            j["Description Type"] = std::to_string(selfID.descriptionType);
            j["UAS ID"] = selfID.description;
            return j;
        }

};

#endif // SELFID_MESSAGE_H