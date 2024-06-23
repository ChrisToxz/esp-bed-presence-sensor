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
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());
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


void setCalibration(float value) {
    EEPROM.put(0, value);
    EEPROM.commit(); // Ensure the data is written to EEPROM

    LoadCell.setCalFactor(value);
}

float getCalibration() {
    float value;
    EEPROM.get(0, value);
    return value;
}

void callback(char* topic, byte* payload, unsigned int length) {

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println(topic);

  String topicStr = String(topic);
  if (topicStr == SET_CALIBRATION_TOPIC) {
    Serial.println("Set calibration");
    setCalibration(message.toFloat());
  } else if (topicStr == "home/bed/sensor/set/offset") {
    // Handle offset action
  } else if (topicStr == SET_TARE_TOPIC){
    Serial.println("Taring..");
    LoadCell.tareNoDelay();
    if (LoadCell.getTareStatus() == true) {
      Serial.println("Tare complete");
    }
  }
}

void initMQTT(){
  mqtt.setServer(MQTT_SERVER, 1883);
  mqtt.setCallback(callback);
}

void connectMQTT() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(MQTT_HOSTNAME, MQTT_USER, MQTT_PASS, AVAILABILITY_TOPIC, 2, true, "offline")) {
      Serial.println("connected");
      mqtt.publish(AVAILABILITY_TOPIC, "online", true);
      mqtt.subscribe("home/bedsensor/set/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void resetLED(){
  // Reset LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}
