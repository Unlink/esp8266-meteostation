#include <stdlib.h>
#include <math.h> 

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>


#include <OneWire.h>
#include <DallasTemperature.h>

#include <DHT.h>

#include <Adafruit_MCP3008.h>

#include "rtc_memory.h"

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.print( __VA_ARGS__ )
#define DEBUG_MSG_LN(...) DEBUG_ESP_PORT.println( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#define DEBUG_MSG_LN(...) 
#endif

#define UPDATE_INTERVAL   60000
#define MEASURMENT_CONUT  10
#define SENSOR_NAME       "meteostanica"

#define DHTPIN        5
#define DHTTYPE       DHT11 
#define ONE_WIRE_BUS  4
#define ANALOG_ENABLE  15

char ssid[32] = "YOUR_WIFI_SSID";
char password[32] = "YOUR_WIFI_PASSWORD";

const char* endpointDomain = "YOUR_ENDPOINT_URL";
const char* endpointPath = "/sensors/postMulti";

ADC_MODE(ADC_VCC);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

WiFiClient espClient;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

DHT dht(DHTPIN, DHTTYPE);

Adafruit_MCP3008 adc;

//28bytes
struct Measurment {
  float temp;
  float temp2;
  float humidity;
  float vdd;
  float sun;
  float rain;
  unsigned long time;
};

//MEASURMENT_CONUT * 28 (MEASURMENT_CONUT == 15) => 
//8 + 420 bytes
struct Measurment_Data {
  int count;
  int timestamp;
  Measurment measurments[MEASURMENT_CONUT];
};


RtcData allRtcData;
Measurment_Data* rtc_measurements = (Measurment_Data*) &allRtcData.data;
unsigned long executionBegining;
