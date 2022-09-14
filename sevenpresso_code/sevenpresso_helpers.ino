// ---------------------------------------------------------
// ----- HELPER OPERATIONS -----
// ---------------------------------------------------------

void shottime_refresh(unsigned long CurMillis)
{
  //if (())
}

// check if the PI mode is to be changed, caused by a button press after the timeout
// note: switches back and forth every <ButtonPressTimeout> (300 ms) if the button stays pressed
void pi_mode_check(unsigned long CurMillis)
{
  if ((CurMillis - PreviousPIMillis >= ButtonPressTimeout) && (PIButtonState == HIGH))
  {
    PreviousPIMillis = CurMillis;
    switch_pi_mode();
  }
}

// refresh and display the current temperature
void temp_refresh(unsigned long CurMillis)
{
  if (CurMillis - PreviousTempMillis >= Interval) 
  {
    PreviousTempMillis = CurMillis;
    float TempC = calc_temp(TempSensorPin, R1);
  
    //serial_print_temp(TempC);
    display_print_temp(TempC);
  }
}


// switch PI mode and LED state on button press, and show the current state on the display
void switch_pi_mode()
{
  if (PIModeState == 0)
  {
    digitalWrite(PILEDPin, HIGH);
    PIModeState = 1;
    pi_mode_checkbox(true);
    Serial.write("PI aktiv\n");
  } else 
  {
    digitalWrite(PILEDPin, LOW);
    PIModeState = 0;
    pi_mode_checkbox(false);
    Serial.write("PI inaktiv\n");
  }
}


// DEBUG FUNCTION
void serial_print_temp(float Temperature)
{
  Serial.print("Temp = ");
  Serial.print(Temperature, 1);
  Serial.print(" °C\n");
}