#include <Arduino.h>
#include <Wire.h>
#include <ArdusatSDK.h>

/*-----------------------------------------------------------------------------
    Setup Software Serial to allow for both RF communication and USB communication
      RX is digital pin 8 (connect to TX/DOUT of RF Device)
      TX is digital pin 9 (connect to RX/DIN of RF Device)
  -----------------------------------------------------------------------------*/
ArdusatSerial serialConnection(SERIAL_MODE_HARDWARE_AND_SOFTWARE, 8, 9);

/*-----------------------------------------------------------------------------
    Constant Definitions
  -----------------------------------------------------------------------------*/
const int READ_INTERVAL = 0; // interval, in ms, to wait between readings

class DustProfiler
{
  private:
    Acceleration accel;
    TemperatureTMP ambient;
    RGBLightTCS rgb;
    Luminosity lum;
    float densityX;
    float densityY;
    float densityZ;
    float dragCoefficient; //google it
    float mass; //measure from cubesat
    float orbitalVelocity; //google speed of cubesat in orbit
    const float crossSectionalAreaX = 10.5; //measure from cubesat
    const float crossSectionalAreaY = 9.5;
    const float crossSectionalAreaZ = 9.5;
    const float dayTime = 88642000; //88642000

    void calcDensityX()
    {
      accel.read();
      densityX = (( 2 * mass * accel.x) / ((orbitalVelocity * orbitalVelocity) * dragCoefficient * crossSectionalAreaX));
    }

    void calcDensityY()
    {
      accel.read();
      densityY = (( 2 * mass * accel.y) / ((orbitalVelocity * orbitalVelocity) * dragCoefficient * crossSectionalAreaY));
    }

    void calcDensityZ()
    {
      accel.read();
      densityZ = (( 2 * mass * accel.z) / ((orbitalVelocity * orbitalVelocity) * dragCoefficient * crossSectionalAreaZ));
    }

    float getDensityX()
    {
      calcDensityX();
      return densityX;
    }

    float getDensityY()
    {
      calcDensityY();
      return densityY;
    }

    float getDensityZ()
    {
      calcDensityZ();
      return densityZ;
    }

    float getLuminosity()
    {
      lum.read();
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
      ambient.read();
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

    float getColorRed()
    {
      rgb.read();
      return rgb.red;
    }

    float getColorGreen()
    {
      rgb.read();
      return rgb.green;
    }

    float getColorBlue()
    {
      rgb.read();
      return rgb.blue;
    }

    float getDustColorDay()
    {
      if (getColorRed() > 255 && getColorGreen() < 100 && getColorBlue() < 50)
        return 10;
      else
        return 0;
    }

    float getDustColorNight()
    {
      if (getColorRed() > 155 && getColorGreen() < 100 && getColorBlue() < 50)
        return 10;
      else
        return 0;
    }

  public:
    DustProfiler()
    {
      densityX = 0;
      densityY = 0;
      densityZ = 0;
      mass = 1;
      orbitalVelocity = 5.5;
      dragCoefficient = 2.2;
      accel.begin();
      ambient.begin();
      rgb.begin();
      lum.begin();
    }

    void sendToEhub()
    {
      serialConnection.println(accel.readToJSON("accelerometerx"));
      serialConnection.println(accel.readToJSON("accelerometery"));
      serialConnection.println(accel.readToJSON("accelerometerz"));
      serialConnection.println(lum.readToJSON("luminosity"));
      serialConnection.println(rgb.readToJSON("rgbred"));
      serialConnection.println(rgb.readToJSON("rgbgreen"));
      serialConnection.println(rgb.readToJSON("rgbblue"));
      serialConnection.println(ambient.readToJSON("ambientTemp"));

      if (millis() <= dayTime) //dayTime < 88642000
      {
        serialConnection.println(valuesToJSON("DustTemperatureDay", "Storm=10/Calm=0/OFF=-999", getDustTempDay()));
        serialConnection.println(valuesToJSON("DustLuminosityDay", "Storm=10/Calm=0/OFF=-999", getDustCoverDay()));
        serialConnection.println(valuesToJSON("DustColorDay", "Storm=10/Calm=0/OFF=-999", getDustColorDay()));
        serialConnection.println(valuesToJSON("DensityXDay", "kg/m^3", getDensityX()));
        serialConnection.println(valuesToJSON("DensityYDay", "kg/m^3", getDensityY()));
        serialConnection.println(valuesToJSON("DensityZDay", "kg/m^3", getDensityZ()));
        serialConnection.println(valuesToJSON("DustTemperatureDay", "Storm=10/Calm=0/OFF=-999", -999));
        serialConnection.println(valuesToJSON("DustLuminosityDay", "Storm=10/Calm=0/OFF=-999", -999));
        serialConnection.println(valuesToJSON("DustColorNight", "Storm=10/Calm=0/OFF=-999", -999));
        serialConnection.println(valuesToJSON("DensityXNight", "kg/m^3", -999));
        serialConnection.println(valuesToJSON("DensityYNight", "kg/m^3", -999));
        serialConnection.println(valuesToJSON("DensityZNight", "kg/m^3", -999));
      }
      else if (millis() > dayTime) //nightTime > 88642000
      {
        serialConnection.println(valuesToJSON("DustTemperatureNight", "Storm=10/Calm=0/OFF=-999", getDustTempNight()));
        serialConnection.println(valuesToJSON("DustLuminosityNight", "Storm=10/Calm=0/OFF=-999", getDustCoverNight()));
        serialConnection.println(valuesToJSON("DustColorNight", "Storm=10/Calm=0/OFF=-999", getDustColorNight()));
        serialConnection.println(valuesToJSON("DensityXNight", "kg/m^3", getDensityX()));
        serialConnection.println(valuesToJSON("DensityYNight", "kg/m^3", getDensityX()));
        serialConnection.println(valuesToJSON("DensityZNight", "kg/m^3", getDensityX()));
        serialConnection.println(valuesToJSON("DustTemperatureDay", "Storm=10/Calm=0/OFF=-999", -999));
        serialConnection.println(valuesToJSON("DustLuminosityDay", "Storm=10/Calm=0/OFF=-999", -999));
        serialConnection.println(valuesToJSON("DustColorDay", "Storm=10/Calm=0/OFF=-999", -999));
        serialConnection.println(valuesToJSON("DensityXDay", "kg/m^3", -999));
        serialConnection.println(valuesToJSON("DensityYDay", "kg/m^3", -999));
        serialConnection.println(valuesToJSON("DensityZDay", "kg/m^3", -999));
      }
    }
};

DustProfiler arduSat1;

void setup(void)
{
  serialConnection.begin(9600);

  /* We're ready to go! */
  serialConnection.println("");
}

void loop(void)
{
  arduSat1.sendToEhub();
  delay(READ_INTERVAL);
}
