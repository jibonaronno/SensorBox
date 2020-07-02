/* A Program for Instrumentation and Sensing of a Medical Ventillator:
External Peripherals : Pressure Sensor BMP280, MPX7002DP
Author        : Sadik shahriar
Team Lead     : Faruk Hannan 
Lead Developer: Md. Mazidul Haque
Board         : STM32F103C8 BLUEPILL
Date          : 6th April, 2020
Crystal       : 16 MHz 
MOD
*/
//####################################################################
// Libraries to include and PIN Mapping
//####################################################################
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
//#include <Adafruit_BME280.h>
#include <ArduinoJson.h>
#include <pt.h>
#include "sensor.h"
#include "pins.h"
#include "config.h"
#include "WatchDog.h"

//####################################################################
// Variables, Constants and Function Declaration
//####################################################################
float MPX_milibar = 0.0, Flow_Sensor_Millibar;
float pressure_1, pressure_2, pressure_3, p_bmp_delta;
float p_bmp_ref , p_amb, p_dut;
float  motor_enable=0;        // Volume Factor = Time in millisecond to fill 1 millimeter volume
float new_peep_level= 4, set_peep_level= 4,  motor_status = 0; /////////// water column in cmH2O
//unsigned long previousTimePump = millis();
long runtime;
int MPX7002DP_offset;
float Lung_Pressure_Millibar, Lung_Pressure_Redundant;
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing
extern float hum, temp;            //  Stores humidity value
char messageFromPC[numChars] = {0}; // variables to hold the parsed data
int integerFromPC = 0;
float floatFromPC = 0.0;
boolean newData = false;

Adafruit_BMP280 bmp_1 (BMP_CS_1, BMP_MOSI_1, BMP_MISO_1,  BMP_SCK_1);
Adafruit_BMP280 bmp_2 (BMP_CS_2, BMP_MOSI_2, BMP_MISO_2,  BMP_SCK_2);
//Adafruit_BME280 bme_2 (BME_CS_2, BME_MOSI_2, BME_MISO_2,  BME_SCK_2);

static struct pt pt1, pt2, pt3, pt4, pt5, pt6, pt7, pt8, pt9 ;  // Declare the protothreads

//####################################################################
// Task 1: LUNG pressure with MPX5010DP 
//####################################################################
static int Lung_Pressure_Value(struct pt *pt)
{
  static unsigned long lastTimePrint = 0;
  static float  Pressure_in_kPa = 0.0;
  PT_BEGIN(pt);
  while(1) {
    lastTimePrint = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimePrint > Lung_Sensor_Update_Rate);
    int Pressure_Sensor_Value = analogRead(Lung_Pressure_Pin);  
    Pressure_Sensor_Value = Pressure_Sensor_Value - MPX_Sensor_Offset ; 
    Pressure_in_kPa = map(Pressure_Sensor_Value, 0, MPX_Sensor_Range, 0, 10000)/1000.0;        // mpx5010DP
    Lung_Pressure_Millibar = Pressure_in_kPa * 10;
    //Serial.println(Lung_Pressure_Millibar);
  }
  PT_END(pt);
}
//####################################################################
// Task 2: PEEP Pressure Value Check with MPX5010DP 
//####################################################################
static int PEEP_Pressure_Value(struct pt *pt)
{
  static unsigned long lastTimePrint = 0;
  static float  Pressure_in_kPa = 0.0;
  PT_BEGIN(pt);
  while(1) {
    lastTimePrint = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimePrint > Peep_Sensor_Update_Rate);
    int Pressure_Sensor_Value = analogRead(PEEP_Pressure_Pin);  
    Pressure_Sensor_Value = Pressure_Sensor_Value - MPX_Sensor_Offset ; 
    Pressure_in_kPa = map(Pressure_Sensor_Value, 0, MPX_Sensor_Range, 0, 10000)/1000.0;        // mpx5010DP
    set_peep_level = Pressure_in_kPa * 10;
    //Serial.println(Lung_Pressure_Millibar);
  }
  PT_END(pt);
}
//####################################################################
// Task 3: Redundant Lung Pressure with MPX5010DP
//####################################################################
static int Redundant_Lung_Pressure (struct pt *pt)
{
  static unsigned long lastTimePrint = 0;
  static float  Pressure_kPa = 0.0;
  PT_BEGIN(pt);
  while(1) {
    lastTimePrint = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimePrint > Redundant_Lung_Pressure_Update);
    int Pressure_Value = analogRead(Lung_Redund_pin);  
    Pressure_Value = Pressure_Value - MPX_Sensor_Offset; 
    Pressure_kPa = map(Pressure_Value, 0, MPX_Sensor_Range, 0, 10000)/1000.0;        // mpx5010DP
    Lung_Pressure_Redundant  = Pressure_kPa * 10;
    //Serial.println(Lung_Pressure_Redundant);
  }
  PT_END(pt);
}

