#include "DHT.h"
#include <PubSubClient.h>
#include <WiFi.h>

// WiFi 设置
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";
const char* mqtt_server = "mqtt_broker_ip";

WiFiClient espClient;
PubSubClient client(espClient);

// 溫溼度監測器
#define DHTPIN 9 
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define fan   1

// 濕度監測器&自動給水
#define moisture A0
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

  // 根据主题进行不同的操作
  if (strcmp(topic, "auto_watering") == 0) {
    // 将湿度值发布到 MQTT 主题
    float humidity = dht.readHumidity();
    String humidityStr = String(humidity);
    client.publish("auto_watering/humidity", humidityStr.c_str());
  } else if (strcmp(topic, "D") == 0) {
    // 将温湿度值发布到 MQTT 主题
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    String data = String(humidity) + "," + String(temperature);
    client.publish("D/humidity_temperature", data.c_str());
  } else if (strcmp(topic, "lighting") == 0) {
    // 将光感值发布到 MQTT 主题
    value = analogRead(sensorPin);
    String lightValue = String(value);
    client.publish("lighting/light_intensity", lightValue.c_str());
  }
}

void reconnect() {
  // Loop 直到重新连接
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // 尝试连接
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      // 订阅
      client.subscribe("auto_watering");
      client.subscribe("D");
      client.subscribe("lighting");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // 等待5秒后重试
      delay(5000);
    }
  }
}

void D(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.print(h);
  Serial.print(',');
  Serial.println(t);
  if (h > 30) {
    digitalWrite(fan, HIGH);
  } else {
    digitalWrite(fan, LOW);
  }
}

void auto_watering(){
  int waterLevel = analogRead(moisture);
  Serial.print("Moisture: ");
  Serial.println(waterLevel);
  if(waterLevel <= 500) {
    digitalWrite(relay, HIGH);
    delay(60000);
  } else {
    digitalWrite(relay, LOW);
  }
  delay(100);
}

void lighting(){
  value = analogRead(sensorPin);
  Serial.print("Light intensity: ");
  Serial.println(value);
  if(value < 900) {
    digitalWrite(pinLightRelay, HIGH);
    Serial.println("Power on the Light.");
  } else if(value > 1200) {
    digitalWrite(pinLightRelay, LOW);
    Serial.println("Power off the Light.");
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
  pinMode(sensorPin,  INPUT);
  pinMode(pinLightRelay,  OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  D();
  auto_watering();
  lighting();
}
