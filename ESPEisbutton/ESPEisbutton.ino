#include <ArduinoJson.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "ConnectionDetails.h"
WiFiClient client;

int inputPin = D0;
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
  WiFi.disconnect();
}

String get_current_time() {
  if (WiFi.status() != WL_CONNECTED) {
    //WIFI Muss verbunden sein!
    return "Fail!";
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
  if (buttonState == 0)
  {
    digitalWrite(ledPin, LOW); 
    wifiConnect();
    String current_time = get_current_time();
    Serial.println(current_time);
    delay(300);
    send_hipchat_notification(current_time);
    WiFi.disconnect();
    Serial.println("disconnected");
    digitalWrite(ledPin, HIGH); 
    delay(10000);
  }
  if (buttonState == 1)
  {
    delay(50);
  }
}
