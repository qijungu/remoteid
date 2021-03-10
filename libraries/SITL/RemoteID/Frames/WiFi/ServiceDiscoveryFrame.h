struct ServiceDiscoveryFrame {
    unsigned CategoryID: 8;
    unsigned ActionField: 8;
    unsigned OUI: 24;
    unsigned OUIType: 8;
    unsigned AttributesID: 8;
    unsigned Length: 16;
    unsigned ServiceID: 48;
    unsigned InstanceID: 8;
    unsigned RequestorID: 8;
    unsigned ServiceControl: 8;
    unsigned ServiceInfoLength: 8;
};