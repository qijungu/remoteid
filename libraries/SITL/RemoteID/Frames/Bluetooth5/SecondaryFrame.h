struct Bluetooth5SecondaryFrame { 
    unsigned Preamble : 8;
    unsigned AccAddr : 48;
    unsigned Cl : 2;
    unsigned Term1 : 3;
    unsigned PDUHdr : 16;
    unsigned HdrLen : 6;
    unsigned AdvMode : 2;
    unsigned ExtHdr : 72;
    unsigned ADInfo : 32;
    unsigned AppCode : 8;
    unsigned Counter : 8;
    unsigned CRC : 32;
    unsigned Term2 : 3;
};