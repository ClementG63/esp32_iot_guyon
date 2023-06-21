#include <Arduino.h>
#include "internalTemperature.h"
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Clement";
const char* password = "ClementG69";
const char* serverName = "http://172.20.10.3:3000/";

HTTPClient http;
WiFiClient client;

void sendTemperature(float temperature);
void sendTemperature(float temperature) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial.print("Connected to ");
    Serial.println(ssid);
  }

  String endpoint = "capteurs/64904b85fd0ea8b8ee054e35";
  String url = String(serverName);
  String finalUrl = url+endpoint;

  Serial.println(finalUrl);

  http.begin(client, finalUrl);
  http.addHeader("Content-Type", "application/json");

  String httpRequestData = "{\"temperature\":\"";
  httpRequestData += String(temperature);
  httpRequestData += "\"}";

  int httpResponseCode = http.PUT(httpRequestData);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Erreur lors de l'envoi de la requête PUT : ");
    Serial.println(httpResponseCode);
  }
  http.end();
  
  Serial.println("WiFi disconnected");
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

void connectToWiFi() {
  Serial.print("Connectiog to ");
 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected.");
  
}

void setup() {
  Serial.begin(9600);
}

void loop() {
  float temp = readTemp2();
  sendTemperature(temp);
  esp_sleep_enable_timer_wakeup(5000000);
  esp_light_sleep_start();
}
