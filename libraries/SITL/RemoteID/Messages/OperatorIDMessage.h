#ifndef OPERATIONID_MESSAGE_H
#define OPERATIONID_MESSAGE_H

#include <stdint.h>

// 5.4.5.20 Operator ID Message Type: 0x5, Static Periodicity, Optional
struct operatorIDMessageData {
    uint8_t operatorIdType;
    uint8_t operatorId[20];
    uint8_t reserved[3];
} __attribute__((packed));

class OperatorIDMessage: public MessageBody {
    public:

        struct operatorIDMessageData operatorID;

        OperatorIDMessage(
            uint8_t operatorIdType,
            char operatorId[20]
        ) {
            assert(sizeof(struct operatorIDMessageData)==24);
            memset(&operatorID, 0, sizeof(struct operatorIDMessageData));
            operatorID.operatorIdType = operatorIdType;
            memcpy(operatorID.operatorId, operatorId, 20);
            data_len = sizeof(struct operatorIDMessageData);
            data = (uint8_t*)&operatorID;
        };

        OperatorIDMessage(uint8_t* d) {
            assert(sizeof(struct operatorIDMessageData)==24);
            memset(&operatorID, 0, sizeof(struct operatorIDMessageData));
            memcpy(&operatorID, d, sizeof(struct operatorIDMessageData));
            data_len = sizeof(struct operatorIDMessageData);
            data = (uint8_t*)&operatorID;
        }
        json toJson() override {
            json j;
            j["OperatorID Type"] = std::to_string(operatorID.operatorIdType);
            j["Operator ID"] = operatorID.operatorId;
            return j;
        }
};

#endif // OPERATIONID_MESSAGE_H