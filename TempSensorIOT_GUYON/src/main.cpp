#include <Arduino.h>
#include "internalTemperature.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoBLE.h>
#include <EEPROM.h>
#include <PubSubClient.h>

#define EEPROM_SIZE 512
#define CONFIG_ADDRESS 0

// Variables de configuration
int tempFreq = 5; // Fréquence d'acquisition de la température en secondes
int connectionFreq = 5; // Fréquence d'envoi des données par WiFi en secondes
int connectionConfig = 1; // Protocole de connexion (1: HTTP, 2: MQTT)


const char *ssid = "Clement";
const char *password = "ClementG69";
const char *serverName = "http://172.20.10.2:3000/";

HTTPClient http;
WiFiClient client;

bool isBLEInitialized;
bool oldDeviceConnected = false;
uint8_t value = 0;
String configuration;

// UUID du service et de la caractéristique
const char SERVICE_UUID[] = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const char CHARACTERISTIC_UUID[] = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

BLEService bleService(SERVICE_UUID);
BLEByteCharacteristic bleCharacteristic(CHARACTERISTIC_UUID, BLERead | BLEWrite | BLENotify);
PubSubClient mqttClient(client);

// Variables pour la communication MQTT
const char* mqttBroker = "test.mosquitto.org";
const int mqttPort = 1883;
const char* mqttClientId = "espClem";
const char* mqttTopic = "ynov-lyon-2023/esp32/in";
const char* espId = "Esp32IdUnique";

void initializeBLE()
{
  if (!BLE.begin())
  {
    Serial.println("starting BLE failed!");
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
  Serial.println("BLE ready to receive data...");
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
    while (!bleCharacteristic.written())
    {
      Serial.println("En attente d'écriture");
      delay(10); // évite une boucle infinie consommant trop de ressources
    }

    value = bleCharacteristic.value();
    bleCharacteristic.writeValue(value);
    value++;
    delay(10);
  }

  BLE.end();
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
    }
    else
    {
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

void readConfigFromEEPROM() {
  EEPROM.begin(EEPROM_SIZE);

  // Lecture de la configuration depuis l'EEPROM
  EEPROM.get(0, tempFreq);
  EEPROM.get(1, connectionConfig);
  EEPROM.get(2, connectionFreq);

  EEPROM.end();
}

// Méthode pour sauvegarder la configuration dans la mémoire EPROM
void saveConfigToEEPROMIfEmpty() {
  EEPROM.begin(EEPROM_SIZE);

  // Vérifier si l'EEPROM est vide en lisant le premier octet
  byte firstByte = EEPROM.read(CONFIG_ADDRESS);
  if (firstByte != 0xFF) {
    // L'EEPROM n'est pas vide, la configuration existe déjà
    EEPROM.end();
    return;
  }

  // L'EEPROM est vide, sauvegarde de la configuration
  EEPROM.put(1, tempFreq);
  EEPROM.put(2, connectionConfig);
  EEPROM.put(3, connectionFreq);

  EEPROM.commit();
  EEPROM.end();
}

// Méthode pour envoyer les données via MQTT
void sendDataToMQTT() {
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect(mqttClientId))
    {
      // Construction du message JSON
      DynamicJsonDocument jsonBody(256);
      jsonBody["body"]["config"]["tempFreq"] = EEPROM.get(0, tempFreq);
      jsonBody["body"]["config"]["connectionConfig"] = connectionConfig;
      jsonBody["body"]["config"]["connectionFreq"] = connectionFreq;
      jsonBody["temperature"] = readTemp2(false);
      jsonBody["id"] = espId;
      String message;
      serializeJson(jsonBody, message);
      Serial.println(message.c_str());
      mqttClient.publish(mqttTopic, message.c_str());
        mqttClient.beginPublish(mqttTopic, 1, false);
          mqttClient.endPublish();
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("------------------------");
  configuration = getConfiguration();
  Serial.println("------------------------");

  delay(300);
}

void loop()
{
  float temp = readTemp2();

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
      Serial.println("BLE initialization");
      initializeBLE();
    }
    else
    {
      Serial.println("BLE already initialized");
    }
    receiveValueBLE();
  }

  esp_sleep_enable_timer_wakeup(5000000);
  esp_light_sleep_start();
}