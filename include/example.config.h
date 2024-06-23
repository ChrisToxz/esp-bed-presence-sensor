// WiFi
#define SSID                    "your_ssid"
#define PASSWORD                "your_wifi_pass"
#define OTA_PASS                "your_ota_pass"

// MQTT
#define MQTT_SERVER             "your_mqtt_server"
#define MQTT_PORT               "1883"
#define MQTT_USER               "mqtt"
#define MQTT_PASS               "password"
#define MQTT_HOSTNAME           "esp-bed-presence-sensor"

// MQTT Topics
#define AVAILABILITY_TOPIC      "home/bedsensor/available"
#define VALUE_TOPIC             "home/bedsensor/value"
#define RAW_TOPIC               "home/bedsensor/raw"

#define CALIBRATION_TOPIC       "home/bedsensor/calibration"
#define SET_TARE_TOPIC          "home/bedsensor/set/tare"
#define SET_CALIBRATION_TOPIC   "home/bedsensor/set/calibration"

const int HX711_dout    = 0;
const int HX711_sck     = 2;
const int samples       = 8;