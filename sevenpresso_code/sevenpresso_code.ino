#include <SPI.h>
#include <Wire.h>
//#include <stdio.h>
//#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include "sevenpresso_gfx_helper.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT   32
#define LOGO_WIDTH    48
#define BETA_NTC      3950


// ---------------------------------------------------------
// ----- GLOBAL VARIABLES -----
// ---------------------------------------------------------

//char OldTemp[];
const int TempSensorPin = A0;
const long R1 = 100000;
const int Interval = 1000;
const int ButtonPressTimeout = 300;
const int PIButtonPin = 2;
const int PILEDPin = 12;

unsigned long PreviousTempMillis = 0;
unsigned long PreviousPIMillis = 0;
bool TempFirstRun = 1;
int PIModeState = 0;
int PIButtonState = 0;


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
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  pinMode(PILEDPin, OUTPUT);
  pinMode(PIButtonPin, INPUT);

  display.clearDisplay();
  draw_logo(LOGO_WIDTH, LOGO_HEIGHT, SpLogo);

  display.clearDisplay();
  draw_frame();
  write_main_labels();
  write_temp_labels();
  zero_shot_time();
}


// main loop (calculate temperatures, display stuff, etc.)
void loop()
{
  PIButtonState = digitalRead(PIButtonPin);
  unsigned long CurrentMillis = millis();

/*
  if (PIModeState == HIGH)
  {
    digitalWrite(PILEDPin, HIGH);
  } else
  {
    digitalWrite(PILEDPin, LOW);
  }
*/

if ((CurrentMillis - PreviousPIMillis >= ButtonPressTimeout) && (PIButtonState == HIGH))
  {
    PreviousPIMillis = CurrentMillis;
    switch_pi_mode();
  }


  if (CurrentMillis - PreviousTempMillis >= Interval) 
  {
    PreviousTempMillis = CurrentMillis;
    float TempC = calc_temp(TempSensorPin, R1);
  
    //serial_print_temp(TempC);
    display_print_temp(TempC);
  }
}


// ---------------------------------------------------------
// ----- DISPLAY OPERATIONS -----
// ---------------------------------------------------------

// DEBUG FUNCTION
void serial_print_temp(float Temperature)
{
  Serial.print("Temp = ");
  Serial.print(Temperature, 1);
  Serial.print(" °C\n");
}


// refresh the calculated temperature
void display_print_temp(float Temperature)
{
  int CursorX = 29;
  int CursorY = 46;
  //char Tempstring[32];

  clear_old_temp_text();
  
  display.setTextSize(1);                     // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  
  display.setCursor(CursorX, CursorY);
  Serial.print(Temperature, 1);
  //sprintf(Tempstring, "CUR %d.%01d", (int)Temperature, (int)(Temperature*10)%10);
  //Serial.print(Tempstring);
  display.println(Temperature, 1);
  
  display.display();

  //OldTemp = strcat(Tempstring, "");
  TempFirstRun = 0;
  Serial.print("Temperature displayed\n");
}


// overwrite old temperature with the same text in black
// could also be realized via a black rectangle over the area
// green areas in paint
void clear_old_temp_text()
{
  display.writeFillRect(29, 46, 31, 7, SSD1306_BLACK); // current temp
  display.writeFillRect(29, 54, 31, 7, SSD1306_BLACK); // min temp
  display.writeFillRect(92, 54, 31, 7, SSD1306_BLACK); // max temp

}

void clear_shot_time()
{
  display.writeFillRect(34, 5, 51, 35, SSD1306_BLACK);
}

void zero_shot_time()
{
  display.setTextSize(5);                     // 6x:5x pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text

  clear_shot_time();
  display.setCursor(34, 5);
  display.println("00");

  display.display();
}


// draw the SevenPresso intro logo, only used on startup
void draw_logo(int LogoWidth, int LogoHeight, const unsigned char LogoFile[])
{
    //display.clearDisplay();

    display.drawBitmap(
        (display.width()  - LogoWidth) / 2,
        (display.height() - LogoHeight) / 2,
        LogoFile, LogoWidth, LogoHeight, 1);
    display.display();
    delay(1000);
}


