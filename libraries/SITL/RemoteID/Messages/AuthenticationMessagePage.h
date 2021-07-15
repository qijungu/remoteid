#ifndef AUTHENTICATION_MESSAGE_PAGE_H
#define AUTHENTICATION_MESSAGE_PAGE_H

#include <stdint.h>

// 5.4.5.9 Authentication Message Type: 0x2, Static Periodicity, Optional
struct AuthenticationMessagePageData {
    unsigned pageNumber : 4;
    unsigned authType : 4;  // enum RID_Auth_Type
    uint8_t authenticationData[23];
} __attribute__((packed));

class AuthenticationMessagePage: public MessageBody {
    public:

        struct AuthenticationMessagePageData authenticationPage;

        AuthenticationMessagePage(
            uint8_t authType,
            uint8_t pageNumber,
            uint8_t pageDataLength,
            const uint8_t authenticationData[23]
        ) {
            assert(sizeof(struct AuthenticationMessagePageData)==24);
            memset(&authenticationPage, 0, sizeof(struct AuthenticationMessagePageData));
            authenticationPage.authType = authType;
            authenticationPage.pageNumber = pageNumber;
            if (pageDataLength > 23) pageDataLength = 23;
            memcpy(authenticationPage.authenticationData, authenticationData, pageDataLength);
            data_len = sizeof(struct AuthenticationMessagePageData);
            data = (uint8_t*)&authenticationPage;
        };

        AuthenticationMessagePage(const uint8_t* d) {
            assert(sizeof(struct AuthenticationMessagePageData)==24);
            memset(&authenticationPage, 0, sizeof(struct AuthenticationMessagePageData));
            memcpy(&authenticationPage, d, sizeof(struct AuthenticationMessagePageData));
            data_len = sizeof(struct AuthenticationMessagePageData);
            data = (uint8_t*)&authenticationPage;
        }

        json toJson() override {
            json j;
            j["Auth Type"] = std::to_string(authenticationPage.authType);
            j["Page Number"] = std::to_string(authenticationPage.pageNumber);
            j["Authentication Data"] = "...";
            return j;
        }
};

#endif // AUTHENTICATION_MESSAGE_PAGE_H