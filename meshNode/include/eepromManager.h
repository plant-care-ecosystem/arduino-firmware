#include <EEPROM.h>

#define NODE_ID_TYPE 'N'
#define SENSOR_1_TYPE '1'
#define SENSOR_2_TYPE '2'
#define SENSOR_3_TYPE '3'
#define SENSOR_4_TYPE '4'

class UserEEPROMManager {
public:
    /**
     * Constructor for the EEPROMManager class.
     * Initializes the EEPROM with 512 bytes.
     */
    UserEEPROMManager() {
        EEPROM.begin(this->EEPROM_SIZE);
    }

    /**
     * Load each ID from EEPROM if they have been set.
     */
    void load() {
        // Load Node ID
        if (EEPROM.read(this->NODE_ID_CHANGE_FLAG) == 1) {
            this->nodeID = this->read(this->NODE_ID);
        }

        // Load Sensor 1 ID
        if (EEPROM.read(this->SENSOR_1_CHANGE_FLAG) == 1) {
            this->sensor1ID = this->read(this->SENSOR_1_ID);
        }
        
        // Load Sensor 2 ID
        if (EEPROM.read(this->SENSOR_2_CHANGE_FLAG) == 1) {
            this->sensor2ID = this->read(this->SENSOR_2_ID);
        }

        // Load Sensor 3 ID
        if (EEPROM.read(this->SENSOR_3_CHANGE_FLAG) == 1) {
            this->sensor3ID = this->read(this->SENSOR_3_ID);
        }
        
        // Load Sensor 4 ID
        if (EEPROM.read(this->SENSOR_4_CHANGE_FLAG) == 1) {
            this->sensor4ID = this->read(this->SENSOR_4_ID);
        }

        this->updateChangeFlag();
    }

    void loadSensorType() {
        // Load Sensor 1 Type
        if (EEPROM.read(this->SENSOR_1_CHANGE_FLAG) == 1) {
            this->sensor1Type = this->read(this->SENSOR_1_TNAME);
        }
        
        // Load Sensor 2 Type
        if (EEPROM.read(this->SENSOR_2_CHANGE_FLAG) == 1) {
            this->sensor2Type = this->read(this->SENSOR_2_TNAME);
        }

        // Load Sensor 3 Type
        if (EEPROM.read(this->SENSOR_3_CHANGE_FLAG) == 1) {
            this->sensor3Type = this->read(this->SENSOR_3_TNAME);
        }
        
        // Load Sensor 4 Type
        if (EEPROM.read(this->SENSOR_4_CHANGE_FLAG) == 1) {
            this->sensor4Type = this->read(this->SENSOR_4_TNAME);
        }

        this->updateChangeFlag();
    }

    void loadChangeFlag() {
        this->updateChangeFlag();
    }

    /**
     * Save the ID to EEPROM and update the change flag.
     * 
     * @param idType The type of ID to save ('N' for Node ID, '1' for Sensor 1 ID, '2' for Sensor 2 ID, '3' for Sensor 3 ID, '4' for Sensor 4 ID).
     * @param id The ID value to save.
     */
    void save(int idType, const char* id, const char* sensorType) {
        Serial.printf("Saving ID: %s\n", id);
        Serial.printf("Saving Sensor Type: %s\n", sensorType);
        Serial.printf("ID Type: %d\n", idType);
        switch (idType) {
            case 0:
                Serial.println("Saving Node ID");
                EEPROM.put(this->NODE_ID, id);
                // Add null terminator
                EEPROM.put(this->NODE_ID + strlen(id), '\0');
                EEPROM.put(this->NODE_ID_CHANGE_FLAG, 1);
                nodeID = atoi(id);
                break;
            case 1:
                Serial.println("Saving Sensor 1 ID");
                EEPROM.put(this->SENSOR_1_ID, id);
                // Add null terminator
                EEPROM.put(this->SENSOR_1_ID + strlen(id), '\0');

                EEPROM.put(this->SENSOR_1_TNAME, sensorType);
                // Add null terminator
                EEPROM.put(this->SENSOR_1_TNAME + strlen(sensorType), '\0');

                EEPROM.put(this->SENSOR_1_CHANGE_FLAG, 1);
                sensor1ID = atoi(id);
                break;
            case 2:
                Serial.println("Saving Sensor 2 ID");
                EEPROM.put(this->SENSOR_2_ID, id);
                // Add null terminator
                EEPROM.put(this->SENSOR_2_ID + strlen(id), '\0');

                EEPROM.put(this->SENSOR_2_TNAME, sensorType);
                // Add null terminator
                EEPROM.put(this->SENSOR_2_TNAME + strlen(sensorType), '\0');

                EEPROM.put(this->SENSOR_2_CHANGE_FLAG, 1);
                nodeID = atoi(id);
                break;
            case 3:
                Serial.println("Saving Sensor 3 ID");
                EEPROM.put(this->SENSOR_3_ID, id);
                // Add null terminator
                EEPROM.put(this->SENSOR_3_ID + strlen(id), '\0');
                EEPROM.write(this->SENSOR_3_CHANGE_FLAG, 1);
                nodeID = atoi(id);
                break;
            case 4:
                Serial.println("Saving Sensor 4 ID");
                EEPROM.put(this->SENSOR_4_ID, id);
                // Add null terminator
                EEPROM.put(this->SENSOR_4_ID + strlen(id), '\0');
                EEPROM.put(this->SENSOR_4_CHANGE_FLAG, 1);
                nodeID = atoi(id);
                break;
        }
        Serial.println("Committing changes to EEPROM");
        EEPROM.commit();
        this->updateChangeFlag();
    }

