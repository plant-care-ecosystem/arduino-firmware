#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Adafruit_Sensor.h>
#include <DHT.h> 
#include <map>
#include <ArduinoJson.h>

// Sensor type enum
enum UserSensorType {
    DHT_SENSOR,
    SOIL_MOISTURE_SENSOR,
    LIGHT_SENSOR,
    RAIN_SENSOR,
    UNKNOWN_SENSOR
};

// Map the power MUX to the signal MUX
std::map<int, int> locationToSignal = {
    {1, 2},
    {2, 1},
    {3, 0},
    {4, 3}
};

// Map the power MUX to the signal MUX
std::map<int, int> locationToPower = {
    {1, 2},
    {2, 0},
    {3, 1},
    {4, 3}
};

// Map the power resistor MUX to the signal MUX
std::map<int, int> locationToResistor = {
    {1, 6},
    {2, 4},
    {3, 7},
    {4, 5}
};

// Define pins
#define MASTER_POWER 15

// Define signal MUX control signal
#define DOUT_SIG_A 11  // MSB
#define DOUT_SIG_B 10   //
#define DOUT_SIG_C 8   // LSB
#define AIN 2         // Analog / Digital IN
#define RIN 3

// Define power MUX control signal
// Control which sensor is getting power
// Control which sensor resistor is getting power for recognition
#define DOUT_PWR_A 23  // MSB
#define DOUT_PWR_B 22  //
#define DOUT_PWR_C 21  // LSB
// #define DOUT_PWR     // Transistor to change 3.3V to 5V

// Other definitions
#define AIN_MAX 4095     // Max value of analog in
#define AIN_VOLTAGE 3.3  // Max expected voltage
// TODO: MIGHT WANT TO READ THIS
// V
#define PU_VOLTAGE 3.3            // Pull up voltage
#define PU_RESISTANCE 9.1         // kOhm
#define RESISTANCE_THRESHOLD 0.2  // kOhm
#define NUM_SENSORS 4

DHT dht(AIN, DHT11); // Initialize DHT sensor

class SensorManager {
public:

    SensorManager() {
        pinMode(AIN, INPUT);
        pinMode(RIN, INPUT);
        pinMode(DOUT_SIG_A, OUTPUT);
        pinMode(DOUT_SIG_B, OUTPUT);
        pinMode(DOUT_SIG_C, OUTPUT);
        pinMode(DOUT_PWR_A, OUTPUT);
        pinMode(DOUT_PWR_B, OUTPUT);
        pinMode(DOUT_PWR_C, OUTPUT);
    }
    
    // void getSensorTypes() {
    //     int rinValue;
    //     // Go through sensor 1 - 4 and check the sensor type
        
    //     // Run power to resistor of sensor 1
    //     this->set_pwr_mux(powerResistorToSensor[1]);
    //     delay(1000);
    //     // Read the analog value on the RIN Pin
    //     rinValue = analogRead(RIN);
    //     this->readResistorType(rinValue, 1);

    //     // Run power to resistor of sensor 2
    //     this->set_pwr_mux(powerResistorToSensor[2]);
    //     delay(1000);
    //     // Read the analog value on the RIN Pin
    //     rinValue = analogRead(RIN);
    //     this->readResistorType(rinValue, 2);

    //     // Run power to resistor of sensor 3
    //     this->set_pwr_mux(powerResistorToSensor[3]);
    //     delay(1000);
    //     // Read the analog value on the RIN Pin
    //     rinValue = analogRead(RIN);
    //     this->readResistorType(rinValue, 3);

    //     // Run power to resistor of sensor 4
    //     this->set_pwr_mux(powerResistorToSensor[4]);
    //     delay(1000);
    //     // Read the analog value on the RIN Pin
    //     rinValue = analogRead(RIN);
    //     this->readResistorType(rinValue, 4);
    // }

    StaticJsonDocument<200> getSensorData(int sensorNumber) {
        StaticJsonDocument<200> sensorData;
        // Go through sensor 1 - 4 and get the data
        // Get sensor1 data
        this->getSingleSensorType(sensorNumber);
        this->set_pwr_mux(locationToPower[sensorNumber]);
        delay(1000);
        this->set_sig_mux(locationToSignal[sensorNumber]);
        switch(sensorNumber) {
            case 1:
                Serial.println("Reading sensor 1 data");
                sensorData = this->readSensorData(this->sensor1Type);
                break;
            case 2:
                Serial.println("Reading sensor 2 data");
                sensorData = this->readSensorData(this->sensor2Type);
                break;
            case 3:
                Serial.println("Reading sensor 3 data");
                sensorData = this->readSensorData(this->sensor3Type);
                break;
            case 4:
                Serial.println("Reading sensor 4 data");
                sensorData = this->readSensorData(this->sensor4Type);
                break;
        }
        return sensorData;
    }

    void getSingleSensorType(int sensorNumber) {
        int rinValue = 0;
        // Run power to resistor of sensor 1
        // Serial.printf("Setting power to sensor %d\n", locationToResistor[sensorNumber]);
        this->set_pwr_mux(locationToResistor[sensorNumber]);
        delay(1000);
        // Read the analog value on the RIN Pin
        for(int i = 0; i < 500; i++) {
            rinValue += analogRead(RIN);
        }
        rinValue = rinValue / 500;
        // Serial.printf("RIN Value: %d\n", rinValue);
        this->readResistorType(rinValue, sensorNumber);
    }

