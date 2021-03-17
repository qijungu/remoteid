#ifndef AUTHENTICATION_MESSAGE_PAGES_H
#define AUTHENTICATION_MESSAGE_PAGES_H

#include <stdint.h>

// 5.4.5.9 Authentication Message Type: 0x2, Static Periodicity, Optional
struct AuthenticationMessagePagesData {
    unsigned authType : 4;  // enum RID_Auth_Type
    unsigned pageNumber : 4;
    uint8_t authenticationData[23];
} __attribute__((packed));

class AuthenticationMessagePages: public MessageBody {
    public:

        struct AuthenticationMessagePagesData authenticationPages;

        AuthenticationMessagePages(
            uint8_t authType,
            uint8_t pageNumber,
            const uint8_t authenticationData[23]
        ) {
            authenticationPages.authType = authType;
            authenticationPages.pageNumber = pageNumber;
            memcpy(authenticationPages.authenticationData, authenticationData, 23);
            data_len = sizeof(struct AuthenticationMessagePagesData);
            data = (uint8_t*)&authenticationPages;
        };

        json toJson() override {
            json j;
            j["Auth Type"] = std::to_string(authenticationPages.authType);
            j["Page Number"] = std::to_string(authenticationPages.pageNumber);
            j["Authentication Data"] = "...";
            return j;
        }
};

#endif // AUTHENTICATION_MESSAGE_PAGES_H