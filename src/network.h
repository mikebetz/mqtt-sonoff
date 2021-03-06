void startOTA() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
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

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}



void setup_wifi() {

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //reset saved settings
  //wifiManager.resetSettings();

  //set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  wifiManager.setDebugOutput(true);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_device_role("role", "role FAN or SWITCH", device_role, 10);
  WiFiManagerParameter custom_mqtt_nickname("nickname", "mqtt nickname", mqtt_nickname, 20);

  WiFiManagerParameter custom_pinLED("LEDpin", "LED pin 13", ch_pinLED, 3);
  WiFiManagerParameter custom_pinRelay("Relaypin", "Relay pin 12", ch_pinRelay, 3);
  WiFiManagerParameter custom_pinButton("ButtonPin", "Button pin 0", ch_pinButton, 3);

  WiFiManagerParameter custom_pinTemperature("TemperaturePin", "Temp Sensor Pin 14", ch_pinTemperature, 3);
  WiFiManagerParameter custom_DHTType("DHTType", "DHTType 11 21 22", ch_DHTType, 3);

  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_device_role);
  wifiManager.addParameter(&custom_mqtt_nickname);

  wifiManager.addParameter(&custom_pinLED);
  wifiManager.addParameter(&custom_pinRelay);
  wifiManager.addParameter(&custom_pinButton);

  wifiManager.addParameter(&custom_pinTemperature);
  wifiManager.addParameter(&custom_DHTType);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  //wifiManager.autoConnect("AutoConnectAP","antwerp");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect, reset");
    delay(5000);
    ESP.reset();
    delay(2000);
    while (1) {
      // spin until reset
    };
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  /*
  // old code that grabbed the MAC address
  char justabyte[3];
  byte mac[6];
  char *a;
  WiFi.macAddress(mac);

  int i;

  a = &macaddr[0];

  for (i = 0; i<6; i++) {
    sprintf(a, "%02X", mac[i]);
    a++;
    a++;
    *a++ = ':';
  }

  a--;
  *a = 0;

  Serial.print("MAC: ");
  Serial.println(macaddr);
  */

  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(device_role, custom_device_role.getValue());
  strcpy(mqtt_nickname, custom_mqtt_nickname.getValue());

  strcpy(ch_pinLED, custom_pinLED.getValue());
  strcpy(ch_pinRelay, custom_pinRelay.getValue());
  strcpy(ch_pinButton, custom_pinButton.getValue());

  strcpy(ch_pinTemperature, custom_pinTemperature.getValue());
  strcpy(ch_DHTType, custom_DHTType.getValue());

  Serial.print("server is now ");
  Serial.println(mqtt_server);

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["device_role"] = device_role;
    json["mqtt_nickname"] = mqtt_nickname;

    json["pinLED"] = ch_pinLED;
    json["pinRelay"] = ch_pinRelay;
    json["pinButton"] = ch_pinButton;

    json["pinTemperature"] = ch_pinTemperature;
    json["DHTType"] = ch_DHTType;

  ////    json["blynk_token"] = blynk_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

// finish naming the mqtt topics
// NICKNAME-whatever

  strcpy(POWER_FEED, mqtt_nickname);
  strcat(POWER_FEED, "-");
  strcat(POWER_FEED, POWER_FEED_SUFFIX);

  strcpy(TEMPERATURE_FEED, mqtt_nickname);
  strcat(TEMPERATURE_FEED, "-");
  strcat(TEMPERATURE_FEED, TEMPERATURE_FEED_SUFFIX);

  strcpy(HUMIDITY_FEED, mqtt_nickname);
  strcat(HUMIDITY_FEED, "-");
  strcat(HUMIDITY_FEED, HUMIDITY_FEED_SUFFIX);

  setFeaturesFromRole();

  startOTA();

}
