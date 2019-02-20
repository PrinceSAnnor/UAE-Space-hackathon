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
const int READ_INTERVAL = 1000; // interval, in ms, to wait between readings

class DustProfiler //DustProfiler class
{
  private:
    acceleration_t accel; //acceleration object
    temperature_t ambient; //ambient object
    luminosity_t lum; //luminosity object
    float densityXZ; //calculated by formular
    float densityYZ; //calculated by formular
    float densityXY; //calculated by formular
    float const dragCoefficient = 2; //resistance in space
    float const mass = 1; //measure from cubesat
    float orbitalVelocity = 3542.8; //average speed of cubesat at the top of the lower orbit = 7000m above the surface of mars
    const float crossSectionalAreaXY = 0.0096; //measured from cubesat length = 0.098 width = 0.098 height = 0.11
    const float crossSectionalAreaXZ = 0.01078; //measured from cubesat length = 0.098 width = 0.098 height = 0.11
    const float crossSectionalAreaYZ = 0.01078; //measured from cubesat length = 0.098 width = 0.098 height = 0.11
    const unsigned long dayTime = 10000; //88620000

    void calcDensityXZ() //calculates atmospheric density at the XZ plane
    {
      accel.read();
      densityXZ = (( 2 * mass * accel.x) / ((orbitalVelocity * orbitalVelocity) * dragCoefficient * crossSectionalAreaXZ));
    }

    void calcDensityYZ() //calculates atmospheric density at the YZ plane
    {
      accel.read();
      densityYZ = (( 2 * mass * accel.y) / ((orbitalVelocity * orbitalVelocity) * dragCoefficient * crossSectionalAreaYZ));
    }

    void calcDensityXY() //calculates atmospheric density at the XY plane
    {
      accel.read();
      densityXY = (float) (( 2 * mass * accel.x) / ((orbitalVelocity * orbitalVelocity) * dragCoefficient * crossSectionalAreaXY));
    }

    float getDensityXZ()
    {
      calcDensityXZ();
      Serial.println("haha");
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
    void start()
    {
      serialConnection.begin(9600);
      densityXZ; //calculated by formular
      densityYZ; //calculated by formular
      densityXY; //calculated by formular
      accel.begin();
      ambient.begin();
      rgb.begin();
      lum.begin();
    }

    void sendToEhub()
    {
     /* serialConnection.println(accel.readToJSON("accelerometerx"));
      serialConnection.println(accel.readToJSON("accelerometery"));
      serialConnection.println(accel.readToJSON("accelerometerz"));
      serialConnection.println(lum.readToJSON("luminosity"));
      serialConnection.println(rgb.readToJSON("rgbred"));
      serialConnection.println(rgb.readToJSON("rgbgreen"));
      serialConnection.println(rgb.readToJSON("rgbblue"));
      serialConnection.println(ambient.readToJSON("ambientTemp"));
*/
     // if (millis() % (dayTime) <= dayTime/2) //dayTime < 88642000
      {
     /*   serialConnection.println(valuesToJSON("DustTemperatureDay", "Storm=10/Calm=0/OFF=-999", getDustTempDay()));
        serialConnection.println(valuesToJSON("DustLuminosityDay", "Storm=10/Calm=0/OFF=-999", getDustCoverDay()));
        serialConnection.println(valuesToJSON("DustColorDay", "Storm=10/Calm=0/OFF=-999", getDustColorDay())); */
        Serial.println(getDensityXY());
        serialConnection.println(valuesToJSON("DensityXZDay", "kg/m^3", getDensityXZ()));
        serialConnection.println(valuesToJSON("DensityYZDay", "kg/m^3", getDensityYZ()));
        serialConnection.println(valuesToJSON("DensityXYDay", "kg/m^3", getDensityXY()));
     /*   serialConnection.println(valuesToJSON("DustTemperatureDay", "Storm=10/Calm=0/OFF=-999", -999));
        serialConnection.println(valuesToJSON("DustLuminosityDay", "Storm=10/Calm=0/OFF=-999", -999));
        serialConnection.println(valuesToJSON("DustColorNight", "Storm=10/Calm=0/OFF=-999", -999));
        serialConnection.println(valuesToJSON("DensityXZNight", "kg/m^3", -999));
        serialConnection.println(valuesToJSON("DensityYZNight", "kg/m^3", -999));
        serialConnection.println(valuesToJSON("DensityXYNight", "kg/m^3", -999));*/
      }
 /*     else if ((millis() % (dayTime)) > dayTime/2) //nightTime > 88642000
      {
        serialConnection.println(valuesToJSON("DustTemperatureNight", "Storm=10/Calm=0/OFF=-999", getDustTempNight()));
        serialConnection.println(valuesToJSON("DustLuminosityNight", "Storm=10/Calm=0/OFF=-999", getDustCoverNight()));
        serialConnection.println(valuesToJSON("DustColorNight", "Storm=10/Calm=0/OFF=-999", getDustColorNight()));
        serialConnection.println(valuesToJSON("DensityXZNight", "kg/m^3", getDensityXZ()));
        serialConnection.println(valuesToJSON("DensityYZNight", "kg/m^3", getDensityYZ()));
        serialConnection.println(valuesToJSON("DensityXYNight", "kg/m^3", getDensityXY()));
        serialConnection.println(valuesToJSON("DustTemperatureDay", "Storm=10/Calm=0/OFF=-999", -999));
        serialConnection.println(valuesToJSON("DustLuminosityDay", "Storm=10/Calm=0/OFF=-999", -999));
        serialConnection.println(valuesToJSON("DustColorDay", "Storm=10/Calm=0/OFF=-999", -999));
        serialConnection.println(valuesToJSON("DensityXZDay", "kg/m^3", -999));
        serialConnection.println(valuesToJSON("DensityYZDay", "kg/m^3", -999));
        serialConnection.println(valuesToJSON("DensityXYDay", "kg/m^3", -999));
      }*/
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
