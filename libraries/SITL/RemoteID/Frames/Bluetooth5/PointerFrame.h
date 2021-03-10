struct Bluetooth5PointerFrame {
    unsigned Preamble : 8;
    unsigned AccAddr : 48;
    unsigned Cl : 2;
    unsigned TERM1 : 3;
    unsigned PDUHdr : 16;
    unsigned ExtHdr : 6;
    unsigned AdvMode : 2;
    unsigned ExtHeader : 96;
    unsigned CRC : 24;
    unsigned Term2 : 3;
};