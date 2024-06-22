#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

#include "config.h"
#include "helpers.h"


void setup() {
  Serial.begin(115200);
  Serial.println("Starting up");

  resetLED();
  initWiFi();
  initOTA();

}

void loop() {
  ArduinoOTA.handle();
}
