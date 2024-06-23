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
void publishMessage(const char* text){
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
  }else if (topicStr == RESTART_TOPIC){
    publishMessage("Restart command received, hope to see you soon!");
    delay(750);
    ESP.restart();
  }
}

void publishDiscoveryConfig() {
  const char* base_topic = "home/bedsensor";
  const char* device_identifiers = "bed_sensor";
  const char* device_name = "ESP Bed Sensor";
  const char* device_model = "V1.0";
  const char* device_manufacturer = "Chris Tox";

  // Weight sensor configuration
  JsonDocument weightConfig;
  weightConfig["~"] = base_topic;
  weightConfig["name"] = "Bed Sensor Weight";
  weightConfig["availability_topic"] = String(base_topic) + "/availability";
  weightConfig["state_topic"] = String(base_topic) + "/value";
  weightConfig["unit_of_measurement"] = "kg";
  weightConfig["value_template"] = "{{ value }}";
  weightConfig["unique_id"] = "bed_sensor_weight";
  weightConfig["device_class"] = "weight";
  weightConfig["device"]["identifiers"][0] = device_identifiers;
  weightConfig["device"]["name"] = device_name;
  weightConfig["device"]["model"] = device_model;
  weightConfig["device"]["manufacturer"] = device_manufacturer;

  // Raw values configuration
  JsonDocument rawConfig;
  rawConfig["~"] = base_topic;
  rawConfig["name"] = "Bed Sensor Raw";
  rawConfig["availability_topic"] = String(base_topic) + "/availability";
  rawConfig["state_topic"] = String(base_topic) + "/raw";
  rawConfig["unique_id"] = "bed_sensor_raw";
  rawConfig["device"]["identifiers"][0] = device_identifiers;
  rawConfig["device"]["name"] = device_name;
  rawConfig["device"]["model"] = device_model;
  rawConfig["device"]["manufacturer"] = device_manufacturer;

  // Tare command configuration
  JsonDocument tareConfig;
  tareConfig["~"] = base_topic;
  tareConfig["name"] = "Bed Sensor Tare";
  tareConfig["command_topic"] = String(base_topic) + "/set/tare";
  tareConfig["availability_topic"] = String(base_topic) + "/availability";
  tareConfig["unique_id"] = "bed_sensor_tare";
  tareConfig["device"]["identifiers"][0] = device_identifiers;
  tareConfig["device"]["name"] = device_name;
  tareConfig["device"]["model"] = device_model;
  tareConfig["device"]["manufacturer"] = device_manufacturer;

  // Calibration command configuration
  JsonDocument calibrationConfig;
  calibrationConfig["~"] = base_topic;
  calibrationConfig["name"] = "Bed Sensor Calibration";
  calibrationConfig["command_topic"] = String(base_topic) + "/set/calibration";
  calibrationConfig["availability_topic"] = String(base_topic) + "/availability";
  calibrationConfig["unique_id"] = "bed_sensor_calibration";
  calibrationConfig["device"]["identifiers"][0] = device_identifiers;
  calibrationConfig["device"]["name"] = device_name;
  calibrationConfig["device"]["model"] = device_model;
  calibrationConfig["device"]["manufacturer"] = device_manufacturer;

  // Start calibration command configuration
  JsonDocument startCalibrationConfig;
  startCalibrationConfig["~"] = base_topic;
  startCalibrationConfig["name"] = "Bed Sensor Start Calibration";
  startCalibrationConfig["command_topic"] = String(base_topic) + "/set/start";
  startCalibrationConfig["availability_topic"] = String(base_topic) + "/availability";
  startCalibrationConfig["unique_id"] = "bed_sensor_start_calibration";
  startCalibrationConfig["device"]["identifiers"][0] = device_identifiers;
  startCalibrationConfig["device"]["name"] = device_name;
  startCalibrationConfig["device"]["model"] = device_model;
  startCalibrationConfig["device"]["manufacturer"] = device_manufacturer;
  
  // Restart command configuration
  JsonDocument restartConfig;
  restartConfig["~"] = base_topic;
  restartConfig["name"] = "Bed Sensor Restart";
  restartConfig["command_topic"] = String(base_topic) + "/restart";
  restartConfig["availability_topic"] = String(base_topic) + "/availability";
  restartConfig["unique_id"] = "bed_sensor_restart";
  restartConfig["device"]["identifiers"][0] = device_identifiers;
  restartConfig["device"]["name"] = device_name;
  restartConfig["device"]["model"] = device_model;
  restartConfig["device"]["manufacturer"] = device_manufacturer;

  // Message status configuration
  JsonDocument messageConfig;
  messageConfig["~"] = base_topic;
  messageConfig["name"] = "Bed Sensor Message";
  messageConfig["state_topic"] = String(base_topic) + "/message";
  messageConfig["availability_topic"] = String(base_topic) + "/availability";
  messageConfig["unique_id"] = "bed_sensor_message";
  messageConfig["device"]["identifiers"][0] = device_identifiers;
  messageConfig["device"]["name"] = device_name;
  messageConfig["device"]["model"] = device_model;
  messageConfig["device"]["manufacturer"] = device_manufacturer;


  // // Serialize the JSON documents to strings
  char weightBuffer[1024];
  char rawBuffer[1024];
  char tareBuffer[1024];
  char calibrationBuffer[1024];
  char startCalibrationBuffer[1024];
  char restartBuffer[1024];
  char messageBuffer[1024];

  size_t weightSize = serializeJson(weightConfig, weightBuffer);
  size_t rawSize = serializeJson(rawConfig, rawBuffer);
  size_t tareSize = serializeJson(tareConfig, tareBuffer);
  size_t calibrationSize = serializeJson(calibrationConfig, calibrationBuffer);
  size_t startCalibrationSize = serializeJson(startCalibrationConfig, startCalibrationBuffer);
  size_t restartSize = serializeJson(restartConfig, restartBuffer);
  size_t messageSize = serializeJson(messageConfig, messageBuffer);

  // // Publish the configurations to Home Assistant's discovery topic
  mqtt.publish("homeassistant/sensor/bedsensor_weight/config", weightBuffer, true);
  mqtt.publish("homeassistant/sensor/bedsensor_raw/config", rawBuffer, true);
  mqtt.publish("homeassistant/button/bedsensor_tare/config", tareBuffer, true);
  mqtt.publish("homeassistant/button/bedsensor_calibration/config", calibrationBuffer, true);
  mqtt.publish("homeassistant/button/bedsensor_start_calibration/config", startCalibrationBuffer, true);
  mqtt.publish("homeassistant/button/bedsensor_restart/config", restartBuffer, true);
  mqtt.publish("homeassistant/sensor/bedsensor_message/config", messageBuffer, true);
}

// Init MQTT
void initMQTT(){
  mqtt.setServer(MQTT_SERVER, 1883);
  mqtt.setCallback(callback);
  mqtt.setBufferSize(1024);
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
      mqtt.subscribe(RESTART_TOPIC);
      publishDiscoveryConfig();
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
