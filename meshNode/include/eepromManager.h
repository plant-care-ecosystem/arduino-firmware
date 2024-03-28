#include <EEPROM.h>

// #define NODE_ID_TYPE 'N'
// #define SENSOR_1_TYPE '1'
// #define SENSOR_2_TYPE '2'
// #define SENSOR_3_TYPE '3'
// #define SENSOR_4_TYPE '4'

#define EEPROM_SIZE 1024
#define EEPROM_UNIT_SIZE 100

#define NODE_ID_CHANGE_FLAG 0
#define NODE_ID 5

#define SENSOR_1_CHANGE_FLAG 1
#define SENSOR_2_CHANGE_FLAG 2
#define SENSOR_3_CHANGE_FLAG 3
#define SENSOR_4_CHANGE_FLAG 4
#define SENSOR_1_ID 106
#define SENSOR_2_ID 206
#define SENSOR_3_ID 306
#define SENSOR_4_ID 406
#define SENSOR_1_TNAME 506
#define SENSOR_2_TNAME 516
#define SENSOR_3_TNAME 526
#define SENSOR_4_TNAME 536

int nodeID;
int nodeIDChangeFlag;

int sensor1ID;
int sensor2ID;
int sensor3ID;
int sensor4ID;

int sensor1ChangeFlag = 0;
int sensor2ChangeFlag = 0;
int sensor3ChangeFlag = 0;
int sensor4ChangeFlag = 0;

int sensor1Type = -1;
int sensor2Type = -1;
int sensor3Type = -1;
int sensor4Type = -1;

int returnChangeFlag(int idType) {
    switch (idType) {
        case 0:
            return nodeIDChangeFlag;
        case 1:
            return sensor1ChangeFlag;
        case 2:
            return sensor2ChangeFlag;
        case 3:
            return sensor3ChangeFlag;
        case 4:
            return sensor4ChangeFlag;
    }
    return -1;
    
}

// Loop through EEPROM and read each byte into string
int readEEPROMChars(int offset) {
    String id = "";
    for(int i = 0; i < EEPROM_UNIT_SIZE; i++) {
        char c = EEPROM.read(offset + i);
        // break if null terminator is found
        if(c == '\0') {
            break;
        }
        id += c;
    }
    return id.toInt();
}

// Read the changeFlag
void readChangeFlags() {
    nodeIDChangeFlag = EEPROM.read(NODE_ID_CHANGE_FLAG);
    // Check if value is 255, if so, set flag to 0
    if(nodeIDChangeFlag == 255) {
        nodeIDChangeFlag = 0;
    }
    sensor1ChangeFlag = EEPROM.read(SENSOR_1_CHANGE_FLAG);
    if(sensor1ChangeFlag == 255) {
        sensor1ChangeFlag = 0;
    }
    sensor2ChangeFlag = EEPROM.read(SENSOR_2_CHANGE_FLAG);
    if(sensor2ChangeFlag == 255) {
        sensor2ChangeFlag = 0;
    }
    sensor3ChangeFlag = EEPROM.read(SENSOR_3_CHANGE_FLAG);
    if(sensor3ChangeFlag == 255) {
        sensor3ChangeFlag = 0;
    }
    sensor4ChangeFlag = EEPROM.read(SENSOR_4_CHANGE_FLAG);
    if(sensor4ChangeFlag == 255) {
        sensor4ChangeFlag = 0;
    }
    Serial.printf("Node ID Change Flag: %d\n", nodeIDChangeFlag);
    Serial.printf("Sensor 1 Change Flag: %d\n", sensor1ChangeFlag);
    Serial.printf("Sensor 2 Change Flag: %d\n", sensor2ChangeFlag);
    Serial.printf("Sensor 3 Change Flag: %d\n", sensor3ChangeFlag);
    Serial.printf("Sensor 4 Change Flag: %d\n", sensor4ChangeFlag);
}