//####################################################################
// Task 4: FLOW Sensor value with MPX7002DP
//####################################################################
static int Flow_Sensor_Value(struct pt *pt)
{
  static unsigned long lastTimePrint = 0;
  //static float  Pressure_in_millibar = 0.0;
  static float  Pressure_in_kPa = 0.0;
  PT_BEGIN(pt);
  while(1) {
    lastTimePrint = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimePrint > 100);
    int Pressure_Sensor_Value = analogRead(Flow_sensor_Pin);   // 
    Pressure_Sensor_Value = Pressure_Sensor_Value - MPX7002DP_offset;
     
    if (Pressure_Sensor_Value < 404) {                       //408
      Pressure_in_kPa = -2.0;
    } 
    else {
      if (Pressure_Sensor_Value > 3686) {                       // 3686, 3627
        Pressure_in_kPa = 2.0;
      } 
      else {  
        //Pressure_in_kPa = map(Pressure_Sensor_Value, 102, 921, -2000, 2000)/1000.0;  // mpx7002DP 
        Pressure_in_kPa = map(Pressure_Sensor_Value, 410, 3627, -2000, 2000)/1000.0;   // mpx7002DP   
      }
    }
    Flow_Sensor_Millibar = Pressure_in_kPa * 10;
  }
  PT_END(pt);
}

//####################################################################
// Task 5: Stop the Pump after runtime passes   <go,5.5,111>
//####################################################################
static int Peep_Pressure_Set(struct pt *pt)
{
  static unsigned long lastRunTime = 0;
  PT_BEGIN(pt);
  while(1) {
    lastRunTime = millis();
    PT_WAIT_UNTIL(pt, millis() - lastRunTime > runtime);
    digitalWrite(Bus1_Relay_Pin, LOW);
    digitalWrite(Bus2_Relay_Pin, LOW);
    digitalWrite(Power_Enable_Relay_Pin, LOW);
    //set_peep_level = new_peep_level;  
    //set_peep_level = MPX_milibar;
    motor_status = 0;
    motor_enable = 0;
    //Serial.println("Motor Stopped");      
  }
  PT_END(pt);
}
//####################################################################
// Task 6: Subroutine to parse Serial Input continuously  
//####################################################################
static int Serial_Input_Parsing(struct pt *pt)
{
  static unsigned long lastTimeCheck = 0;
  
  PT_BEGIN(pt);
  while(1) {
    lastTimeCheck = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeCheck > 5);
    recvWithStartEndMarkers();
    if (newData == true) {
      strcpy(tempChars, receivedChars);
        // this temporary copy is necessary to protect the original data
        //   because strtok() used in parseData() replaces the commas with \0
      parseData();
      //showParsedData();
      newData = false;
            
      if (messageFromPC[0]=='E'){
        motor_enable = 1;
        //Serial.println("motor_enable = TRUE");
      }
      
      if (messageFromPC[0]=='D'){
        motor_enable = 0;
        //Serial.println("motor_enable = FALSE");
      }
      MPX7002DP_offset = integerFromPC;
      new_peep_level = floatFromPC;
    }
  }
  PT_END(pt);
}
//####################################################################
// Task 7: Subroutine to parse Serial Output continuously  
//####################################################################
static int Serial_Output_Parsing(struct pt *pt)
{
  static unsigned long lastTimeCheck = 0;
  
  PT_BEGIN(pt);
  while(1) {
    lastTimeCheck = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimeCheck > Serial_Data_Sending_Rate);
    
    Serial.print(Lung_Pressure_Millibar);
    Serial.print(",");
    Serial.print(Lung_Pressure_Redundant);
    Serial.print(",");
    Serial.print(Flow_Sensor_Millibar);
    Serial.print(",");
    Serial.print(set_peep_level);
    Serial.print(",");
    Serial.print(lastTimeCheck);
    Serial.println();    
  }
  PT_END(pt);
}

//#################################################################### 
// Task 8 : Reload WatchDog Timer every 50 milli seconds
//####################################################################
static int WatchDog_Protothread(struct pt *pt)
{
  static unsigned long lastTimePrint = 0;
  PT_BEGIN(pt);
  while(1) {
    lastTimePrint = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimePrint > WatchDog_Time_Out);
    //Serial.println("WacthOut");
    iwdg_init(IWDG_PRE_32, 100);
  }
  PT_END(pt);
}

//####################################################################
// Task 9: Measure Lung Pressure From BMP Sensor
//####################################################################
static int Lung_Pressure_BMP(struct pt *pt)
{
  static unsigned long lastTimePrint = 0;
  static float  Pressure_in_kPa = 0.0;
  PT_BEGIN(pt);
  while(1) {
    lastTimePrint = millis();
    PT_WAIT_UNTIL(pt, millis() - lastTimePrint > 50);
    pressure_1 = bmp_1.readPressure();
    p_bmp_ref = pressure_1/100;        // Inside Pressure in Milibar 
    pressure_2 = bmp_2.readPressure();
    p_amb = pressure_2/100;        // Ambient Pressure in Milibar
//  pressure_3 = bme_2.readPressure();
//  p_dut = pressure_2/100;        // Ambient Pressure in Milibar
 
    p_bmp_delta = p_bmp_ref - p_amb + 1.5;
    //Serial.println(Lung_Pressure_Millibar);
  }
  PT_END(pt);
}
//####################################################################

