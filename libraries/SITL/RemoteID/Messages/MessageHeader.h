#ifndef MESSAGE_HEADER_H
#define MESSAGE_HEADER_H

#include <stdint.h>
#include <string>

#include "MessageBody.h"

struct MessageHeaderData {
    unsigned MessageType : 4;
    unsigned ProtocolVersion : 4;
} __attribute__((packed));

class MessageHeader {
    public:
        struct MessageHeaderData header;
        const uint8_t* data = (uint8_t*)&header;
        const size_t data_len = sizeof(struct MessageHeaderData);

        MessageHeader(uint8_t messageType) {
            header.MessageType = messageType & 0xF;
            header.ProtocolVersion = 0x0;
        }

        MessageHeader(const uint8_t* _header) {
            memcpy(&header, _header, data_len);
        }

        json toJson() {
            json j;
            j["Message Type"] = "0x" + std::to_string(header.MessageType);
            j["Protocol Version"] = "0x" + std::to_string(header.ProtocolVersion);
            return j;
        }
};

#endif // MESSAGE_HEADER_H