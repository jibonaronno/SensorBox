// Enclosed BMP280 Sensor_1 Pin Config 
#define BMP_SCK_1 32              //  SCL -> Uno 13, Mega 32
#define BMP_MOSI_1 33             //  SDA -> Uno 11, Mega 33
#define BMP_CS_1 34               //  CSB -> Uno 10, Mega 34
#define BMP_MISO_1 35             //  SDO -> Uno 12, Mega 35

//  Ambient BMP280 Sensor_2 Pin Config
#define BMP_SCK_2 32              //  SCL -> Uno 13, Mega 36
#define BMP_MOSI_2 33             //  SDA -> Uno 11, Mega 37 
#define BMP_CS_2 36               //  CSB -> Uno 10, Mega 38
#define BMP_MISO_2 35             //  SDO -> Uno 12, Mega 39

// Other Pin Mappings 0                                                                                                                                                                                                 
#define Lung_Redund_pin PA7          //  MPX7002DP output connected to PA0 for PEEP
#define Lung_Pressure_Pin PA5     //  MPX5010DP output connected to PA1 for LUNG
#define Flow_sensor_Pin PA1       //  MPX5010DP output connected to PA3 for Flow 
#define PEEP_Pressure_Pin PA3     //  MPX5010DP output connected to PA5 for PEEP

#define BuzzerPin 13              //  Buzzer connected at Pin 13

// Humidity Sensor Pin
#define DHTPIN PB2               // what pin we're connected to

// Pump Relay Pins
#define Bus1_Relay_Pin PB12
#define Bus2_Relay_Pin PB13
#define Power_Enable_Relay_Pin PB14
