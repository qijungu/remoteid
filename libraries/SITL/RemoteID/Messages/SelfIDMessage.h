#ifndef SELFID_MESSAGE_H
#define SELFID_MESSAGE_H

#include <stdint.h>

struct selfIDMessageData {
    unsigned descriptionType : 8;
    char description[23];
} __attribute__((packed));

class SelfIDMessage: public MessageBody {
    public:

        struct selfIDMessageData selfID;

        SelfIDMessage(
            uint8_t descriptionType,
            const char description[23]
        ) {
            selfID.descriptionType = descriptionType;
            memcpy(selfID.description, description, 23);
            data_len = sizeof(struct selfIDMessageData);
            data = (uint8_t*)&selfID;
        };

        json toJson() override {
            json j;
            j["Description Type"] = std::to_string(selfID.descriptionType);
            j["UAS ID"] = selfID.description;
            return j;
        }

};

#endif // SELFID_MESSAGE_H