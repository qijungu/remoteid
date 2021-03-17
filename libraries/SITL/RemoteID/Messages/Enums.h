#ifndef ENUMS_H
#define ENUMS_H

// Remote ID message type
enum RID_Msg_Type {
    RID_BasicID = 0,       // static, mandatory
    RID_Location = 1,    // dynamic, mandatory
    RID_Auth = 2,        // static, optional
    RID_SelfID = 3,      // static, optional
    RID_System = 4,      // static, optional
    RID_OperatorID = 5,  // static, optional
    RID_MsgPack = 0xF,   // static/dynamic, optioanl
};

enum RID_ID_Type {
    RID_ID_Type_None = 0 ,    // None
    RID_ID_Type_Serial = 1 ,  // Serial Number (ANSI/CTA-2063-A)
    RID_ID_Type_CAA = 2 ,     // CAA Assigned Registration ID
    RID_ID_Type_UTM = 3 ,     // UTM Assigned UUID
};

enum RID_UA_Type {
    RID_UA_Type_None = 0,              // None/Not Declared Up to 16 Types
    RID_UA_Type_Aeroplane = 1,         // Aeroplane
    RID_UA_Type_Helicopter = 2,        // Helicopter (or Multirotor) These values were derived from the official ICAO UA Type list. Additional
    RID_UA_Type_Gyroplane = 3,         // Gyroplane types were added if they had unique flight characteristics.
    RID_UA_Type_Hybrid= 4,             // Hybrid Lift (Fixed wing aircraft that can take off vertically)
    RID_UA_Type_Ornithopter= 5,        // Ornithopter
    RID_UA_Type_Glider = 6,            // Glider
    RID_UA_Type_Kite = 7,              // Kite
    RID_UA_Type_FreeBalloon = 8,       // Free Balloon
    RID_UA_Type_CaptiveBalloon = 9,    // Captive Balloon
    RID_UA_Type_Airship = 10,          // Airship (such as a blimp)
    RID_UA_Type_Parachute = 11,        // Free Fall/Parachute (unpowered)
    RID_UA_Type_Rocket = 12,           // Rocket
    RID_UA_Type_Tethered = 13,         // Tethered Powered Aircraft
    RID_UA_Type_Obstacle = 14,         // Ground Obstacle
    RID_UA_Type_Other = 15,            // Other
};

enum RID_OP_Status {
    RID_OP_Status_Undeclared = 0, // Undeclared
    RID_OP_Status_Ground = 1,     // Ground
    RID_OP_Status_Airborne = 2,   // Airborne
                                // 3-15: Reserved
};

enum RID_Horizontal_Accuracy {
    RID_Horizontal_Accuracy_Unknown = 0,  // >=18.52 km (10 NM) or Unknown
    RID_Horizontal_Accuracy_10NM = 1,     // <18.52 km (10 NM)
    RID_Horizontal_Accuracy_4NM = 2,      // <7.408 km (4 NM)
    RID_Horizontal_Accuracy_2NM = 3,      // <3.704 km (2 NM)
    RID_Horizontal_Accuracy_1NM = 4,      // <1852 m (1 NM)
    RID_Horizontal_Accuracy_05NM = 5,     // <926 m (0.5 NM)
    RID_Horizontal_Accuracy_03NM = 6,     // <555.6 m (0.3 NM)
    RID_Horizontal_Accuracy_01NM = 7,     // <185.2 m (0.1 NM)
    RID_Horizontal_Accuracy_005NM = 8,    // <92.6 m (0.05 NM)
    RID_Horizontal_Accuracy_30m = 9,      // <30 m
    RID_Horizontal_Accuracy_10m = 10,     // <10 m
    RID_Horizontal_Accuracy_3m = 11,      // <3 m
    RID_Horizontal_Accuracy_1m = 12,      // <1 m
                                          // 13-15: Reserved
};

enum RID_Vertical_Accuracy {
    RID_Vertical_Accuracy_Unknown = 0,  // >=150 m or Unknown
    RID_Vertical_Accuracy_150m = 1,     // <150 m
    RID_Vertical_Accuracy_45m = 2,      // <45 m
    RID_Vertical_Accuracy_25m = 3,      // <25 m
    RID_Vertical_Accuracy_10m = 4,      // <10 m
    RID_Vertical_Accuracy_3m = 5,       // <3 m
    RID_Vertical_Accuracy_1m = 6,       // <1 m
                                        // 7-15: Reserved
};

enum RID_Speed_Accuracy {
    RID_Speed_Accuracy_Unknown = 0,  // >=10 m/s or Unknown
    RID_Speed_Accuracy_10m = 1,      // <10 m/s
    RID_Speed_Accuracy_3m = 2,       // <3 m/s
    RID_Speed_Accuracy_1m = 3,       // <1 m/s
    RID_Speed_Accuracy_03m = 4,      // <0.3 m/s
                                     // 5-15: Reserved
};

enum RID_Height_Type {
    RID_Height_Type_ATO = 0,  // above takeoff
    RID_Height_Type_AGL = 1,  // above ground level
};

enum RID_EW_Direction {
    RID_EW_Direction_East = 0, // <180
    RID_EW_Direction_West = 1, // >=180
};

enum RID_Speed_Multiplier {
    RID_Speed_Multiplier_1Q = 0, // *1/4
    RID_Speed_Multiplier_3Q = 1, // *3/4
};

enum RID_Auth_Type {
    RID_Auth_Type_None = 0,     // None
    RID_Auth_Type_UasID = 1,    // UAS ID Signature
    RID_Auth_Type_OpID = 2,     // Operator ID Signature
    RID_Auth_Type_Message = 3,  // Message Set Signature
    RID_Auth_Type_NetID = 4,    // Authentication Provided by Network Remote ID
                                // 5-9: Reserved for Spec
                                // A-F: Available for Private Use
};

enum RID_OP_Location_Type {
    RID_OP_Location_Type_TakeOff = 0,  // Take Off
    RID_OP_Location_Type_GNSS = 1,     // Live GNSS
    RID_OP_Location_Type_Fixed = 2,    // Fixed Location
};

#endif // ENUMS_H
