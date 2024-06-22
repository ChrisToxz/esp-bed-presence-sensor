bool ledState = false;


void initWiFi(){
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("ESP-BED-SENSOR");
  WiFi.begin(SSID, PASSWORD);

  unsigned long startAttemptTime = millis();
  unsigned long timeout = 60000;

  while (WiFi.status() != WL_CONNECTED) {
    // Restart ESP if it takes too long
    if (millis() - startAttemptTime > timeout) {
      Serial.println("WiFi timeout! Restarting");
        ESP.restart();
    }
    ledState = !ledState;             // Toggle the LED state
    digitalWrite(LED_BUILTIN, ledState);   // Set the LED
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP:");
  Serial.print(WiFi.localIP());
}

void initOTA(){

  ArduinoOTA.setPassword(OTA_PASS);
  
  ArduinoOTA.onStart([]() {
    Serial.println("Starting OTA!");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnding OTA!");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void resetLED(){
  // Reset LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}