//#################################################################### 

void setup() {
  
  pinMode(BuzzerPin, OUTPUT); // Set buzzer - pin 9 as an output
  pinMode(Bus1_Relay_Pin, OUTPUT); 
  pinMode(Bus2_Relay_Pin, OUTPUT);  // Set buzzer - pin 9 as an output
  pinMode(Power_Enable_Relay_Pin, OUTPUT); // Set buzzer - pin 9 as an output
  
  digitalWrite(Bus1_Relay_Pin, LOW);   //  Relay BUS1
  digitalWrite(Bus2_Relay_Pin, LOW);   //  Relay BUS2
  digitalWrite(Power_Enable_Relay_Pin, LOW);  //  Relay Power
  
  digitalWrite(BuzzerPin, LOW);

  Serial.begin(Serial_Data_BAUD_Rate);
  //Serial.println("Start:");
  
  bmp_1.begin();
  bmp_2.begin();

  PT_INIT(&pt1);  // LUNG
  PT_INIT(&pt2);  // PEEP
  PT_INIT(&pt3);  // RDND
  PT_INIT(&pt4);  // FLOW 
  PT_INIT(&pt5);  // STOP
  PT_INIT(&pt6);  // RXIN
  PT_INIT(&pt7);  // TXOUT
  PT_INIT(&pt8);  // Redundant BMP
  PT_INIT(&pt9);  // WatchDog Reload
      
  PEEP_Pressure_Value(&pt2);                                                        
  WatchDog_Protothread(&pt8);   
} 

//####################################################################
void loop(){
//  StaticJsonDocument<200> doc;              // Json encoding file
  Lung_Pressure_Value(&pt1);                  // Subroutine to print presure value                                      
  PEEP_Pressure_Value(&pt2);
  Redundant_Lung_Pressure(&pt3);
  Flow_Sensor_Value(&pt4);
  
//  if (MPX_max_Pressure < MPX_milibar){
//    BuzzBeep(&pt2);                         // Checking if any of the pressure value exceeds the mark
//  }
  
  if ((new_peep_level > (set_peep_level + sensitivity))&& motor_enable){
      runtime = volume_factor *(new_peep_level-set_peep_level) * 10;  // CM to mm, millisecond
      digitalWrite(Bus1_Relay_Pin, LOW);
      digitalWrite(Bus2_Relay_Pin, LOW);
      digitalWrite(Power_Enable_Relay_Pin, HIGH);
//      Serial.print(set_peep_level);
//      Serial.print("\t");
//      Serial.println(new_peep_level);
      //Serial.println("Motor Flooding");
      //Peep_Pressure_Set(&pt4);
      motor_status = 1;
    }  
  if ((new_peep_level < (set_peep_level - sensitivity))&& motor_enable){
      runtime = volume_factor *(set_peep_level-new_peep_level) * 10;  // CM to mm, millisecond
      digitalWrite(Bus1_Relay_Pin, HIGH);
      digitalWrite(Bus2_Relay_Pin, HIGH);
      digitalWrite(Power_Enable_Relay_Pin, HIGH);
//      Serial.print(set_peep_level);
//      Serial.print("\t");
//      Serial.println(new_peep_level);
//      Serial.println("Motor Draining");
      motor_status = 1;   
    }
    
    
    if (motor_status == 1){
      Peep_Pressure_Set(&pt5);
    }
      
  Serial_Input_Parsing(&pt6);                 // Serial Input change monitoring
  Serial_Output_Parsing(&pt7);
  WatchDog_Protothread(&pt8);
  Lung_Pressure_BMP(&pt9);
  
  //Humidity_Sensor(&pt10);
  //serializeJson(doc, Serial);
  //serializeJsonPretty (doc, Serial);
  
}
//####################################################################  

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();
        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0';    // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }
        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void parseData() {                            // split the data into its parts
    char * strtokIndx;                        // this is used by strtok() as an index
    strtokIndx = strtok(tempChars,",");       // get the first part - the string
    strcpy(messageFromPC, strtokIndx);        // copy it to messageFromPC
    strtokIndx = strtok(NULL, ",");           // this continues where the previous call left off
    integerFromPC = atoi(strtokIndx);         // convert this part to an integer
    strtokIndx = strtok(NULL, ",");
    floatFromPC = atof(strtokIndx);         // convert this part to a float

}

void showParsedData() {
    Serial.print("Message ");
    Serial.println(messageFromPC);
    Serial.print("Integer ");
    Serial.println(integerFromPC);
    Serial.print("Float ");
    Serial.println(floatFromPC);
}
