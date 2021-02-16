#ifndef OPERATIONID_MESSAGE_H
#define OPERATIONID_MESSAGE_H

#include <stdint.h>

struct operatorIDMessageData {
    unsigned operatorIdType : 8;
    char operatorId[20];
    uint8_t reserved[3];
} __attribute__((packed));

class OperatorIDMessage: public MessageBody {
    public:

        struct operatorIDMessageData operatorID;

        OperatorIDMessage(
            uint8_t operatorIdType,
            char operatorId[20]
        ) {
            operatorID.operatorIdType = operatorIdType;
            memcpy(operatorID.operatorId, operatorId, 20);
            data_len = sizeof(struct operatorIDMessageData);
            data = (uint8_t*)&operatorID;
        };

        json toJson() override {
            json j;
            j["OperatorID Type"] = std::to_string(operatorID.operatorIdType);
            j["Operator ID"] = operatorID.operatorId;
            return j;
        }
};

#endif // OPERATIONID_MESSAGE_H