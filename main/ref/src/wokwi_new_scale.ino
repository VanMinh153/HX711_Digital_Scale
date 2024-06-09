#include "HX711_ADC.h"
#include <LiquidCrystal_I2C.h>
HX711_ADC LoadCell(9, 8);
LiquidCrystal_I2C lcd(0x27,16,2);
const int buzzer = 11;
int button1 = 2;
int button2 = 3;
int button3 = 4;
int button4 = 5;
int taree = 6;
int num = 1;
int buttonState = 0;          
int buttonState1 = 0;
float alarm1 = 0;
float alarm2 = 0;
float z1 = 28.3495;


unsigned long prevTime1 = millis();
unsigned long prevTime2 = millis();

long Time1 = 3000;
long Time2 = 1000;
void setup()
{
  pinMode (button1, INPUT_PULLUP);
  pinMode (button2, INPUT_PULLUP);
  pinMode (button3, INPUT_PULLUP);
  pinMode (button4, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode (taree, INPUT_PULLUP);
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Weighing Scale");
  lcd.setCursor(3, 1);
  lcd.print("With Alarm");
  delay(2500);
  lcd.clear();
  delay(150);
  lcd.setCursor(3, 0);
  lcd.print("Taring");
  delay(200);
  lcd.setCursor(9, 0);
  lcd.print(".");
  delay(200);
  lcd.setCursor(10, 0);
  lcd.print(".");
  delay(200);
  lcd.setCursor(11, 0);
  lcd.print(".");
  delay(200);
  lcd.setCursor(12, 0);
  lcd.print(".");
  delay(200);
  delay(1000);
  lcd.clear();
  LoadCell.begin(); 
  LoadCell.start(1000);
  LoadCell.setCalFactor(0.42);
  
   
  

}
void loop() {
 unsigned long currentTime = millis();
 static unsigned long currentTime1 = millis();
 LoadCell.update(); 
 float i = LoadCell.getData(); 
 float z = i/28.3495;
 if(digitalRead(button3) == LOW){
 alarm1=0;
 alarm2=0;
 lcd.clear();}

 buttonState = digitalRead(button2);
   buttonState1 = digitalRead(button1);

       if(buttonState == 0)      
       currentTime1 = millis();
       if(buttonState1 == 0)
       currentTime1 = millis();

 switch(num){
  case 1:
  {
   if(millis() - currentTime1 >= 4000) // else
    { num = 1;
 }  
   else
     num = num; 
  break;
}
case 2:
{
  if(digitalRead(button1) == LOW){alarm1 = alarm1+1;
  delay(75);}
  else if(digitalRead(button2) == LOW){alarm1 = alarm1-1;
  delay(75);}
 if(digitalRead(button3) == LOW)
 alarm1=0;
  lcd.setCursor(1, 0);
  lcd.print("Set for Gram");
  lcd.setCursor(0, 1); // set cursor to secon row
  lcd.print(alarm1, 0); // print out the retrieved value to the second row
  lcd.print("g ");
  
  alarm2 = alarm1/z1;
  lcd.setCursor(9, 1);
  lcd.print(alarm2, 1);
  lcd.print("oz ");

   if(millis() - currentTime1 >= 8000) 
     { num = 1;
     lcd.clear();
 lcd.setCursor(1, 0);
 lcd.print("Weighing Scale");
 delay(500);
 lcd.clear();}    
   else
     num = num;   
  break;}
  case 3:
 {
  if(digitalRead(button1) == LOW){alarm2 = alarm2+1;
  delay(75);}
  else if(digitalRead(button2) == LOW){alarm2 = alarm2-1;
  delay(75);}

lcd.setCursor(1, 0);
  lcd.print("Set for Ounce");
 lcd.setCursor(0, 1); // set cursor to secon row
  lcd.print(alarm1, 0); // print out the retrieved value to the second row
  lcd.print("g  ");
  alarm1 = alarm2 * z1;
  lcd.setCursor(9, 1);
  lcd.print(alarm2, 1);
  lcd.print("oz ");

   if(digitalRead(button3) == LOW)
 alarm2=0;

 if(millis() - currentTime1 >= 4000) // else
     { num = 1;
     lcd.clear();
 lcd.setCursor(1, 0);
 lcd.print("Weighing Scale");
 delay(500);
 lcd.clear();}    
   else
     num = num; 
  break;}
  }

 



  if (digitalRead (taree) == LOW)
  {
    lcd.clear();
    lcd.setCursor(3, 0);
  lcd.print("Taring");
  delay(200);
  lcd.setCursor(9, 0);
  lcd.print(".");
  delay(200);
  lcd.setCursor(10, 0);
  lcd.print(".");
  delay(200);
  lcd.setCursor(11, 0);
  lcd.print(".");
  delay(200);
  lcd.setCursor(12, 0);
  lcd.print(".");
  delay(200);
  delay(500);
    LoadCell.start(1000);
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.clear();
  }
 if (digitalRead (button4) == LOW){{if(num>2) num = 1;
 else num++;
 currentTime1 = millis();
 delay (300);
 lcd.clear();
 delay (300);
}
if (num==3)
{ lcd.setCursor(1, 0);
 lcd.print("Set for Ounce");
 delay(500);
 lcd.clear();}
 
 else if (num==2)
 {lcd.setCursor(1, 0);
 lcd.print("Set for Gram");
 delay(500);
 lcd.clear();}
 else if (num==1)
 {
 lcd.setCursor(1, 0);
 lcd.print("Weighing Scale");
 delay(500);
 lcd.clear();}
 }

if(currentTime - prevTime1 > Time1){
  
  if(i>alarm1 && alarm1>0 )
  {
  noTone(buzzer);
   delay(300);
  tone(buzzer,250);
  delay(300);
  noTone(buzzer);
   delay(300);
  tone(buzzer,450); 
  delay(300);
  noTone(buzzer);
   delay(300);}
   
  prevTime1 = currentTime;
  }
  if(i<-0.9){i==i;} 
  else if(-0.9<i && i<0.1){i=0;}
  
  if(currentTime - prevTime2 > Time2){
  if (i>=3000 && num<4)
  {
    lcd.clear();
    delay(500);
    num=4;
  }else if(num==4 && i<3000){
    num=1;
    lcd.clear();
    delay(1000);
  }
   if(num==1){
    lcd.setCursor(1, 0);
   lcd.print("Weighing Scale");
   lcd.setCursor(0,1);
   lcd.print(i, 0);
   lcd.print("g  ");
   lcd.setCursor(9,1);
   lcd.print(z, 1);
   lcd.print("oz ");}
prevTime2 = currentTime;
   if(num==4){
  lcd.setCursor(0, 0);
  lcd.print("  Over Loaded   "); 
  delay(500);
  lcd.clear();
  delay(500);}

  
  }
  Serial.print(currentTime);
  Serial.print("    ");
  Serial.print(millis());
  Serial.print("    ");
  Serial.print(prevTime2);
  Serial.println("    ");
}