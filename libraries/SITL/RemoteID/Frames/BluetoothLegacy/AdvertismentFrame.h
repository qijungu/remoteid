struct LegacyAdvertismenentFrame {
    unsigned Preamble : 8;
    unsigned AccAddress : 48;
    unsigned PDUHdr : 16;
    unsigned ADAddr : 48;
    unsigned ADInfo : 32;
    unsigned ADApp : 8;
    unsigned ADCounter : 8;
    unsigned ODIDMsg : 200;
    unsigned CRC : 24;
};