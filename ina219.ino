#include "secrets.h"
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Define three INA219 instances with different addresses
Adafruit_INA219 ina219_0(0x40); // Default address
Adafruit_INA219 ina219_1(0x41); // Address with A0 connected to VCC
Adafruit_INA219 ina219_2(0x44); // Address with A0 and A1 configured for this address

bool ina219_0_initialized = false;
bool ina219_1_initialized = false;
bool ina219_2_initialized = false;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void checkWifiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting to reconnect...");
    setup_wifi(); // Reuse the setup_wifi function to reconnect
  }
}

void reconnectMQTT() {
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(HA_MQTT_CLIENT, HA_MQTT_USER, HA_MQTT_PASSWORD)) {
      Serial.println("connected to MQTT broker");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
      return; // Early return to try reconnecting in the next loop iteration
    }
  }
}

void setup(void) {
  Serial.begin(115200);
  while (!Serial) {
    // Wait for serial port to connect. Needed for native USB port only
  }

  setup_wifi();
  client.setServer(HA_MQTT_SERVER, HA_MQTT_PORT);

  // Attempt to initialize all three INA219 sensors
  ina219_0_initialized = ina219_0.begin();
  Serial.println(ina219_0_initialized ? "INA219 at address 0x40 found." : "Failed to find INA219 chip at address 0x40");
  ina219_1_initialized = ina219_1.begin();
  Serial.println(ina219_1_initialized ? "INA219 at address 0x41 found." : "Failed to find INA219 chip at address 0x41");
  ina219_2_initialized = ina219_2.begin();
  Serial.println(ina219_2_initialized ? "INA219 at address 0x44 found." : "Failed to find INA219 chip at address 0x44");
}

void publishSensorData(Adafruit_INA219* sensor, const char* topic, bool isInitialized) {
  if (isInitialized) {
    float shuntvoltage = sensor->getShuntVoltage_mV();
    float busvoltage = sensor->getBusVoltage_V();
    float current_mA = sensor->getCurrent_mA();
    float power_mW = sensor->getPower_mW();
    float loadvoltage = busvoltage + (shuntvoltage / 1000);

    // Prepare JSON payload
    StaticJsonDocument<256> doc;
    doc["Bus_Voltage_V"] = busvoltage;
    doc["Shunt_Voltage_mV"] = shuntvoltage;
    doc["Load_Voltage_V"] = loadvoltage;
    doc["Current_mA"] = current_mA;
    doc["Power_mW"] = power_mW;
    char payload[256];
    serializeJson(doc, payload);

    client.publish(topic, payload);
    Serial.println(" topic: " + String(topic) + "\n" + " Bus Voltage: " + String(busvoltage) + " V \n" + " Shunt Voltage: " + String(shuntvoltage) + " mV \n" + " Load Voltage: " + String(loadvoltage) + " V \n" + " Current: " + String(current_mA) + " mA \n" + " Power: " + String(power_mW) + " mW \n");
  }
}

void loop(void) {
  checkWifiConnection(); // Ensure WiFi connection
  reconnectMQTT(); // Ensure MQTT connection
  
  Adafruit_INA219* sensors[] = {&ina219_0, &ina219_1, &ina219_2};
  bool* initialized[] = {&ina219_0_initialized, &ina219_1_initialized, &ina219_2_initialized};
  const char* topics[] = {HA_MQTT_TOPIC_INA219_0,HA_MQTT_TOPIC_INA219_1,HA_MQTT_TOPIC_INA219_2};
  
  for (int sensorNum = 0; sensorNum < 3; sensorNum++) {
    if (!*initialized[sensorNum]) {
      *initialized[sensorNum] = sensors[sensorNum]->begin();
      if (!*initialized[sensorNum]) {
        Serial.print("Sensor ");
        Serial.print(sensorNum);
        Serial.println(" failed to initialize in loop.");
        continue;
      }
    }   
    publishSensorData(sensors[sensorNum], topics[sensorNum], *initialized[sensorNum]);
  }
  
  delay(10000); // Delay for 1 second 60000
}
