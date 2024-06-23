#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <HX711_ADC.h>
#include "config.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);
HX711_ADC LoadCell(HX711_dout, HX711_sck);

#include "helpers.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Starting up");

  EEPROM.begin(4);
  resetLED();

  initWiFi();
  initOTA();
  initMQTT();

  LoadCell.begin();
  LoadCell.start(2000, true);

  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, HX711 not found");
    ESP.restart();
  }else{
    Serial.println(LoadCell.getSamplesInUse());
    Serial.print("Setting Calibration Factor: ");
    Serial.println(getCalibration());
    LoadCell.setCalFactor(getCalibration()); // set calibration value (float)
    LoadCell.setSamplesInUse(samples);
    Serial.println("Startup is complete");
  }
}

void loop() {
  static boolean newDataReady = 0;
  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

 if (newDataReady) {
      float i = LoadCell.getData();
      Serial.print("Load_cell output val: ");
      Serial.println(i);
      newDataReady = 0;
      mqtt.publish(RAW_TOPIC, (char *)String(i).c_str()); // Publish calibration value
    }

  ArduinoOTA.handle();
  
  if (!mqtt.connected()) {
    connectMQTT();
  }
  mqtt.loop();

  mqtt.publish(CALIBRATION_TOPIC, (char *)String(getCalibration()).c_str()); // Publish calibration value


  delay(100);
}
