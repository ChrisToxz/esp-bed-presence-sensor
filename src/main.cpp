#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <HX711_ADC.h>
#include <ArduinoJson.h>
#include "../include/config.h"

WiFiClient    espClient;
PubSubClient  mqtt(espClient);
HX711_ADC     LoadCell(HX711_dout, HX711_sck);

#include "helpers.h"

void setup() {
  Serial.begin(115200); delay(10);
  Serial.println("Starting up");

  EEPROM.begin(4);
  
  resetLED();
  initWiFi();
  initOTA();
  initMQTT();
  initLoadCell();
}

void loop() {
  delay(200);
  static boolean newDataReady = 0;
  // LoadCell.powerUp();
  if (LoadCell.update()) newDataReady = true;

  if (newDataReady) {
    float i = LoadCell.getData();
    Serial.print("Load_cell output val: ");
    Serial.println(i);
    newDataReady = 0;

    mqtt.publish(VALUE_TOPIC, String(i/1000).c_str()); // Publish calibration value
    mqtt.publish(RAW_TOPIC, String(i).c_str()); // Publish calibration value
  }

  // LoadCell.powerDown();
  handleLoop();
}
