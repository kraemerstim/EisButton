#include <ArduinoJson.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "ConnectionDetails.h"

WiFiClient client;

void wifiConnect() {
  Serial.print("Connecting to AP");
  WiFi.begin(AP_SSID, AP_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("Connected");
}

void setup() {
  Serial.begin(9600);
  
  Serial.println("starting");
  wifiConnect();
  String current_time = get_current_time();
  Serial.println(current_time);
  send_hipchat_notification(current_time);
  WiFi.disconnect();
  Serial.println("disconnected");
  ESP.deepSleep(0);
}

String get_current_time() {
  if (WiFi.status() != WL_CONNECTED) {
    return "Fail!";
  }
  HTTPClient http;
  http.begin("http://worldclockapi.com/api/json/cet/now");
  
  int httpCode = http.GET();  
  Serial.println(httpCode);
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    response = response.substring(41, 46);
    return response;
  }
  http.end();
}

void send_hipchat_notification(String text) {
  HTTPClient http;
  char buf[256];
  snprintf(buf, sizeof buf, "https://cassoftware.hipchat.com/v2/room/%s/notification?auth_token=%s", HIPCHAT_CHANNEL, HIPCHAT_TOKEN);
  http.begin(buf, HIPCHAT_THUMBPRINT);
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonBuffer jsonBuffer(400);
  JsonObject& root = jsonBuffer.createObject();
  root["message"] = text;
  root["notify"] = "true";
  root["message_format"] = "text";
  root["color"] = "random";
  String buffer;
  root.printTo(buffer);

  int httpCode = 0;
  int try_count = 3;
  while (httpCode < 200 || httpCode >= 300)
  {
    httpCode = http.POST(buffer);  
    Serial.println(httpCode);
    if(httpCode >= 200 && httpCode < 300) {
      Serial.print("HTTP response code ");
      Serial.println(httpCode);
      String response = http.getString();
      Serial.println(response);
    }
  }
  http.end();
}

void loop() {
  
}

