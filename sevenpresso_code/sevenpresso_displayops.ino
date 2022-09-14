// ---------------------------------------------------------
// ----- DISPLAY OPERATIONS -----
// ---------------------------------------------------------

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
  Serial.print(Temperature, 1);               // DEBUG
  display.println(Temperature, 1);
  
  display.display();

  //OldTemp = strcat(Tempstring, "");
  TempFirstRun = 0;
  Serial.print("Temperature displayed ");
  //Serial.print(millis());
  Serial.print("\n");
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


// tick or clear the PI mode checkbox
void pi_mode_checkbox(bool active)
{
  if (active)
  {
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
    // remove X in PI checkbox
    display.writeFillRect(105, 26, 11, 11, SSD1306_BLACK);
    display.display();
  }
}