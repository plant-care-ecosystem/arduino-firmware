#include <EEPROM.h>

class EEPROMManager {
public:
    /**
     * Constructor for the EEPROMManager class.
     * Initializes the EEPROM with 512 bytes.
     */
    EEPROMManager() {
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

    /**
     * Save the ID to EEPROM and update the change flag.
     * 
     * @param idType The type of ID to save ('N' for Node ID, '1' for Sensor 1 ID, '2' for Sensor 2 ID, '3' for Sensor 3 ID, '4' for Sensor 4 ID).
     * @param id The ID value to save.
     */
    void save(char idType, char* id) {
        switch (idType) {
            case 'N':
                EEPROM.put(this->NODE_ID, id);
                // Add null terminator
                EEPROM.put(this->NODE_ID + strlen(id), '\0');
                EEPROM.put(this->NODE_ID_CHANGE_FLAG, 1);
                nodeID = atoi(id);
                break;
            case '1':
                EEPROM.put(this->SENSOR_1_ID, id);
                // Add null terminator
                EEPROM.put(this->SENSOR_1_ID + strlen(id), '\0');
                EEPROM.put(this->SENSOR_1_CHANGE_FLAG, 1);
                sensor1ID = atoi(id);
                break;
            case '2':
                EEPROM.put(this->SENSOR_2_ID, id);
                // Add null terminator
                EEPROM.put(this->SENSOR_2_ID + strlen(id), '\0');
                EEPROM.put(this->SENSOR_2_CHANGE_FLAG, 1);
                nodeID = atoi(id);
                break;
            case '3':
                EEPROM.put(this->SENSOR_3_ID, id);
                // Add null terminator
                EEPROM.put(this->SENSOR_3_ID + strlen(id), '\0');
                EEPROM.write(this->SENSOR_3_CHANGE_FLAG, 1);
                nodeID = atoi(id);
                break;
            case '4':
                EEPROM.put(this->SENSOR_4_ID, id);
                // Add null terminator
                EEPROM.put(this->SENSOR_4_ID + strlen(id), '\0');
                EEPROM.put(this->SENSOR_4_CHANGE_FLAG, 1);
                nodeID = atoi(id);
                break;
        }
        EEPROM.commit();
        this->updateChangeFlag();
    }

    /**
     * Returns the ID based on the given idType.
     *
     * @param idType The type of ID to retrieve. Valid values are 'N', '1', '2', '3', '4'.
     * @return The ID corresponding to the given idType.
     */
    int getID(char idType) {
        switch (idType) {
            case 'N':
                return this->nodeID;
            case '1':
                return this->sensor1ID;
            case '2':
                return this->sensor2ID;
            case '3':
                return this->sensor3ID;
            case '4':
                return this->sensor4ID;
        }
    }

    uint8_t getChangeFlag() {
        return this->changeFlag;
    }

private:
    // Total EEPROM size
    static const int EEPROM_SIZE = 512;
    // ID storage unit size
    static const int EEPROM_UNIT_SIZE = 102;

    // Node ID storage offsets
    static const int NODE_ID_CHANGE_FLAG = 0;
    static const int NODE_ID = 5;

    // Sensor ID storage offsets
    static const int SENSOR_1_CHANGE_FLAG = 1;
    static const int SENSOR_2_CHANGE_FLAG = 2;
    static const int SENSOR_3_CHANGE_FLAG = 3;
    static const int SENSOR_4_CHANGE_FLAG = 4;
    static const int SENSOR_1_ID = 103;
    static const int SENSOR_2_ID = 205;
    static const int SENSOR_3_ID = 307;
    static const int SENSOR_4_ID = 409;

    // Class ID storage
    int nodeID;
    int sensor1ID;
    int sensor2ID;
    int sensor3ID;
    int sensor4ID;
    uint8_t changeFlag;

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

        if(EEPROM.read(this->NODE_ID_CHANGE_FLAG) == 1) {
            this->changeFlag |= 1 << 0;
        }

        if(EEPROM.read(this->SENSOR_1_CHANGE_FLAG) == 1) {
            this->changeFlag |= 1 << 1;
        }

        if(EEPROM.read(this->SENSOR_2_CHANGE_FLAG) == 1) {
            this->changeFlag |= 1 << 2;
        }

        if(EEPROM.read(this->SENSOR_3_CHANGE_FLAG) == 1) {
            this->changeFlag |= 1 << 3;
        }

        if(EEPROM.read(this->SENSOR_4_CHANGE_FLAG) == 1) {
            this->changeFlag |= 1 << 4;
        }
    }

};