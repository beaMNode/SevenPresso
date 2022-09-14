// ---------------------------------------------------------
// ----- CALC OPERATIONS -----
// ---------------------------------------------------------

// calculate the current temperature detected by the NTC3950
// formula reference: https://en.wikipedia.org/wiki/Thermistor#B_or_%CE%B2_parameter_equation
float calc_temp(int NtcPin, long RRef)
{
  float AnalogIn = 0;
  int i;
  
  // take five measurements 10 ms apart and take the average to mitigate random variance
  // the analog input is really not /that/ accurate
  for (i=0;i<5;i++)
  {
    AnalogIn += analogRead(NtcPin);
    //Serial.print(AnalogIn);     //debug
    delay(10);
  }
  AnalogIn = AnalogIn/5;
  
  float R2 = RRef * (1023.0 / (float)AnalogIn - 1.0);
  float RInf = RRef * expf((-1 * BETA_NTC) / (25 + 273.15));  // Const: 0,1763227
  float Tmp = BETA_NTC / log(R2 / RInf);
  float TempC = Tmp - 273.15;

  return TempC;
}


// return the shot time since the pump started the first time
int get_passed_time(unsigned long Starttime)
{
  int PassedSeconds = trunc((millis() - Starttime)/1000);
  
  return PassedSeconds;
}