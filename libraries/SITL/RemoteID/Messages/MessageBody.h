#ifndef MESSAGEBODY_H
#define MESSAGEBODY_H

#include "../json.h"
using json = nlohmann::json;

class MessageBody {
    public:

        uint8_t* data;
        size_t data_len;

        MessageBody() : data(NULL), data_len(0) {}

        virtual json toJson() {
            json j;
            return j;
        }
};

#endif // MESSAGEBODY_H