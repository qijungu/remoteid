#ifndef MESSAGEPACK_H
#define MESSAGEPACK_H

#include <stdint.h>
#include <vector> 

// 5.4.5.22 Message Pack Message Type: 0xF, Dynamic Periodicity if dynamic message in contents
struct MessagePackData {
    uint8_t messageSize;  // Size of single message in message pack. Set to 0x19 (25).
    uint8_t numberMessages;  // Number of messages (N) contained in message. Up to 10
    uint8_t messages[10][25];  // Series of concatenated messages in message number order. Up to 250 bytes
} __attribute__((packed));

class MessagePack: public MessageBody {
    public:

        struct MessagePackData pack;

        json messagePack;

        std::vector<json> messagesVector;

        MessagePack() {
            clearPack();
        }

        MessagePack(const uint8_t* d) {
            clearPack();
            uint8_t count = (uint8_t)d[1];
            if (count > 10) return;
            memcpy(&pack, d, 2+count*25);
        }

        void addMessage(const uint8_t msg[25]){
            if (pack.numberMessages >= 10) return; 
            memcpy(pack.messages[pack.numberMessages], msg, 25);
            pack.numberMessages++;
            data_len += 25;
        }

        void clearPack() {
            pack.messageSize = 25;
            pack.numberMessages = 0;
            memset(pack.messages, 0, 250);
            data = (uint8_t*)&pack;
            data_len = 2;
        }

        json toJson() override {
            json j;
            j["Message Size"] = "0x19";
            j["No of Msgs"] = "0";
            j["Messages"] = messagesVector;
            return j;
        }
};

#endif // MESSAGEPACK_H