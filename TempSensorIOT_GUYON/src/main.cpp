#include <Arduino.h>
#include "internalTemperature.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoBLE.h>

const char *ssid = "Clement";
const char *password = "ClementG69";
const char *serverName = "http://172.20.10.2:3000/";

HTTPClient http;
WiFiClient client;
String configuration;

bool isBLEInitialized;
bool oldDeviceConnected = false;
uint8_t value = 0;

// UUID du service et de la caractéristique
const char SERVICE_UUID[] = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const char CHARACTERISTIC_UUID[] = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

BLEService bleService(SERVICE_UUID);
BLEByteCharacteristic bleCharacteristic(CHARACTERISTIC_UUID, BLERead | BLEWrite | BLENotify);

void initializeBLE()
{
  if (!BLE.begin())
  {
    Serial.println("starting BLE failed!");
    while (1)
      ;
  }

  BLE.setLocalName("ESP32_BLE");
  BLE.setAdvertisedService(bleService);

  bleService.addCharacteristic(bleCharacteristic);
  BLE.addService(bleService);
  bleCharacteristic.writeValue(value);

  BLE.advertise();

  Serial.println("L'ESP32 BLE est prêt à accepter une connexion.");
  isBLEInitialized = true;
}

void receiveValueBLE()
{
  BLEDevice central = BLE.central();

  if (central)
  {
    Serial.print("Nouvel appareil connecté : ");
    Serial.println(central.address());
    oldDeviceConnected = true;
  }

  if (!central.connected() && oldDeviceConnected)
  {
    delay(500);
    Serial.println("Appareil déconnecté.");
    oldDeviceConnected = false;
  }

  if (central.connected())
  {
    // Attendez que les données soient écrites
    while (!bleCharacteristic.written())
    {
      delay(10); // évite une boucle infinie consommant trop de ressources
    }

    value = bleCharacteristic.value();
    bleCharacteristic.writeValue(value);
    value++;
    delay(10);
  }
}


void sendTemperature(float temperature)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("Connected to ");
    Serial.println(ssid);
  }

  String endpoint = "sensors/64904b85fd0ea8b8ee054e35";
  String url = String(serverName);
  String finalUrl = url + endpoint;

  Serial.println(finalUrl);

  http.begin(client, finalUrl);
  http.addHeader("Content-Type", "application/json");

  String httpRequestData = "{\"temperature\":\"";
  httpRequestData += String(temperature);
  httpRequestData += "\"}";

  int httpResponseCode = http.PUT(httpRequestData);

  if (httpResponseCode > 0)
  {
    String response = http.getString();
    Serial.println(httpResponseCode);
  }
  else
  {
    Serial.print("Erreur lors de l'envoi de la requête PUT : ");
    Serial.println(httpResponseCode);
  }
  http.end();

  Serial.println("WiFi disconnected");
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

String getConfiguration()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("Connected to ");
    Serial.println(ssid);
  }

  String endpoint = "configuration";
  String url = String(serverName);
  String finalUrl = url + endpoint;

  Serial.println(finalUrl);

  http.begin(client, finalUrl);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0)
  {
    String response = http.getString();                          // Obtenir la réponse JSON sous forme de chaîne de caractères
    DynamicJsonDocument doc(1024);                               // Créer un document JSON pour stocker la réponse analysée
    DeserializationError error = deserializeJson(doc, response); // Analyser la réponse

    if (error)
    {
      Serial.print(F("deserializeJson() a échoué : "));
      return error.f_str();
    }
    
    Serial.println(response);
    if (doc.containsKey("type"))
    {
      const char *sensor = doc["type"];
      String sensorStr = String(sensor);

      Serial.print("CONFIGURATION GET: ");
      Serial.println(sensor);
      return String(sensor);
    } else {
      Serial.println("Le champ TYPE n'a pas été trouvé");
    }

    return "Error";
  }
  else
  {
    Serial.print("Erreur lors de l'envoi de la requête GET : ");
    Serial.println(httpResponseCode);
    return "";
  }

  http.end();

  Serial.println("WiFi disconnected");
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

void setup()
{
  Serial.begin(9600);
  delay(300);
}

void loop()
{
  float temp = readTemp2();
  Serial.println("------------------------");
  configuration = getConfiguration();
  Serial.println("------------------------");

  if (configuration.compareTo("http") == 0)
  {
    Serial.println("Configuration: HTTP");
    sendTemperature(temp);
  }
  else if (configuration.compareTo("mqtt") == 0)
  {
    Serial.println("Configuration: MQTT");
  }
  else if (configuration.compareTo("http") == 0 && configuration.compareTo("mqtt") == 0)
  {
    Serial.println("Configuration: HTTP & MQTT");
    sendTemperature(temp);
  }
  else if (configuration.compareTo("ble") == 0)
  {
    Serial.println("Configuration: BLE");

    if (!isBLEInitialized)
    {
      initializeBLE();
    }
    receiveValueBLE();
  }

  esp_sleep_enable_timer_wakeup(5000000);
  esp_light_sleep_start();
}