void readSensorTypes() {
    if(returnChangeFlag(1) == 1) {
        sensor1Type = readEEPROMChars(SENSOR_1_TNAME);
    }
    
    if(returnChangeFlag(2) == 1) {
        sensor2Type = readEEPROMChars(SENSOR_2_TNAME);
    }

    if(returnChangeFlag(3) == 1) {
        sensor3Type = readEEPROMChars(SENSOR_3_TNAME);
    }

    if(returnChangeFlag(4) == 1) {
        sensor4Type = readEEPROMChars(SENSOR_4_TNAME);
    }

    Serial.printf("Sensor 1 Type: %d\n", sensor1Type);
    Serial.printf("Sensor 2 Type: %d\n", sensor2Type);
    Serial.printf("Sensor 3 Type: %d\n", sensor3Type);
    Serial.printf("Sensor 4 Type: %d\n", sensor4Type);
}

void readSensorIDs() {
    if(returnChangeFlag(1) == 1) {
        sensor1ID = readEEPROMChars(SENSOR_1_ID);
    }
    
    if(returnChangeFlag(2) == 1) {
        sensor2ID = readEEPROMChars(SENSOR_2_ID);
    }

    if(returnChangeFlag(3) == 1) {
        sensor3ID = readEEPROMChars(SENSOR_3_ID);
    }

    if(returnChangeFlag(4) == 1) {
        sensor4ID = readEEPROMChars(SENSOR_4_ID);
    }
}

int returnSensorType(int idType) {
    switch (idType) {
        case 1:
            return sensor1Type;
        case 2:
            return sensor2Type;
        case 3:
            return sensor3Type;
        case 4:
            return sensor4Type;
    }
    return -1;
}

int returnSensorTypeOffset(int idType) {
    switch (idType) {
        case 1:
            return SENSOR_1_TNAME;
        case 2:
            return SENSOR_2_TNAME;
        case 3:
            return SENSOR_3_TNAME;
        case 4:
            return SENSOR_4_TNAME;
    }
    return -1;
}

int returnChangeFlagOffset(int idType) {
    switch (idType) {
        case 0:
            return NODE_ID_CHANGE_FLAG;
        case 1:
            return SENSOR_1_CHANGE_FLAG;
        case 2:
            return SENSOR_2_CHANGE_FLAG;
        case 3:
            return SENSOR_3_CHANGE_FLAG;
        case 4:
            return SENSOR_4_CHANGE_FLAG;
    }
}

void setSingleSensorType(int idType, int currentSensorType) {
    int offset = returnSensorTypeOffset(idType);
    String sensorTypeString = String(currentSensorType).c_str();
    // Store the new sensor type in RAM
    switch (idType) {
        case 1:
            sensor1Type = currentSensorType;
            break;
        case 2:
            sensor2Type = currentSensorType;
            break;
        case 3:
            sensor3Type = currentSensorType;
            break;
        case 4:
            sensor4Type = currentSensorType;
            break;
    }
    Serial.printf("Offset: %d\n", offset);
    Serial.printf("changeFlag: %d\n", returnChangeFlag(idType));
    Serial.printf("Sensor Type String: %s\n", sensorTypeString);
    // Store the new sensor type in EEPROM as a string
    EEPROM.put(offset, sensorTypeString); // c_str for null termination
    EEPROM.commit();
    EEPROM.put(returnChangeFlagOffset(idType), 1);
    EEPROM.commit();
}

// // Convert an integer to a char array with a \0 terminator
// void intToCharArray(int id, char* idArray) {
//     sprintf(idArray, "%d", id);
//     idArray[strlen(idArray)] = '\0';
// }



// class UserEEPROMManager {
// public:
//     /**
//      * Constructor for the EEPROMManager class.
//      * Initializes the EEPROM with 512 bytes.
//      */
//     UserEEPROMManager() {
//         EEPROM.begin(this->EEPROM_SIZE);
//     }

//     /**
//      * Load each ID from EEPROM if they have been set.
//      */
//     void load() {
//         // Load Node ID
//         if (EEPROM.read(this->NODE_ID_CHANGE_FLAG) == 1) {
//             this->nodeID = this->read(this->NODE_ID);
//         }

