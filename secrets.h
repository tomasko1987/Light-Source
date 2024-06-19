#include <pgmspace.h>
 #define SECRET
//WIFI
const char WIFI_SSID[] = "oriesok";             
const char WIFI_PASSWORD[] = "deX9hud6";           
 
// Local Home Assistant MQTT Broker
const char* HA_MQTT_SERVER = ""; // Local MQTT server IP Address
const int HA_MQTT_PORT = 1883; // Port (standard 1883 for MQTT)
const char* HA_MQTT_USER = ""; // MQTT User name
const char* HA_MQTT_PASSWORD = ""; // MQTT User password
const char* HA_MQTT_CLIENT = ""; // Local HA Broker Topic
const char* HA_MQTT_TOPIC_INA219_0 = "home/sensors/ina219_0"; // Local HA Broker Topic
const char* HA_MQTT_TOPIC_INA219_1 = "home/sensors/ina219_1"; // Local HA Broker Topic
const char* HA_MQTT_TOPIC_INA219_2 = "home/sensors/ina219_2"; // Local HA Broker Topic