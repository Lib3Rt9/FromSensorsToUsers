#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include "DHT.h"
#include "SimpleKalmanFilter.h" 

const char* ssid = "*****";
const char* password = "******";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Json Variable to Hold Sensor Readings
JSONVar readings;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

// Define DHT sensor type and pin
#define DHTPIN 4 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11

// Create a DHT sensor object
DHT dht(DHTPIN, DHTTYPE);

// Create Kalman filter objects for temperature and humidity
SimpleKalmanFilter temperatureKalmanFilter(2, 2, 0.01);
SimpleKalmanFilter humidityKalmanFilter(2, 2, 0.01);

// Init DHT11
void initDHT(){
  dht.begin();
}

// Get Sensor Readings and return JSON object
String getSensorReadings(){
  float rawTemperature = dht.readTemperature();
  float rawHumidity = dht.readHumidity();
  // Check if any reads failed and exit early (to try again).
  if (isnan(rawTemperature) || isnan(rawHumidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return "";
  }

  // Apply Kalman filter to raw readings
  float filteredTemperature = temperatureKalmanFilter.updateEstimate(rawTemperature);
  float filteredHumidity = humidityKalmanFilter.updateEstimate(rawHumidity);

  readings["temperature"] = String(filteredTemperature);
  readings["humidity"] =  String(filteredHumidity);
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

// Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  initDHT();
  initWiFi();
  initLittleFS();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getSensorReadings();
    if (json != "") {
      request->send(200, "application/json", json);
    } else {
      request->send(500, "application/json", "{\"error\":\"Failed to read sensor data\"}");
    }
  });

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  // Start server
  server.begin();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Send Events to the client with the Sensor Readings Every 10 seconds
    events.send("ping",NULL,millis());
    String sensorData = getSensorReadings();
    if (sensorData != "") {
      events.send(sensorData.c_str(),"new_readings" ,millis());
    }
    lastTime = millis();
  }
}