//         // Load Sensor 1 ID
//         if (EEPROM.read(this->SENSOR_1_CHANGE_FLAG) == 1) {
//             this->sensor1ID = this->read(this->SENSOR_1_ID);
//         }
        
//         // Load Sensor 2 ID
//         if (EEPROM.read(this->SENSOR_2_CHANGE_FLAG) == 1) {
//             this->sensor2ID = this->read(this->SENSOR_2_ID);
//         }

//         // Load Sensor 3 ID
//         if (EEPROM.read(this->SENSOR_3_CHANGE_FLAG) == 1) {
//             this->sensor3ID = this->read(this->SENSOR_3_ID);
//         }
        
//         // Load Sensor 4 ID
//         if (EEPROM.read(this->SENSOR_4_CHANGE_FLAG) == 1) {
//             this->sensor4ID = this->read(this->SENSOR_4_ID);
//         }

//         this->updateChangeFlag();
//     }

//     void loadSensorType() {
//         // Load Sensor 1 Type
//         if (EEPROM.read(this->SENSOR_1_CHANGE_FLAG) == 1) {
//             this->sensor1Type = this->read(this->SENSOR_1_TNAME);
//         }
        
//         // Load Sensor 2 Type
//         if (EEPROM.read(this->SENSOR_2_CHANGE_FLAG) == 1) {
//             this->sensor2Type = this->read(this->SENSOR_2_TNAME);
//         }

//         // Load Sensor 3 Type
//         if (EEPROM.read(this->SENSOR_3_CHANGE_FLAG) == 1) {
//             this->sensor3Type = this->read(this->SENSOR_3_TNAME);
//         }
        
//         // Load Sensor 4 Type
//         if (EEPROM.read(this->SENSOR_4_CHANGE_FLAG) == 1) {
//             this->sensor4Type = this->read(this->SENSOR_4_TNAME);
//         }

//         this->updateChangeFlag();
//     }

//     void loadChangeFlag() {
//         this->updateChangeFlag();
//     }

//     /**
//      * Save the ID to EEPROM and update the change flag.
//      * 
//      * @param idType The type of ID to save ('N' for Node ID, '1' for Sensor 1 ID, '2' for Sensor 2 ID, '3' for Sensor 3 ID, '4' for Sensor 4 ID).
//      * @param id The ID value to save.
//      */
//     void save(int idType, const char* id, const char* sensorType) {
//         Serial.printf("Saving ID: %s\n", id);
//         Serial.printf("Saving Sensor Type: %s\n", sensorType);
//         Serial.printf("ID Type: %d\n", idType);
//         switch (idType) {
//             case 0:
//                 Serial.println("Saving Node ID");
//                 EEPROM.put(this->NODE_ID, id);
//                 // Add null terminator
//                 EEPROM.put(this->NODE_ID + strlen(id), '\0');
//                 EEPROM.put(this->NODE_ID_CHANGE_FLAG, 1);
//                 nodeID = atoi(id);
//                 break;
//             case 1:
//                 Serial.println("Saving Sensor 1 ID");
//                 EEPROM.put(this->SENSOR_1_ID, id);
//                 // Add null terminator
//                 EEPROM.put(this->SENSOR_1_ID + strlen(id), '\0');

//                 EEPROM.put(this->SENSOR_1_TNAME, sensorType);
//                 // Add null terminator
//                 EEPROM.put(this->SENSOR_1_TNAME + strlen(sensorType), '\0');

//                 EEPROM.put(this->SENSOR_1_CHANGE_FLAG, 1);
//                 sensor1ID = atoi(id);
//                 break;
//             case 2:
//                 Serial.println("Saving Sensor 2 ID");
//                 EEPROM.put(this->SENSOR_2_ID, id);
//                 // Add null terminator
//                 EEPROM.put(this->SENSOR_2_ID + strlen(id), '\0');

