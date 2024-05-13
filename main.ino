#include "DHT.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

// WiFi 设置
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";
const char* mqtt_server = "mqtt_broker_ip";

WiFiClient espClient;
PubSubClient client(espClient);

// 狀態
#define autoPin 4

// 溫溼度監測器
#define DHTPIN 9
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define fan   1

// 濕度監測器&自動給水
#define moisture 7
#define  relay   15

// 感光元件&植物燈
int sensorPin = 2;
int value = 0;
#define pinLightRelay 3

void setup_wifi() {
  delay(10);
  // 连接WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  DynamicJsonDocument doc(128);
  deserializeJson(doc, payload, length);

  // 根据主题进行不同的操作
  if (strcmp(topic, "autoPin") == 0) {
    const char* action = doc["action"];
    if (strcmp(action, "ON") == 0) {
      auto_D();
      auto_watering();
      auto_lighting();
    }
  } else if (strcmp(topic, "FUN") == 0) {
    const char* action = doc["action"];
    if (strcmp(action, "ON") == 0) {
      digitalWrite(fan, HIGH);
    } else if (strcmp(action, "OFF") == 0) {
      digitalWrite(fan, LOW);
    }
  } else if (strcmp(topic, "LIGHT") == 0) {
    const char* action = doc["action"];
    if (strcmp(action, "ON") == 0) {
      digitalWrite(pinLightRelay, HIGH);
    } else if (strcmp(action, "OFF") == 0) {
      digitalWrite(pinLightRelay, LOW);
    }
  } else if (strcmp(topic, "WATER") == 0) {
    const char* action = doc["action"];
    if (strcmp(action, "ON") == 0) {
      digitalWrite(relay, HIGH);
      delay(60000);
    } else if (strcmp(action, "OFF") == 0) {
      digitalWrite(relay, LOW);
    }
  }
}

void reconnect() {
  // Loop 直到重新连接
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // 尝试连接
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // 订阅
      client.subscribe("autoPin");
      client.subscribe("FUN");
      client.subscribe("LIGHT");
      client.subscribe("WATER");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // 等待5秒后重试
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  dht.begin();

  pinMode(moisture, INPUT);
  pinMode(relay, OUTPUT);
  pinMode(fan,  OUTPUT);
  pinMode(pinLightRelay,  OUTPUT);
  pinMode(autoPin,  OUTPUT);
}

void auto_D(){
  delay(1000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.print(h);
  Serial.print(',');
  Serial.print(t);
  if (t > 30) {
    digitalWrite(fan, HIGH);
  } else {
    digitalWrite(fan, LOW);
  }
}

void observe_D(){
  delay(1000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.print(h);
  Serial.print(',');
  Serial.print(t);
}

void auto_watering(){
  int waterLevel = analogRead(moisture);
  Serial.print(',');
  Serial.println(waterLevel);
  if(waterLevel <= 500) {
    digitalWrite(relay, HIGH);
    delay(60000);
  } else {
    digitalWrite(relay, LOW);
  }
  delay(1000);
}

void observe_watering(){
  int waterLevel = analogRead(moisture);
  Serial.print(',');
  Serial.println(waterLevel);
}

void auto_lighting(){
  value = analogRead(sensorPin);
  Serial.print(',');
  Serial.println(value, DEC);
  delay(50);
  if(value < 900) {
    digitalWrite(pinLightRelay, HIGH);
    Serial.print("Power on the Light.");
  } else if(value > 1200) {
    digitalWrite(pinLightRelay, LOW);
    Serial.print("Power off the Light.");
  }
}

void observe_lighting(){
  value = analogRead(sensorPin);
  Serial.print(',');
  Serial.println(value, DEC);
  delay(1000);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
