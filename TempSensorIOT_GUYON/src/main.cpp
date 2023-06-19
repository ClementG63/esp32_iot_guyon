#include <Arduino.h>
#include "internalTemperature.h"
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Telephone de Clement";
const char* password = "ClementG69";
const char* serverName = "http://172.20.10.3:3000/";

void sendTemperature(float temperature);
void sendTemperature(float temperature) {
  HTTPClient http;
  WiFiClient client;

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
}

void setup() {
  Serial.begin(9600);

    WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial.print("Connected to ");
    Serial.println(ssid);
  }
}

void loop() {
  float temp = readTemp2();
  sendTemperature(temp);
  delay(5000);
}
