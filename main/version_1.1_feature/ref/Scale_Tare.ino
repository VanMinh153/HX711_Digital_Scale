int32_t d;
if (_tare == 1)
{
  tare = 0;
  lcd.setCursor(1, 1);
  lcd.print("Taring...       ");
  for (byte i = 0; i < 5; i++)
  {
    d = getData_Avg();
    if (d != 0x7fffff && sensor_error < Absolute_error)
    {
      if (abs(d - getData_()) < Absolute_error)
        break;
    }
    delay(105);
  }
  if (d != 0x7fffff && sensor_error < Absolute_error)
  {
    Zero = d;
    sensor.setZero(Zero);
  }
  else
    Serial.println("Error: Failed to tare the scale");
  lcd_(getWeight());
}