    int returnSensorType(int sensorNumber) {
        this->getSingleSensorType(sensorNumber);
        switch(sensorNumber) {
            case 1:
                return this->sensor1Type;
            case 2:
                return this->sensor2Type;
            case 3:
                return this->sensor3Type;
            case 4:
                return this->sensor4Type;
        }
    }

    void readResistorType(int analogReadValue, int portNumber) {
        // Check the analog read value and set the sensor type
        if(analogReadValue >= dhtResistorMin && analogReadValue <= dhtResistorMax) {
            // Set the sensor type to DHT
            Serial.printf("Setting sensor on port %d to DHT\n", portNumber);
            this->setSensorType(portNumber, UserSensorType::DHT_SENSOR);
        } else if(analogReadValue >= soilMoistureResistorMin && analogReadValue <= soilMoistureResistorMax) {
            // Set the sensor type to Soil Moisture
            Serial.printf("Setting sensor on port %d to Soil Moisture\n", portNumber);
            this->setSensorType(portNumber, UserSensorType::SOIL_MOISTURE_SENSOR);
        } else if(analogReadValue >= lightResistorMin && analogReadValue <= lightResistorMax) {
            // Set the sensor type to Light
            Serial.printf("Setting sensor on port %d to Light\n", portNumber);
            this->setSensorType(portNumber, UserSensorType::LIGHT_SENSOR);
        } else if(analogReadValue >= rainResistorMin && analogReadValue <= rainResistorMax) {
            // Set the sensor type to Rain
            Serial.printf("Setting sensor on port %d to Rain\n", portNumber);
            this->setSensorType(portNumber, UserSensorType::RAIN_SENSOR);
        }
        else {
            // Set the sensor type to Unknown
            Serial.println("Setting sensor type to Unknown");
            this->setSensorType(portNumber, UserSensorType::UNKNOWN_SENSOR);
        }
        
    }

    void setSensorType(int sensorNumber, UserSensorType sensorType) {
        switch(sensorNumber) {
            case 1:
                this->sensor1Type = sensorType;
                break;
            case 2:
                this->sensor2Type = sensorType;
                break;
            case 3:
                this->sensor3Type = sensorType;
                break;
            case 4:
                this->sensor4Type = sensorType;
                break;
        }
    }

    // Funciton to check the sensor type and based on sensor type read the data
    StaticJsonDocument<200> readSensorData(UserSensorType sensorType) {
        StaticJsonDocument<200> sensorData;
        switch(sensorType) {
            case UserSensorType::DHT_SENSOR:
                // Read DHT sensor data
                temperature = dht.readTemperature();
                humidity = dht.readHumidity();
                sensorData["temperature"] = temperature;
                sensorData["humidity"] = humidity;
                break;
            case UserSensorType::SOIL_MOISTURE_SENSOR:
                // Read Soil Moisture sensor data
                soilMoisture = analogRead(AIN);
                sensorData["soilMoisture"] = soilMoisture;
                break;
            case UserSensorType::LIGHT_SENSOR:
                // Read Light sensor data
                light = analogRead(AIN);
                sensorData["light"] = light;
                break;
            case UserSensorType::RAIN_SENSOR:
                // Read Rain sensor data
                rain = analogRead(AIN);
                sensorData["rain"] = rain;
                break;
        }
        return sensorData;
    }

    // Function to set signal MUX control bits
    void set_sig_mux(int pin) {
        pin = max(0, min(7, pin)); // Make sure the input is less than 7 and greater than 0
        digitalWrite(DOUT_SIG_A, (((pin >> 2) & 1) == 1)? HIGH : LOW);
        digitalWrite(DOUT_SIG_B, (((pin >> 1) & 1) == 1)? HIGH : LOW);
        digitalWrite(DOUT_SIG_C, ((pin & 1) == 1)? HIGH : LOW);
    }
    // Function to set power MUX control bits
    void set_pwr_mux(int pin) {
        pin = max(0, min(7, pin)); // Make sure the input is less than 7 and greater than 0
        digitalWrite(DOUT_PWR_A, (((pin >> 2) & 1) == 1)? HIGH : LOW);
        digitalWrite(DOUT_PWR_B, (((pin >> 1) & 1) == 1)? HIGH : LOW);
        digitalWrite(DOUT_PWR_C, ((pin & 1) == 1)? HIGH : LOW);
    }

    String generateSensorName(int sensorType, int sensorNumber) {
        String sensorName = "";
        switch(sensorType) {
            case UserSensorType::DHT_SENSOR:
                sensorName = "DHT";
                break;
            case UserSensorType::SOIL_MOISTURE_SENSOR:
                sensorName = "Soil Moisture";
                break;
            case UserSensorType::LIGHT_SENSOR:
                sensorName = "Light";
                break;
            case UserSensorType::RAIN_SENSOR:
                sensorName = "Rain";
                break;
        }
        sensorName += "-";
        sensorName += sensorNumber;
        return sensorName;
    }

private:
    int dhtResistorMin = 300;
    int dhtResistorMax = 400;
    int soilMoistureResistorMin = 500;
    int soilMoistureResistorMax = 650;
    int lightResistorMin = 850;
    int lightResistorMax = 950;
    int rainResistorMin = 1050;
    int rainResistorMax = 1200;

    // Sensor types
    UserSensorType sensor1Type;
    UserSensorType sensor2Type;
    UserSensorType sensor3Type;
    UserSensorType sensor4Type;

    float temperature;
    float humidity;
    int soilMoisture;
    int light;
    int rain;

    
};

#endif // SENSOR_MANAGER_H