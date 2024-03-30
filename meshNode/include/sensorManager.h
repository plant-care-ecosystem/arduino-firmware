#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Adafruit_Sensor.h>
#include <DHT.h> 
#include <map>
#include <ArduinoJson.h>
#include <eepromManager.h>

// Sensor type enum
enum UserSensorType {
    DHT_SENSOR,
    SOIL_MOISTURE_SENSOR,
    LIGHT_SENSOR,
    RAIN_SENSOR,
    UNKNOWN_SENSOR
};

// // Map the power MUX to the signal MUX
// std::map<int, int> locationToSignal = {
//     {1, 2},
//     {2, 1},
//     {3, 0},
//     {4, 3}
// };

// Map the power MUX to the signal MUX
std::map<int, int> locationToPower = {
    {1, 0},
    {2, 1},
    {3, 2},
    {4, 3}
};

// Map the power resistor MUX to the signal MUX
std::map<int, int> locationToResistor = {
    {1, 4},
    {2, 5},
    {3, 6},
    {4, 7}
};

// Define pins
#define MASTER_POWER 10

// Define signal MUX control signal
// #define DOUT_SIG_A 11  // MSB
// #define DOUT_SIG_B 10   //
// #define DOUT_SIG_C 8   // LSB
// #define AIN 2         // Analog / Digital IN
#define AIN1 4 // Analog / Digital IN sensor 1
#define AIN2 5
#define AIN3 6
#define AIN4 0
#define RIN 1

// Define power MUX control signal
// Control which sensor is getting power
// Control which sensor resistor is getting power for recognition
#define DOUT_PWR_A 3  // MSB
#define DOUT_PWR_B 2  //
#define DOUT_PWR_C 11  // LSB
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

DHT dhtSensors[] = {
    DHT(AIN1, DHT11), // Initialize DHT sensor
    DHT(AIN2, DHT11), // Initialize DHT sensor
    DHT(AIN3, DHT11), // Initialize DHT sensor
    DHT(AIN4, DHT11) // Initialize DHT sensor
};

int analogPins[] = {AIN1, AIN2, AIN3, AIN4};

class SensorManager {
public:

    SensorManager() {
        pinMode(AIN1, INPUT);
        pinMode(AIN2, INPUT);
        pinMode(AIN3, INPUT);
        pinMode(AIN4, INPUT);
        pinMode(RIN, INPUT);
        // pinMode(DOUT_SIG_A, OUTPUT);
        // pinMode(DOUT_SIG_B, OUTPUT);
        // pinMode(DOUT_SIG_C, OUTPUT);
        pinMode(DOUT_PWR_A, OUTPUT);
        pinMode(DOUT_PWR_B, OUTPUT);
        pinMode(DOUT_PWR_C, OUTPUT);
        pinMode(MASTER_POWER, OUTPUT);

        dhtSensors[0].begin();
        dhtSensors[1].begin();
        dhtSensors[2].begin();
        dhtSensors[3].begin();

        // Turn on master power
        digitalWrite(MASTER_POWER, HIGH);
        Serial.print("Master power on\n");
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

        // TODO: SET POWER MUX IN TASK!!!!
        // this->set_pwr_mux(locationToPower[sensorNumber]);
        // delay(1000);
        Serial.printf("Reading sensor %d data\n", sensorNumber);
        sensorData = this->readSensorData(this->returnSensorType(sensorNumber), sensorNumber);
        // switch(sensorNumber) {
        //     case 1:
        //         Serial.println("Reading sensor 1 data");
        //         sensorData = this->readSensorData(sensor1Type, sensorNumber);
        //         break;
        //     case 2:
        //         Serial.println("Reading sensor 2 data");
        //         sensorData = this->readSensorData(sensor2Type, sensorNumber);
        //         break;
        //     case 3:
        //         Serial.println("Reading sensor 3 data");
        //         sensorData = this->readSensorData(sensor3Type, sensorNumber);
        //         break;
        //     case 4:
        //         Serial.println("Reading sensor 4 data");
        //         sensorData = this->readSensorData(sensor4Type, sensorNumber);
        //         break;
        // }
        return sensorData;
    }

