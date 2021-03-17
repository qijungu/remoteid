#ifndef MESSAGEPACK_H
#define MESSAGEPACK_H

#include <stdint.h>
#include <vector> 

// 5.4.5.22 Message Pack Message Type: 0xF, Dynamic Periodicity if dynamic message in contents
struct MessagePackData {
    MessagePackData() : messageSize(0x19), numberMessages(0){}
    uint8_t messageSize;  // Size of single message in message pack. Set to 0x19 (25).
    uint8_t numberMessages;  // Number of messages (N) contained in message. Up to 10
    uint8_t messages[10][25];  // Series of concatenated messages in message number order. Up to 250 bytes
} __attribute__((packed));

class MessagePack {
    public:

        struct MessagePackData pack;

        json messagePack;

        std::vector<json> messagesVector;

        MessagePack() {
            clearPack();
            messagePack["Message Size"] = "0x19";
            messagePack["No of Msgs"] = "0";
            messagePack["Messages"] = messagesVector;

        }

        void addMessage(uint8_t msg[25]){
            if (pack.numberMessages >= 10) return; 
            memcpy(pack.messages[pack.numberMessages], msg, 25);
            pack.numberMessages++;
        }

        void clearPack() {
            pack.messageSize = 0x19;
            pack.numberMessages = 0;
            memset(pack.messages, 0, 250);
        }

        json toJson(){
            return messagePack;
        }
};

#endif // MESSAGEPACK_H