#include "Arduino.h"
#include <Wire.h>
#include <SPI.h>

#include <Adafruit_MCP23008.h> // SDA; 18, SCL; 19 || GP0; push1 | GP1; led1 | GP2; push2| GP3;led2 | GP4; push3| GP5; led3| GP6; push4 |GP7 led4;

#include "mpdencoder.h"
#include <analogmuxdemux.h> //  4051 analogmultiplexer: 4 rotary encoders; X; 17 | S0;16 | S1;15 | S2;14
// encoder 0; X0 | X3
// encoder 1; X1 | X2
// encoder 2 ; X4 | X6
// encoder 3; X5 | X7

#include <ButtonV2.h>
#include "mcpButtonV2.h"


#include <U8glib.h> //  OLED ssd1332; CS; 10 | DC; 9 | MOSI; 11 | SCLK; 13 | RST; 8

#include "LFO.h"
#include "ADSR.h"


#define LED0 29
#define PUSH0 30
#define GATE_PIN 31
#define ENV_MOD_PIN 3
#define CUTOFF_PIN 4
#define SAW_PIN 5
#define SQR_PIN 6
#define ACCENT_PIN 20
#define DECAY_PIN 21
#define VCO_PIN A14
#define SLIDE_OUT_PIN 22
#define SLIDE_IN_PIN 23

// all currently used pwmpins in array to shorten setup mode
int allControls[9] = { ENV_MOD_PIN, CUTOFF_PIN, SAW_PIN, SQR_PIN, ACCENT_PIN, DECAY_PIN, GATE_PIN, SLIDE_OUT_PIN, SLIDE_IN_PIN };
int pwmArray[6] = {SAW_PIN, SQR_PIN, ENV_MOD_PIN, DECAY_PIN, CUTOFF_PIN, ACCENT_PIN};

// MCP hardware connection stuff

int pushList[4] = {0, 2, 4, 6};
int ledList[4] = { 1 , 3, 5, 7};

Adafruit_MCP23008 mcp;

//ButtonV2 *button[5];
ButtonV2 button;
mcpButtonV2 mcpButton;


// Analog multiplexer stuff
mpdencoder encoder(16, 15, 14, 17);
long tellerNew[4] = {0, 0, 0, 0};
long tellerOld[4] = {0, 0, 0, 0};

// OLED connection
U8GLIB_NHD31OLED_2X_GR u8g(10, 9); // SPI Com: SCK = 13, MOSI = 11, CS = 10, A0 = 9

// Notes and frequency
int liveNoteCount = 0;
int pitchbendOffset = 0;
float baseNoteFrequency;
bool noteFlag;
byte midiChannel;



#define OSC 0
#define FILTER 1
#define AMP 2
#define PRESETS 3
#define SETUP 4
/*
  #define SLIDE_ADSR 2
  #define CUTOFF_ADSR 0
  #define ENVMOD_ADSR 2
  #define ACCENT_ADSR 0
  #define AMP_ADSR 2

  #define TUNE_LFO 3
  #define CUTOFF_LFO 1
  #define ENVMOD_LFO 3
  #define AMP_LFO 3
*/
// control and display mode stuff
byte viewMode[2] = {0, 0}; // first number is the page, second is the subpage
const char* controlsRot[][5] = {{"OSC", "Tune", "Saw/Square", "Slide", "Tune LFO"}, { "", "", "", "", ""}, {"Slide ADSR", "Attack", "Decay", "Sustain", "Release"},  {"Tune LFO", "Frequency", "Depth", "Shape", " "}, {"Filter", "Cutoff", "Cutoff LFO", "Envmod", "Envmod LFO"}, {"Cutoff LFO", "Frequency", "Depth", "Shape", " "}, {"Cutoff ADSR", "Attack", "Decay", "Sustain", "Release"}, {"Envmod LFO", "Frequency", "Depth", "Shape", " "}, {"Amp", "Decay", "Accent", "Volume", "Amp LFO"}, {"Accent ADSR", "Attack", "Decay", "Sustain", "Release"}, {"Amp ADSR", "Attack", "Decay", "Sustain", "Release"}, {"Amp LFO", "Frequency", "Depth", "Shape", " "}, { "", "", "", "", ""}, { "", "", "", "", ""}, { "", "", "", "", ""}, { "", "", "", "", ""}};
const char* controlsBut[][4] = {{"back", " ", "Slide ADSR", "Tune LFO"}, {"back", "Cutoff LFO", "Cutoff ADSR", "Envmod LFO"}, {"back", "Accent ADSR", "Amp ADSR", "Amp LFO"}, { "", "", "", ""}};

// main array where all controls are stored: 1st; viewmode [0] | 2nd; viewmode [1] | 3rd actual rotary controlled parameters
byte controls[4][4][4];



void setup() {
  Serial.begin(9600);

  // MCP controlled pins
  mcp.begin();      // use default address 0

  for ( int i = 0; i < 4; i++ ) {
    mcp.pinMode(pushList[i], INPUT);
    mcp.pullUp(pushList[i], HIGH);  // turn on a 100K pullup internally
  }
  mcpButton.SetStateAndTime(LOW);
  for ( int i = 0; i < 4; i++ ) {
    mcp.pinMode(ledList[i], OUTPUT);
  }

  // Seperate setupsbutton
  pinMode(PUSH0, INPUT_PULLUP);   //pushbutton setup
  pinMode(LED0, OUTPUT);         // LED setup
  button.SetStateAndTime(LOW);
  // XOX related pins
  for (int i = 0; i < 9; i++) {
    pinMode(allControls[i], OUTPUT);  // all control pins are output
  }
  analogWriteResolution(12);
  for (int i = 0; i < 6; i++) {
    analogWriteFrequency(pwmArray[i], 46875); // change frequency of pwm pins
  }
  analogWriteFrequency(VCO_PIN, 46875); // change frequency of A14

  usbMIDI.setHandleNoteOn(handleNoteOn);
  usbMIDI.setHandleNoteOff(handleNoteOff);
  usbMIDI.setHandlePitchChange(OnPitchChange);
  usbMIDI.setHandleControlChange(handleControlChange);
  makePointer();  // make wavetable pointers accessible
  updateLed();
}

void loop() {
  usbMIDI.read();   // check midi messages
  buttonRead();
  updateOLED();
  for (int i = 0; i < 4; i++) {
    tellerNew[i] = encoder.lees(i);
    if (tellerNew[i] != tellerOld[i]) {
      if (tellerNew[i] > tellerOld[i] + 1) {
        encoderRead(i, 1);  // teller i is rotated right
        tellerOld[i] = tellerNew[i];
      } else if (tellerNew[i] < tellerOld[i] - 1) {
        encoderRead(i, 0 );  // teller i is rotated left
        tellerOld[i] = tellerNew[i];
      }
    }
  }
}
