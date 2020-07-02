#define	volume_factor 170                       // millisecond required per millimeter high of water column              
#define	sensitivity 0.5                         // Acceptable peep pressure error margin for pristaltic pump
#define DHTTYPE DHT22                           // Sensor type = DHT22  (AM2302)

#define MPX_Sensor_Offset 147                   // Practical measured Offset Value of MPX5010DP Sensor                   
#define MPX_Sensor_Range 3384                   // Millibar Value Transfer Function Constant

#define Lung_Sensor_Update_Rate 93              // LUNG Sensor Update Rate in millisecond
#define Flow_Sensor_Update_Rate 94              // FLOW Sensor Update Rate in millisecond
#define Peep_Sensor_Update_Rate 495             // PEEP Sensor Update Rate in millisecond
#define Redundant_Lung_Pressure_Update 95       // Redundant Lung Pressure Update Rate in ms

#define Serial_Data_Sending_Rate 100            // Serial Data Sending Rate in millisecond
#define Serial_Data_BAUD_Rate 115200
//#define Blue_Pill_VCC 3.388                   // Practical regulated voltage
//#define MPX7002DP_offset 20                   // MPX7002 Offset after voltage devider 


#define IWDG_PR_DIV_4 0x0
#define IWDG_PR_DIV_8 0x1
#define IWDG_PR_DIV_16 0x2
#define IWDG_PR_DIV_32 0x3
#define IWDG_PR_DIV_64 0x4
#define IWDG_PR_DIV_128 0x5
#define IWDG_PR_DIV_256 0x6
