#include <SPI.h>
#include <Wire.h>
//#include <stdio.h>
//#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32; NOTE: GEEKCREIT 0.96" 128x64 DISPLAY USES 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT   32
#define LOGO_WIDTH    48
#define BETA_NTC      3950


// ---------------------------------------------------------
// ----- GLOBAL -----
// ---------------------------------------------------------

// constants
const int TempSensorPin = A0;
const long R1 = 100000;
const int Interval = 1000;
const int ButtonPressTimeout = 300;
const int ShotTimeRefreshInterval = 100;
const int PIButtonPin = 2;
const int PumpDetectorPin = 4;
const int PILEDPin = 12;
const int PumpLEDPin = 10;

// variables
unsigned long PreviousTempMillis = 0;
unsigned long PreviousPIMillis = 0;
unsigned long ShotStartTime = 0;
bool TempFirstRun = 1;
bool PIModeState = 0;
bool PIButtonState = 0;
bool PumpState;
bool PreviousPumpState = 0;


// LCD Assistant settings: Horizontal byte orientation, little endian, 8 pixels/byte
// WIDTH AND HEIGHT MULTIPLES OF 16!
const unsigned char PROGMEM SpLogo [] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x08, 0x00,
  0x00, 0x00, 0x03, 0x80, 0x0C, 0x18, 0xE8, 0x9D, 0x21, 0x80, 0x0C, 0x25, 0x08, 0xA1, 0xA1, 0x80,
  0x0C, 0x21, 0x08, 0xA1, 0xA1, 0x80, 0x0C, 0x18, 0xC8, 0x99, 0x61, 0x80, 0x0C, 0x05, 0x08, 0xA1,
  0x61, 0x80, 0x0C, 0x25, 0x05, 0x21, 0x21, 0x80, 0x0C, 0x18, 0xE2, 0x1D, 0x21, 0x80, 0x0C, 0x00,
  0x00, 0x00, 0x01, 0x80, 0x0D, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x0C, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0,
  0x0C, 0x00, 0x00, 0x00, 0x01, 0x80, 0x0C, 0x63, 0x1C, 0xC6, 0x31, 0x80, 0x0C, 0x94, 0xA1, 0x29,
  0x49, 0x80, 0x0C, 0x94, 0xA1, 0x08, 0x49, 0x80, 0x0C, 0xE7, 0x18, 0xC6, 0x49, 0x80, 0x0C, 0x86,
  0x20, 0x21, 0x49, 0x80, 0x0C, 0x85, 0x21, 0x29, 0x49, 0x80, 0x0C, 0x84, 0x9C, 0xC6, 0x31, 0x80,
  0x0C, 0x00, 0x00, 0x00, 0x01, 0x80, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x80, 0x0F, 0xFF, 0xFF, 0xFF,
  0xFC, 0x00, 0x07, 0xFF, 0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


// ---------------------------------------------------------
// ----- MAIN OPERATIONS -----
// ---------------------------------------------------------

// initialize serial connection to display, show SevenPresso logo
void setup()
{
  Serial.begin(9600);                               //start the serial communication via USB, mainly used for debugging in this project

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  pinMode(PILEDPin, OUTPUT);
  pinMode(PumpLEDPin, OUTPUT);
  pinMode(PIButtonPin, INPUT);
  pinMode(PumpDetectorPin, INPUT);

  display.clearDisplay();                           // clear the display, we need a fresh start
  draw_logo(LOGO_WIDTH, LOGO_HEIGHT, SpLogo);       // make it fancy with a teeny tiny 7P logo

  display.clearDisplay();                           // make way for actually useful information
  draw_frame();                                     // draw the main frame to separate the three main display units
  write_main_labels();                              // create the labels SHOT TIME and PI MODE (upper/main display units)
  write_temp_labels();                              // create the labels CUR, MIN, MAX, UNIT: C (temp display unit)
  zero_shot_time();                                 // zero the shot time to be ready for everything LESSGOOOOOO
}


// main loop (calculate temperatures, display stuff, etc.)
void loop()
{
  PIButtonState = digitalRead(PIButtonPin);
  PumpState = digitalRead(PumpDetectorPin);
  unsigned long CurrentMillis = millis();

  pi_mode_check(CurrentMillis);                     // estimated duration: < 10 ms
  temp_refresh(CurrentMillis);                      // estimated duration: 60 ms
  shottime_refresh(CurrentMillis);
}


