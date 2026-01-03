unsigned long previousMillisWiFi = 0;
long intervalWiFi = 6000; 


#include "WiFi.h"
#include "esp_wps.h"

#define ESP_WPS_MODE      WPS_TYPE_PBC
#define ESP_MANUFACTURER  "XIAO"
#define ESP_MODEL_NUMBER  "ESP32S3"
#define ESP_MODEL_NAME    "SEED STUDIO"
#define ESP_DEVICE_NAME   "Walllamp"

static esp_wps_config_t config;

void wpsInitConfig() {
  config.wps_type = ESP_WPS_MODE;
  strcpy(config.factory_info.manufacturer, ESP_MANUFACTURER);
  strcpy(config.factory_info.model_number, ESP_MODEL_NUMBER);
  strcpy(config.factory_info.model_name, ESP_MODEL_NAME);
  strcpy(config.factory_info.device_name, ESP_DEVICE_NAME);
  strcpy(config.pin, "00000000");
}

String wpspin2string(uint8_t a[]) {
  char wps_pin[9];
  for (int i = 0; i < 8; i++) {
    wps_pin[i] = a[i];
  }
  wps_pin[8] = '\0';
  return (String)wps_pin;
}

void WiFiEvent(WiFiEvent_t event, arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START: Serial.println("Station Mode Started"); break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.println("Connected to :" + String(WiFi.SSID()));
      Serial.print("Got IP: ");
      Serial.println(WiFi.localIP());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("Disconnected from station, attempting reconnection");
      WiFi.reconnect();
      break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:
      Serial.println("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
      esp_wifi_wps_disable();
      delay(10);
      WiFi.begin();
      break;
    case ARDUINO_EVENT_WPS_ER_FAILED:
      Serial.println("WPS Failed, retrying");
      esp_wifi_wps_disable();
      esp_wifi_wps_enable(&config);
      esp_wifi_wps_start(0);
      break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:
      Serial.println("WPS Timedout, retrying");
      esp_wifi_wps_disable();
      esp_wifi_wps_enable(&config);
      esp_wifi_wps_start(0);
      break;
    case ARDUINO_EVENT_WPS_ER_PIN: Serial.println("WPS_PIN = " + wpspin2string(info.wps_er_pin.pin_code)); break;
    default:                       break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);


  Serial.println();
  Serial.println("Wifi Connector");
  WiFi.setHostname("Walllamp");
  WiFi.begin();

  Serial.print("Try Wifi Connect");

  while (WiFi.status() != WL_CONNECTED) {
    if (previousMillisWiFi < intervalWiFi)
    {
      previousMillisWiFi = millis();
      Serial.print(".");


      delay(500);
    }
    else break;

  }

  if (WiFi.status() != WL_CONNECTED) {

    WiFi.onEvent(WiFiEvent);
    WiFi.mode(WIFI_MODE_STA);

    Serial.println("Not Successful. Starting WPS");

    wpsInitConfig();
    esp_wifi_wps_enable(&config);
    esp_wifi_wps_start(0);
  }
  else {
    Serial.println();
    Serial.print("Verbunden mit: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

void loop() {
  //nothing to do here
delay(10);
}