//                 EEPROM.put(this->SENSOR_2_TNAME, sensorType);
//                 // Add null terminator
//                 EEPROM.put(this->SENSOR_2_TNAME + strlen(sensorType), '\0');

//                 EEPROM.put(this->SENSOR_2_CHANGE_FLAG, 1);
//                 nodeID = atoi(id);
//                 break;
//             case 3:
//                 Serial.println("Saving Sensor 3 ID");
//                 EEPROM.put(this->SENSOR_3_ID, id);
//                 // Add null terminator
//                 EEPROM.put(this->SENSOR_3_ID + strlen(id), '\0');
//                 EEPROM.write(this->SENSOR_3_CHANGE_FLAG, 1);
//                 nodeID = atoi(id);
//                 break;
//             case 4:
//                 Serial.println("Saving Sensor 4 ID");
//                 EEPROM.put(this->SENSOR_4_ID, id);
//                 // Add null terminator
//                 EEPROM.put(this->SENSOR_4_ID + strlen(id), '\0');
//                 EEPROM.put(this->SENSOR_4_CHANGE_FLAG, 1);
//                 nodeID = atoi(id);
//                 break;
//         }
//         Serial.println("Committing changes to EEPROM");
//         EEPROM.commit();
//         this->updateChangeFlag();
//     }

//     /**
//      * Returns the ID based on the given idType.
//      *
//      * @param idType The type of ID to retrieve. Valid values are 'N', '1', '2', '3', '4'.
//      * @return The ID corresponding to the given idType.
//      */
//     int getID(int idType) {
//         switch (idType) {
//             case 0:
//                 return this->nodeID;
//             case 1:
//                 return this->sensor1ID;
//             case 2:
//                 return this->sensor2ID;
//             case 3:
//                 return this->sensor3ID;
//             case 4:
//                 return this->sensor4ID;
//         }
//         return -1;
//     }

//     int getSensorType(int idType) {
//         switch (idType) {
//             case 1:
//                 return this->sensor1Type;
//             case 2:
//                 return this->sensor2Type;
//             case 3:
//                 return this->sensor3Type;
//             case 4:
//                 return this->sensor4Type;
//         }
//         return -1;
//     }

//     uint8_t getChangeFlag() {
//         return this->changeFlag;
//     }

// private:


//     /**
//      * Write the ID to the specified EEPROM offset.
//      * 
//      * @param id The ID value to write.
//      * @param offset The EEPROM offset to write the ID to.
//      */
//     // void write(String id, int offset) {
//     //     for(int i = 0; i < id.length(); i++) {
//     //         EEPROM.write(offset + i, id[i]);
//     //     }
//     //     // Add null terminator
//     //     EEPROM.write(NODE_NAME + id.length(), '\0');
//     // }

//     // Loop through EEPROM and read each byte into string
//     int read(int offset) {
//         String id = "";
//         for(int i = 0; i < this->EEPROM_UNIT_SIZE; i++) {
//             char c = EEPROM.read(offset + i);
//             // break if null terminator is found
//             if(c == '\0') {
//                 break;
//             }
//             id += c;
//         }
//         return id.toInt();
//     }

//     void updateChangeFlag() {
//         this->changeFlag = 0;

//         if(this->read(this->NODE_ID_CHANGE_FLAG) == 1){
//             this->changeFlag |= 1 << 0;
//         }

//         if(this->read(this->SENSOR_1_CHANGE_FLAG) == 1) {
//             this->changeFlag |= 1 << 1;
//         }

//         if(this->read(this->SENSOR_2_CHANGE_FLAG) == 1) {
//             this->changeFlag |= 1 << 2;
//         }

//         if(this->read(this->SENSOR_3_CHANGE_FLAG) == 1) {
//             this->changeFlag |= 1 << 3;
//         }

//         if(this->read(this->SENSOR_4_CHANGE_FLAG) == 1) {
//             this->changeFlag |= 1 << 4;
//         }
//     }

// };