    void getSingleSensorType(int sensorNumber) {
        int rinValue = 0;
        // Serial.printf("Getting sensor type for sensor %d\n", sensorNumber);
        // Run power to resistor of sensor 1
        // Serial.printf("Setting power to sensor %d\n", locationToResistor[sensorNumber]);

        // TODO: SET POWER MUX IN TASK!!!!

        // this->set_pwr_mux(locationToResistor[sensorNumber]);
        // delay(1000);
        // Read the analog value on the RIN Pin
        for(int i = 0; i < 500; i++) {
            rinValue += analogRead(RIN);
        }
        rinValue = rinValue / 500;
        // Serial.printf("RIN Value: %d\n", rinValue);
        this->readResistorType(rinValue, sensorNumber);
    }

    int returnSensorType(int sensorNumber) {
        // this->getSingleSensorType(sensorNumber);
        switch(sensorNumber) {
            case 1:
                return sensor1Type;
            case 2:
                return sensor2Type;
            case 3:
                return sensor3Type;
            case 4:
                return sensor4Type;
        }
    }

    void readResistorType(int analogReadValue, int portNumber) {
        // Check the analog read value and set the sensor type
        if(analogReadValue >= dhtResistorMin && analogReadValue <= dhtResistorMax) {
            // Set the sensor type to DHT
            // Serial.printf("Setting sensor on port %d to DHT\n", portNumber);
            this->setSensorType(portNumber, UserSensorType::DHT_SENSOR);
        } else if(analogReadValue >= soilMoistureResistorMin && analogReadValue <= soilMoistureResistorMax) {
            // Set the sensor type to Soil Moisture
            // Serial.printf("Setting sensor on port %d to Soil Moisture\n", portNumber);
            this->setSensorType(portNumber, UserSensorType::SOIL_MOISTURE_SENSOR);
        } else if(analogReadValue >= lightResistorMin && analogReadValue <= lightResistorMax) {
            // Set the sensor type to Light
            // Serial.printf("Setting sensor on port %d to Light\n", portNumber);
            this->setSensorType(portNumber, UserSensorType::LIGHT_SENSOR);
        } else if(analogReadValue >= rainResistorMin && analogReadValue <= rainResistorMax) {
            // Set the sensor type to Rain
            // Serial.printf("Setting sensor on port %d to Rain\n", portNumber);
            this->setSensorType(portNumber, UserSensorType::RAIN_SENSOR);
        }
        else {
            // Set the sensor type to Unknown
            // Serial.println("Setting sensor type to Unknown");
            this->setSensorType(portNumber, UserSensorType::UNKNOWN_SENSOR);
        }
        
    }

    void setSensorType(int sensorNumber, UserSensorType sensorType) {
        switch(sensorNumber) {
            case 1:
                sensor1Type = sensorType;
                break;
            case 2:
                sensor2Type = sensorType;
                break;
            case 3:
                sensor3Type = sensorType;
                break;
            case 4:
                sensor4Type = sensorType;
                break;
        }
    }

    // // Funciton to check the sensor type and based on sensor type read the data
    // StaticJsonDocument<200> readSensorData(int sensorType, int sensorNumber) {
    //     StaticJsonDocument<200> sensorData;
    //     switch(sensorType) {
    //         case UserSensorType::DHT_SENSOR:
    //             // Read DHT sensor data
    //             dhtSensors[sensorNumber - 1].begin();
    //             temperature = dhtSensors[sensorNumber - 1].readTemperature();
    //             humidity = dhtSensors[sensorNumber - 1].readHumidity();
    //             // temperature = dht.readTemperature();
    //             // humidity = dht.readHumidity();
    //             sensorData["temperature"] = temperature;
    //             sensorData["humidity"] = humidity;
    //             break;
    //         case UserSensorType::SOIL_MOISTURE_SENSOR:
    //             // Read Soil Moisture sensor data
    //             soilMoisture = analogRead(analogPins[sensorNumber - 1]);
    //             sensorData["soilMoisture"] = soilMoisture;
    //             break;
    //         case UserSensorType::LIGHT_SENSOR:
    //             // Read Light sensor data
    //             light = analogRead(analogPins[sensorNumber - 1]);
    //             sensorData["light"] = light;
    //             break;
    //         case UserSensorType::RAIN_SENSOR:
    //             // Read Rain sensor data
    //             rain = analogRead(analogPins[sensorNumber - 1]);
    //             sensorData["rain"] = rain;
    //             break;
    //     }
    //     return sensorData;
    // }

