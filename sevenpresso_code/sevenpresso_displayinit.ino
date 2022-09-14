// ---------------------------------------------------------
// ----- VISUAL INITIALIZATION -----
// ---------------------------------------------------------

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