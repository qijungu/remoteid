#ifndef MESSAGEPACK_H
#define MESSAGEPACK_H

#include <stdint.h>
#include <vector> 

struct MessagePackData {
    MessagePackData() : MessageSize(0x19), NumberMessages(0){}
    unsigned MessageSize : 8;
    unsigned NumberMessages : 8;
} __attribute__((packed));

class MessagePack {
    public:

        struct MessagePackData pack;

        json messagePack;

        std::vector<json> messagesVector;

        MessagePack() {
            messagePack["Message Size"] = "0x19";
            messagePack["No of Msgs"] = "0";
            messagePack["Messages"] = messagesVector;

        }

        void addMessage(MessageHeader *header, MessageBody *body){
            
            /*
            pack.NumberMessages++;
            messagePack["No of Msgs"] = std::to_string(pack.NumberMessages);
            json message;
            message["Message Header"] = header->toJson();
            message["Message Body"] = body->toJson();
            messagesVector.push_back(message);
            messagePack["Messages"] = messagesVector;
            */
        }

        json toJson(){
            return messagePack;
        }
};

#endif // MESSAGEPACK_H