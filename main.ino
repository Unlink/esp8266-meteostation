void connectToWifi()
{
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DEBUG_MSG(".");
  }

  DEBUG_MSG_LN("");
  DEBUG_MSG_LN("WiFi connected");  
  DEBUG_MSG_LN("IP address: ");
  DEBUG_MSG_LN(WiFi.localIP());
  timeClient.begin();
}

void loadRTC()
{
  if (!readRtcMemory(&allRtcData)) {
    rtc_measurements->count = 0;
    rtc_measurements->timestamp = 0;
  }
}

void loadNTPTime()
{
  connectToWifi();
  while (timeClient.getEpochTime() < 1000000) {
    DEBUG_MSG_LN("Updating time");
    DEBUG_MSG_LN(timeClient.getFormattedTime());
    timeClient.update();
  }
  DEBUG_MSG_LN(timeClient.getFormattedTime());
}

void uploadData()
{
    DEBUG_MSG_LN("Uploading data");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["sensor"] = SENSOR_NAME;
    JsonArray& data = root.createNestedArray("data");
    
    for (int i=0; i<rtc_measurements->count; i++) {
      Measurment measurment = rtc_measurements->measurments[i];
      JsonObject& record = data.createNestedObject();
      record["temp"] = measurment.temp;
      record["temp2"] = measurment.temp2;
      record["humidity"] = measurment.humidity;
      record["time"] = measurment.time;
      record["sun"] = measurment.sun;
      record["rain"] = measurment.rain;
      record["vdd"] = measurment.vdd;
    }
    String output;
    root.printTo(output);
    DEBUG_MSG_LN(output);

    connectToWifi();
    int repeatCount = 5;
    while (repeatCount > 0) {
      HTTPClient http;
      http.begin(endpointDomain, 80, endpointPath);
      http.addHeader("Content-Type", "application/json");
      int httpCode = http.POST(output);
      String payload = http.getString();
      DEBUG_MSG_LN(httpCode);
      DEBUG_MSG_LN(payload);
      http.end();
      payload.trim();
      if (payload == "OK") {
        break;
      }
      repeatCount--;
      delay(500);
    }
}

/**
 * Esp Setup
 */
void setup()
{
  Serial.begin(115200);
  DEBUG_MSG_LN();
  loadRTC();

  if (rtc_measurements->timestamp == 0) {
    rtc_measurements->count = 0;
    loadNTPTime();
    rtc_measurements->timestamp = timeClient.getEpochTime() - (millis() / 1000);
  }
  else {
    rtc_measurements->timestamp += (UPDATE_INTERVAL/1000);
  }

  DEBUG_MSG("Measurments: ");
  DEBUG_MSG_LN(rtc_measurements->count);

  Measurment measurment;

  //Teplota
  int cunter = 0;
  do {
    DS18B20.requestTemperatures(); 
    measurment.temp = DS18B20.getTempCByIndex(0);
    yield();
    cunter++;
  } while ((measurment.temp == 85.0 || measurment.temp == (-127.0)) && cunter < 5);
  DEBUG_MSG("Temp Dalas: ");
  DEBUG_MSG_LN(measurment.temp);


  //Vlhkomer
  dht.begin();
  measurment.humidity = dht.readHumidity();
  measurment.temp2 = dht.readTemperature();
  DEBUG_MSG("Temp DHT: ");
  DEBUG_MSG_LN(measurment.temp2);
  DEBUG_MSG("Humidity DHT: ");
  DEBUG_MSG_LN(measurment.humidity);

  pinMode(ANALOG_ENABLE, OUTPUT);
  digitalWrite(ANALOG_ENABLE, HIGH);
  //Slnečnosť na adc0
  adc.begin(14, 13, 12, 0);
  measurment.sun = adc.readADC(0);
  DEBUG_MSG("Sun: ");
  DEBUG_MSG_LN((measurment.sun / 1024)* 100);

  //Dážď
  measurment.rain = adc.readADC(1);
  DEBUG_MSG("Rain: ");
  DEBUG_MSG_LN((measurment.rain / 1024)* 100);
  digitalWrite(ANALOG_ENABLE, LOW);

  //Batéria
  DEBUG_MSG("Power: ");
  measurment.vdd = ESP.getVcc() / 1000.0;
  DEBUG_MSG_LN(measurment.vdd);

  measurment.time = rtc_measurements->timestamp;
  
  rtc_measurements->measurments[rtc_measurements->count++] = measurment;
  
  if (rtc_measurements->count == MEASURMENT_CONUT) {
    uploadData();
    loadNTPTime();
    rtc_measurements->timestamp = timeClient.getEpochTime() - (millis()/1000);
    rtc_measurements->count = 0;
    DEBUG_MSG_LN("Upload Done");
  }
  
  writeRtcMemory(&allRtcData);
  WiFi.disconnect();
  ESP.deepSleep((UPDATE_INTERVAL - millis()) * 1000);
}


void loop()
{
}
