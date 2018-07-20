#include <ArduinoJson.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "ConnectionDetails.h"
WiFiClient client;

int inputPin = D2;
int ledPin = D4;
int buttonState = 0;

void wifiConnect() {
  Serial.print("Connecting to AP");
  WiFi.begin(AP_SSID, AP_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected");
}

void setup() {
  Serial.begin(9600);
  pinMode(0,1);
  Serial.begin(9600);
  delay(300);
  pinMode(inputPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.println("starting");
  wifiConnect();
}

String get_current_time() {
  if (WiFi.status() != WL_CONNECTED) {
    //WIFI Muss verbunden sein!
    return "00:00";
  }
  HTTPClient http;
  http.begin("http://worldclockapi.com/api/json/cet/now");
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    response = response.substring(41, 46);
    return response;
  }
}

int getTotalMinutes(const char* timeToTransform) {
  int hh, mm, minutes;
  char s[255];
  sscanf(timeToTransform, "%d:%d",&hh,&mm);
  minutes = hh*60+mm;
  sprintf(s, "time: %s minutes: %d", timeToTransform, minutes);
  Serial.println(s);
  return minutes;
}

boolean isValidTime(const char* currentTime) {
  int cur, first, last;
  cur = getTotalMinutes(currentTime);
  first = getTotalMinutes(ACTIVATE_TIME);
  last = getTotalMinutes(DEACTIVATE_TIME);

  if (cur > last || cur < first) {
    return false;
  }
  return true;
}

void send_hipchat_notification(boolean isIceTime) {
  HTTPClient http;
  char buf[256];
  if (isIceTime) {
    snprintf(buf, sizeof buf, "https://cassoftware.hipchat.com/v2/room/%s/notification?auth_token=%s", HIPCHAT_CHANNEL_EIS, HIPCHAT_TOKEN_EIS);
  } else {
    snprintf(buf, sizeof buf, "https://cassoftware.hipchat.com/v2/room/%s/notification?auth_token=%s", HIPCHAT_CHANNEL_TEST, HIPCHAT_TOKEN_TEST);
  }
  http.begin(buf, HIPCHAT_THUMBPRINT);
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonBuffer jsonBuffer(400);
  JsonObject& root = jsonBuffer.createObject();
  root["message"] = "(eismann)";
  root["notify"] = "true";
  root["message_format"] = "text";
  root["color"] = "random";
  String buffer;
  root.printTo(buffer);

  int httpCode =http.POST(buffer);
  
  Serial.println(httpCode);
  if(httpCode == HTTP_CODE_OK) {
    Serial.print("HTTP response code ");
    Serial.println(httpCode);
    String response = http.getString();
    Serial.println(response);
  }
  http.end();
}

void loop() {
  buttonState = digitalRead(inputPin);
  if (buttonState == 1)
  {
    digitalWrite(ledPin, LOW); 
    const char* current_time = get_current_time().c_str();
    Serial.println(current_time);
    delay(300);
    send_hipchat_notification(isValidTime(current_time));
    Serial.println("send to Hipchat");
    digitalWrite(ledPin, HIGH); 
    delay(10000);
  }
  if (buttonState == 0)
  {
    delay(50);
  }
}
