bool ledState = false;


// WiFi
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


// Init OTA
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
    mqtt.publish(CALIBRATION_TOPIC, String(value).c_str()); // Publish calibration value
}

float getCalibration() {
    float value;
    EEPROM.get(0, value);
    return value;
}


// Publish MQTT message
void publishMessage(char* text){
  mqtt.publish(MESSAGE_TOPIC, text);
}

// MQTT Callback
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
    LoadCell.tare();
    Serial.println("Tare complete!");
  }else if(topicStr == START_CALIBRATION_TOPIC){
    publishMessage("Calibration started"); delay(500);
    publishMessage("Please remove all load within 5 seconds"); delay(5000);
    publishMessage("Tarring now!");
    LoadCell.tare();
    publishMessage("Tarred!"); delay(500);

    publishMessage("Now place 2 KG as load within 5 seconds"); delay(7000);
    LoadCell.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
    float newCalibrationValue = LoadCell.getNewCalibration(2000); //get the new calibration value
    setCalibration(newCalibrationValue);
    publishMessage("Calibration done!");
  }
}


// Init MQTT
void initMQTT(){
  mqtt.setServer(MQTT_SERVER, 1883);
  mqtt.setCallback(callback);
}


// Connect MQTT
void connectMQTT() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(MQTT_HOSTNAME, MQTT_USER, MQTT_PASS, AVAILABILITY_TOPIC, 2, true, "offline")) {
      Serial.println("MQTT connected!");
      mqtt.publish(AVAILABILITY_TOPIC, "online", true);
      mqtt.publish(CALIBRATION_TOPIC, String(getCalibration()).c_str()); // Publish calibration value
      mqtt.subscribe("home/bedsensor/set/#");
      publishMessage("Im alive!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void initLoadCell(){
  LoadCell.begin();
  LoadCell.start(3000, true);

  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, HX711 not found");
    ESP.restart();
  }else{
    Serial.print("Setting Calibration Factor: ");
    Serial.println(getCalibration());
    LoadCell.setCalFactor(getCalibration()); // set calibration value
    Serial.print("Setting Samples: ");
    Serial.println(samples);
    LoadCell.setSamplesInUse(samples);
    LoadCell.tare();
  }
}

void handleLoop(){
  ArduinoOTA.handle();
  
  if (!mqtt.connected()) {
    connectMQTT();
  }
  mqtt.loop();
}

void resetLED(){
  // Reset LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}