// draw the frame around everything
void draw_frame()
{
  // 7
  display.drawLine(2, 0, 127, 0, SSD1306_WHITE);
  display.drawLine(3, 1, 127, 1, SSD1306_WHITE);
  display.drawLine(126, 2, 126, 60, SSD1306_WHITE);
  display.drawLine(127, 2, 127, 61, SSD1306_WHITE);
  display.drawLine(4, 43, 125, 43, SSD1306_WHITE);
  display.drawLine(5, 44, 125, 44, SSD1306_WHITE);

  // L
  display.drawLine(0, 2, 0, 63, SSD1306_WHITE);
  display.drawLine(1, 3, 1, 63, SSD1306_WHITE);
  display.drawLine(2, 62, 124, 62, SSD1306_WHITE);
  display.drawLine(2, 63, 125, 63, SSD1306_WHITE);

  // separator
  display.drawLine(94, 3, 94, 41, SSD1306_WHITE);

  // PI mode box
  display.drawRect(104, 25, 13, 13, SSD1306_WHITE);
}

void write_main_labels()
{
  // timer and mode labels, red areas in paint
  display.setTextSize(1);                     // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text

  display.setCursor(4,5);
  display.println("SHOT");
  display.setCursor(4, 13);
  display.println("TIME");

  display.setCursor(105, 5);
  display.println("PI");
  display.setCursor(99, 13);
  display.println("MODE");
}

void write_temp_labels()
{
  // temperature labels, red areas in paint
  display.setTextSize(1);                     // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text

  display.setCursor(5, 46);
  display.println("CUR");
  display.setCursor(5, 54);
  display.println("MIN");
  display.setCursor(68, 54);
  display.println("MAX");
  display.setCursor(76, 46);
  display.println("UNIT: C");
}


// ---------------------------------------------------------
// ----- CALC OPERATIONS -----
// ---------------------------------------------------------

float calc_temp(int NtcPin, long RRef)
{
  float AnalogIn = 0;
  int i;
  
  for (i=0;i<5;i++)
  {
    AnalogIn += analogRead(NtcPin);
    //Serial.print(AnalogIn);     //debug
    delay(10);
  }
  AnalogIn = AnalogIn/5;
  
  float R2 = RRef * (1023.0 / (float)AnalogIn - 1.0);
  // float lgr = log(R2);
  float RInf = RRef * expf((-1 * BETA_NTC) / (25 + 273.15));  // Const: 0,1763227
  float Tmp = BETA_NTC / log(R2 / RInf);
  
  //float tmp = (1.0 / (1.009249522e-03 + 2.378405444e-04*lgr + 2.019202697e-07*lgr*lgr*lgr));
  float TempC = Tmp - 273.15;

  return TempC;
}


// ---------------------------------------------------------
// ----- HELPER OPERATIONS -----
// ---------------------------------------------------------

void switch_pi_mode()
{
  if (PIModeState == 0)
  {
    digitalWrite(PILEDPin, HIGH);
    PIModeState = 1;
    Serial.write("PI aktiv\n");
    // draw X into PI checkbox
    display.drawLine(106, 27, 114, 35, SSD1306_WHITE);
    display.drawLine(107, 27, 114, 34, SSD1306_WHITE);
    display.drawLine(106, 28, 113, 35, SSD1306_WHITE);
    display.drawLine(106, 35, 114, 27, SSD1306_WHITE);
    display.drawLine(106, 34, 113, 27, SSD1306_WHITE);
    display.drawLine(107, 35, 114, 28, SSD1306_WHITE);
    display.display();
  } else 
  {
    digitalWrite(PILEDPin, LOW);
    PIModeState = 0;
    Serial.write("PI inaktiv\n");
    // remove X in PI checkbox
    display.writeFillRect(105, 26, 11, 11, SSD1306_BLACK);
    display.display();
  }
}