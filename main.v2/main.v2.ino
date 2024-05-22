#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>

// Define DHT11 sensor pin and type
#define DHTPIN 5
#define DHTTYPE DHT11

const int soilSensorPin = A0; 
const int fanPinA = 13;   
const int fanPinB = 14;       
const int motorPin = 12;       

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "HUAWEI-94XN";
const char* password = "Y95c83Mk";
const char* mqtt_server = "aa5105dc6f4740eb8062eb06e26b0347.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "Iot_plant";
const char* mqtt_password = "Aa123456";

WiFiClientSecure espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);

  if (String(topic) == "fun") {
    if (message == "on") {
      digitalWrite(fanPinA, LOW);
      digitalWrite(fanPinB, HIGH);
      Serial.println("FUN ON");
    } else if (message == "off") {
      digitalWrite(fanPinB, LOW);
      Serial.println("FUN OFF");
    }
  } else if (String(topic) == "watering") {
    if (message == "on") {
      digitalWrite(motorPin, HIGH);
      Serial.println("MOTOR ON");
      delay(5000); 
      digitalWrite(motorPin, LOW);
      Serial.println("MOTOR OFF");
    } 
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("D");
      client.subscribe("D_t");
      client.subscribe("lighting");
      client.subscribe("auto_watering");
      client.subscribe("fun");
      client.subscribe("watering");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  espClient.setInsecure();  // For testing purposes, not recommended for production

  pinMode(fanPinA, OUTPUT);
  pinMode(fanPinB, OUTPUT);
  pinMode(motorPin, OUTPUT);
  digitalWrite(fanPinA, LOW);
  digitalWrite(fanPinB, LOW);
  digitalWrite(motorPin, LOW);
  dht.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();  
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  int soilMoistureValue = analogRead(soilSensorPin);
  soilMoistureValue = map(soilMoistureValue, 1023, 0, 0, 100);
  int lightValue = analogRead(A0);
  lightValue = map(lightValue, 1023, 0, 0, 100);

  //Serial.print("Humidity: ");
  //Serial.print(h);
  //Serial.print(" %\t");
  //Serial.print("Temperature: ");
  //Serial.print(t);
  //Serial.print(" *C\t");
  //Serial.print("Soil Moisture: ");
  //Serial.print(soilMoistureValue);
  //Serial.print(" %\t");
  //Serial.print("Light Level: ");
  //Serial.println(lightValue);

  char msg[50];
  snprintf(msg, 50, "%.2f", h);
  //client.publish("D", msg);
  snprintf(msg, 50, "%.2f", t);
  //client.publish("D_t", msg);
  snprintf(msg, 50, "%d", lightValue);
  //client.publish("lighting", msg);
  snprintf(msg, 50, "%d", soilMoistureValue);
  //client.publish("auto_watering", msg);

  delay(1000); 
}