    // Function to check the sensor type and based on sensor type read the data
StaticJsonDocument<200> readSensorData(int sensorType, int sensorNumber, int numReadings = 500) {
    StaticJsonDocument<200> sensorData;
    // Serial.printf("Reading sensor %d data\n", sensorNumber);
    // Serial.printf("Sensor Type: %d\n", sensorType);
    // Serial.printf("Num Readings: %d\n", numReadings);
    switch(sensorType) {
        case UserSensorType::DHT_SENSOR:
            // Read DHT sensor data
            dhtSensors[sensorNumber - 1].begin();
            temperature = 0.0;
            humidity = 0.0;
            for(int i = 0; i < numReadings; i++) {
                temperature += dhtSensors[sensorNumber - 1].readTemperature();
                humidity += dhtSensors[sensorNumber - 1].readHumidity();
            }
            sensorData["temperature"] = temperature / numReadings;
            sensorData["humidity"] = humidity / numReadings;
            break;
        case UserSensorType::SOIL_MOISTURE_SENSOR:
            // Read Soil Moisture sensor data
            soilMoisture = 0;
            for(int i = 0; i < numReadings; i++) {
                soilMoisture += analogRead(analogPins[sensorNumber - 1]);
            }
            sensorData["soilMoisture"] = soilMoisture / numReadings;
            break;
        case UserSensorType::LIGHT_SENSOR:
            // Read Light sensor data
            light = 0;
            for(int i = 0; i < numReadings; i++) {
                light += analogRead(analogPins[sensorNumber - 1]);
                delay(2000);  // Wait a bit between readings
            }
            sensorData["light"] = light / numReadings;
            break;
        case UserSensorType::RAIN_SENSOR:
            // Read Rain sensor data
            rain = 0;
            for(int i = 0; i < numReadings; i++) {
                rain += analogRead(analogPins[sensorNumber - 1]);
                delay(2000);  // Wait a bit between readings
            }
            sensorData["rain"] = rain / numReadings;
            break;
    }
    return sensorData;
}

    // // Function to set signal MUX control bits
    // void set_sig_mux(int pin) {
    //     pin = max(0, min(7, pin)); // Make sure the input is less than 7 and greater than 0
    //     digitalWrite(DOUT_SIG_A, (((pin >> 2) & 1) == 1)? HIGH : LOW);
    //     digitalWrite(DOUT_SIG_B, (((pin >> 1) & 1) == 1)? HIGH : LOW);
    //     digitalWrite(DOUT_SIG_C, ((pin & 1) == 1)? HIGH : LOW);
    // }
    // Function to set power MUX control bits
    void set_pwr_mux(int pin) {
        // Serial.printf("Setting power mux to %d\n", pin);
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
        Serial.printf("Sensor Name: %s\n", sensorName.c_str());
        return sensorName;
    }

private:
    int dhtResistorMin = 300;
    int dhtResistorMax = 400;
    int soilMoistureResistorMin = 1050; //500;
    int soilMoistureResistorMax = 1200;//650;
    int lightResistorMin = 850;
    int lightResistorMax = 950;
    int rainResistorMin = 500;//1050;
    int rainResistorMax = 650;//1200;

    // Sensor types
    // UserSensorType sensor1Type;
    // UserSensorType sensor2Type;
    // UserSensorType sensor3Type;
    // UserSensorType sensor4Type;

    float temperature;
    float humidity;
    int soilMoisture;
    int light;
    int rain;

    
};

#endif // SENSOR_MANAGER_H