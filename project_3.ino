#include <Arduino.h>
#include <Wire.h>
#include <ArdusatSDK.h>

/*-----------------------------------------------------------------------------
    Setup Software Serial to allow for both RF communication and USB communication
      RX is digital pin 8 (connect to TX/DOUT of RF Device)
      TX is digital pin 9 (connect to RX/DIN of RF Device)
  -----------------------------------------------------------------------------*/
ArdusatSerial serialConnection(SERIAL_MODE_HARDWARE_AND_SOFTWARE, 8, 9); // 8TX, 9RX

/*-----------------------------------------------------------------------------
    Constant Definitions
  -----------------------------------------------------------------------------*/

const int READ_INTERVAL = 0; // interval, in ms, to wait between readings

class DustProfiler //DustProfiler class
{
  private:
  
    acceleration_t accel; //acceleration object
    temperature_t ambient; //ambient object
    luminosity_t lum; //luminosity object
    double densityXZ; //calculated by formular
    double densityYZ; //calculated by formular
    float densityXY; //calculated by formular
    float const dragCoefficient = 2; //resistance in space
    float const mass = 1; //measure from cubesat
    float orbitalVelocity = 1; //average speed of cubesat at the top of the lower orbit = 7000m above the surface of mars 3542.8
    const float crossSectionalAreaXY = 0.0096; //measured from cubesat length = 0.098 width = 0.098 height = 0.11
    const float crossSectionalAreaXZ = 0.01078; //measured from cubesat length = 0.098 width = 0.098 height = 0.11
    const float crossSectionalAreaYZ = 0.01078; //measured from cubesat length = 0.098 width = 0.098 height = 0.11
    const unsigned long dayTime = 10000; //88620000

    void calcDensityXZ() //calculates atmospheric density at the XZ plane
    {
      readAcceleration(accel);
      densityXZ = (( 2 * mass * accel.x) / ((orbitalVelocity * orbitalVelocity)));
    }

    void calcDensityYZ() //calculates atmospheric density at the YZ plane
    {
      readAcceleration(accel);
      densityYZ = (( 2 * mass * accel.y) / ((orbitalVelocity * orbitalVelocity)));
    }

    void calcDensityXY() //calculates atmospheric density at the XY plane
    {
      readAcceleration(accel);
      densityXY = (2 * mass * accel.z) / ((orbitalVelocity * orbitalVelocity));
    }

    float getDensityXZ()
    {
      calcDensityXZ();
      return densityXZ;
    }

    float getDensityYZ()
    {
      calcDensityYZ();
      return densityYZ;
    }

    float getDensityXY()
    {
      calcDensityXY();
      return densityXY;
    }

    float getLuminosity()
    {
      readLuminosity(lum);
      return lum.lux;
    }

    float getDustCoverDay()
    {
      if (getLuminosity() < 1000)
      {
        return 10;
      }
      else
        return 0;
    }

    float getDustCoverNight()
    {
      if (getLuminosity() < 200)
      {
        return 10;
      }
      else
        return 0;
    }

    float getTemperature()
    {
      readTemperature(ambient);
      return ambient.t;
    }

    float getDustTempDay()
    {
      if (getTemperature() > 100)
      {
        return 10;
      }
      else
        return 0;
    }

    float getDustTempNight()
    {
      if (getTemperature() > 10)
      {
        return 10;
      }
      else
        return 0;
    }

  public:
  
    void start()
    {
      serialConnection.begin(9600);
      densityXZ = 0; //calculated by formular
      densityYZ = 0; //calculated by formular
      densityXY = 0; //calculated by formular
      if (!beginAccelerationSensor()) {
        serialConnection.println("Can't initialize IMU! Check your wiring.");
      }

      if (!beginTemperatureSensor()) {
        serialConnection.println("Can't initialize temperature sensor! Check your wiring.");
      }

      if (!beginLuminositySensor()) {
        serialConnection.println("Can't initialize luminosity sensor! Check your wiring.");
      }
    }

    void sendToEhub()
    {
      readAcceleration(accel);
      readLuminosity(lum);
      readTemperature(ambient);
      calcDensityXY();

      serialConnection.println(valueToJSON("accelerometerx", "m/s^2", accel.x));
      serialConnection.println(valueToJSON("accelerometery", "m/s^2", accel.y));
      serialConnection.println(valueToJSON("accelerometerz", "m/s^2", accel.z));
      serialConnection.println(luminosityToJSON("luminosity", lum));
      serialConnection.println(temperatureToJSON("ambientTemp", ambient));

      if (millis() % (dayTime) <= dayTime / 2) //dayTime < 88642000
      {
        serialConnection.println(valueToJSON("DensityXZDay", "kg/m^3", getDensityXZ()));
        serialConnection.println(valueToJSON("DensityYZDay", "kg/m^3", getDensityYZ()));
        serialConnection.println(valueToJSON("DensityXYDay", "kg/m^3", getDensityXY()));
        serialConnection.println(valueToJSON("DustTemperatureDay", "Storm=10/Calm=0/OFF=-999", getDustTempDay()));
        serialConnection.println(valueToJSON("DustLuminosityDay", "Storm=10/Calm=0/OFF=-999", getDustCoverDay()));
        serialConnection.println(valueToJSON("DustTemperatureNight", "Storm=10/Calm=0/OFF=-999", -999));
        serialConnection.println(valueToJSON("DustLuminosityNight", "Storm=10/Calm=0/OFF=-999", -999));
        serialConnection.println(valueToJSON("DensityXZNight", "kg/m^3", -999));
        serialConnection.println(valueToJSON("DensityYZNight", "kg/m^3", -999));
        serialConnection.println(valueToJSON("DensityXYNight", "kg/m^3", -999));
      }
      else if ((millis() % (dayTime)) > dayTime / 2) //nightTime > 88642000
      {
        serialConnection.println(valueToJSON("DensityXZNight", "kg/m^3", getDensityXZ()));
        serialConnection.println(valueToJSON("DensityYZNight", "kg/m^3", getDensityYZ()));
        serialConnection.println(valueToJSON("DensityXYNight", "kg/m^3", getDensityXY()));
        serialConnection.println(valueToJSON("DustTemperatureNight", "Storm=10/Calm=0/OFF=-999", getDustTempNight()));
        serialConnection.println(valueToJSON("DustLuminosityNight", "Storm=10/Calm=0/OFF=-999", getDustCoverNight()));
        serialConnection.println(valueToJSON("DustTemperatureDay", "Storm=10/Calm=0/OFF=-999", -999));
        serialConnection.println(valueToJSON("DustLuminosityDay", "Storm=10/Calm=0/OFF=-999", -999));
        serialConnection.println(valueToJSON("DensityXZDay", "kg/m^3", -999));
        serialConnection.println(valueToJSON("DensityYZDay", "kg/m^3", -999));
        serialConnection.println(valueToJSON("DensityXYDay", "kg/m^3", -999));
      }
    }
};

DustProfiler arduSat1;

void setup(void)
{
  arduSat1.start();
  /* We're ready to go! */
  serialConnection.println("");
}

void loop(void)
{
  arduSat1.sendToEhub();
  delay(READ_INTERVAL);
}
