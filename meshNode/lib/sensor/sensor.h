#ifndef SENSOR_H
#define SENSOR_H

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <memory>

// Enum for sensor types
enum SensorType {
    MESH_DHT11 = DHT11,
    MESH_DHT22 = DHT22
};

// Abstract sensor class
class Sensor {
public:
    virtual ~Sensor() = default;
    virtual void readSensor() = 0;

    virtual float getTemperatureF() const = 0;
    virtual float getTemperatureC() const = 0;
    virtual float getHumidity() const = 0;
};

// DHT sensor class
class DHTSensor : public Sensor {
public:
    DHTSensor(SensorType type, int pin); // Constructor class
    void readSensor() override;

    // Public getters
    float getTemperatureF() const override { return temperature_f; }
    float getTemperatureC() const override { return temperature_c; }
    float getHumidity() const override { return humidity; }
private:
    DHT dht;
    float temperature_f;
    float temperature_c;
    float humidity;
};

// Abstract sensor creator class
class SensorFactory {
public:
    virtual Sensor* createSensor(SensorType type, int pin) = 0;
    virtual ~SensorFactory() = default;
    // static std::unique_ptr<Sensor> createSensor(SensorType type, int port) = 0;
    // virtual ~SensorFactory() = default;
};

class DHTSensorFactory : public SensorFactory {
public:
    Sensor* createSensor(SensorType type, int pin) override;
};

#endif // EXAMPLE_HEADER_H