    /**
     * Returns the ID based on the given idType.
     *
     * @param idType The type of ID to retrieve. Valid values are 'N', '1', '2', '3', '4'.
     * @return The ID corresponding to the given idType.
     */
    int getID(int idType) {
        switch (idType) {
            case 0:
                return this->nodeID;
            case 1:
                return this->sensor1ID;
            case 2:
                return this->sensor2ID;
            case 3:
                return this->sensor3ID;
            case 4:
                return this->sensor4ID;
        }
        return -1;
    }

    int getSensorType(int idType) {
        switch (idType) {
            case 1:
                return this->sensor1Type;
            case 2:
                return this->sensor2Type;
            case 3:
                return this->sensor3Type;
            case 4:
                return this->sensor4Type;
        }
        return -1;
    }

    uint8_t getChangeFlag() {
        return this->changeFlag;
    }

private:
    // Total EEPROM size
    static const int EEPROM_SIZE = 1024;
    // ID storage unit size
    static const int EEPROM_UNIT_SIZE = 100;

    // Node ID storage offsets
    static const int NODE_ID_CHANGE_FLAG = 0;
    static const int NODE_ID = 5;

    // Sensor ID storage offsets
    static const int SENSOR_1_CHANGE_FLAG = 1;
    static const int SENSOR_2_CHANGE_FLAG = 2;
    static const int SENSOR_3_CHANGE_FLAG = 3;
    static const int SENSOR_4_CHANGE_FLAG = 4;
    static const int SENSOR_1_ID = 106;
    static const int SENSOR_2_ID = 206;
    static const int SENSOR_3_ID = 306;
    static const int SENSOR_4_ID = 406;
    static const int SENSOR_1_TNAME = 506;
    static const int SENSOR_2_TNAME = 507;
    static const int SENSOR_3_TNAME = 508;
    static const int SENSOR_4_TNAME = 509;

    // Class ID storage
    int nodeID;
    int sensor1ID;
    int sensor2ID;
    int sensor3ID;
    int sensor4ID;
    uint8_t changeFlag;
    int sensor1Type;
    int sensor2Type;
    int sensor3Type;
    int sensor4Type;

    /**
     * Write the ID to the specified EEPROM offset.
     * 
     * @param id The ID value to write.
     * @param offset The EEPROM offset to write the ID to.
     */
    // void write(String id, int offset) {
    //     for(int i = 0; i < id.length(); i++) {
    //         EEPROM.write(offset + i, id[i]);
    //     }
    //     // Add null terminator
    //     EEPROM.write(NODE_NAME + id.length(), '\0');
    // }

    // Loop through EEPROM and read each byte into string
    int read(int offset) {
        String id = "";
        for(int i = 0; i < this->EEPROM_UNIT_SIZE; i++) {
            char c = EEPROM.read(offset + i);
            // break if null terminator is found
            if(c == '\0') {
                break;
            }
            id += c;
        }
        return id.toInt();
    }

    void updateChangeFlag() {
        this->changeFlag = 0;

        if(this->read(this->NODE_ID_CHANGE_FLAG) == 1){
            this->changeFlag |= 1 << 0;
        }

        if(this->read(this->SENSOR_1_CHANGE_FLAG) == 1) {
            this->changeFlag |= 1 << 1;
        }

        if(this->read(this->SENSOR_2_CHANGE_FLAG) == 1) {
            this->changeFlag |= 1 << 2;
        }

        if(this->read(this->SENSOR_3_CHANGE_FLAG) == 1) {
            this->changeFlag |= 1 << 3;
        }

        if(this->read(this->SENSOR_4_CHANGE_FLAG) == 1) {
            this->changeFlag |= 1 << 4;
        }
    }

};