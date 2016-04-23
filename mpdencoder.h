#include <Wire.h>
#include "Arduino.h"
#include <analogmuxdemux.h>


class mpdencoder
{
  private:
    int potTable[4][2] = {{0, 3}, {2, 1}, {6,4}, {5, 7}}; //location of potentiometers on 4051 chip

    int pinS0;
    int pinS1;
    int pinS2;
    int pinX;

    int rotationA[4];
    int rotationB[4];
    int rotationMeanOld[4];
    int rotationMeanNew[4];
    long teller[4];
    int sensorValueA[4];
    int sensorValueB[4];

    AnalogMux encoder;

  public:
    mpdencoder(int pinS0, int pinS1, int pinS2, int pinX);
    void init(int pinS0, int pinS1, int pinS2, int pinX);
    long lees(int i);
};

mpdencoder::mpdencoder(int pinS0, int pinS1, int pinS2, int pinX)  : encoder(pinS0, pinS1, pinS2, pinX)  {

};



long mpdencoder::lees(int i) {



  sensorValueA[i] = encoder.AnalogRead(potTable[i][0]);
  sensorValueB[i] = encoder.AnalogRead(potTable[i][1]);


  /////////////////



  if (sensorValueA[i] < 512)  {                               // 90 - 270 graden
    if (sensorValueB[i] < 512) {                             // 180  -270 graden
      rotationA[i] = 180 + (sensorValueA[i] * 0.1759);
      rotationB[i] = 270 - (sensorValueB[i] * 0.1759);

    }
    if (sensorValueB[i] > 512) {                              // 90-180 graden
      rotationA[i] = 180 - (sensorValueA[i] * 0.1759);
      rotationB[i] = 270 - (sensorValueB[i] * 0.1759);
    }

  }

  if (sensorValueA[i] > 512) {                                // 0-90 of 270-360
    if (sensorValueB[i] < 512) {                               // 270 -360 graden
      rotationA[i] = 180 + (sensorValueA[i] * 0.1759);
      rotationB[i] = 270 + (sensorValueB[i] * 0.1759);
    }
    if (sensorValueB[i] > 512) {                               // 0-90 graden
      rotationA[i] = 180 - (sensorValueA[i] * 0.1759);
      rotationB[i] =  (sensorValueB[i] * 0.1759) - 90;
    }

  }
  /////////////////////////////



  /////////////////
  rotationMeanNew[i] = rotationA[i];
  // rotationMeanNew[i] = (rotationA[i] + rotationB[i])/2;

  if (rotationMeanNew[i] != rotationMeanOld[i]) {
    if (rotationMeanNew[i] > (rotationMeanOld[i])) {
      teller[i]++;
    } else if (rotationMeanNew[i] < (rotationMeanOld[i])) {
      teller[i]--;
    }
        rotationMeanOld[i] = rotationMeanNew[i];
  }
return teller[i];
};
