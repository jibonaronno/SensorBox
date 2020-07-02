#include "pins.h"
#include "pt.h"
#include <arduino.h>
#include <DHT.h>
#include "config.h"

static float hum = 10.0;            //  Stores humidity value
static float temp = 20.0;            //  Stores humidity value

DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino


int Humidity_Sensor(struct pt *pt)
{
  static unsigned long lastTimeBuzz = 0;
  PT_BEGIN(pt);
  while(1) {
    lastTimeBuzz = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeBuzz > 2000);
    hum = dht.readHumidity();
    temp = dht.readTemperature();
//  mystring = String(hum);0
//  doc["humidity"] = mystring;
//  mystring = String(temp);
//  doc["temperature"] = mystring;
//  mystring = String(peak_pressure_alarm);
//  doc["peakpressure_alarm"] = mystring;
//  doc["peakpressure_limit"] = "35"; 
//  Serial.print("Buzzer stopped");    
  }
  PT_END(pt);
}

// activate Buzzer every other second

int BuzzBeep(struct pt *pt)
{
  static unsigned long lastTimeBuzz = 0;
  
  PT_BEGIN(pt);
  while(1) {
    lastTimeBuzz = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeBuzz > 1000);
    digitalWrite(BuzzerPin, HIGH);
    //Serial.print("Buzzer start");
    lastTimeBuzz = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeBuzz > 1000);
    digitalWrite(BuzzerPin, LOW);  
    //Serial.print("Buzzer stopped");    
  }
  PT_END(pt);
}
