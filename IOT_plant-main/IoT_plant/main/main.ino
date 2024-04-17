#include "DHT.h"
#include "Timer.h"
//溫溼度監測器
#define DHTPIN 9 
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define fan   1
//溫溼度監測器
//濕度監測器
#define moisture A0
#define  relay   15
//濕度監測器&自動給水
//感光元件&植物燈
int sensorPin = 2;
int value = 0;
#define pinLightRelay 3
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //溫溼度監視器
  dht.begin();  //初始化DHT
  //濕度監測器&自動給水
  pinMode(moisture, INPUT);
  pinMode(relay, OUTPUT);
  pinMode(fan,  OUTPUT);
  pinMode(pinLightRelay,  OUPUT);
}
void D(){
  delay(1000);
  float h = dht.readHumidity();   //取得濕度
  float t = dht.readTemperature();  //取得溫度C

  //顯示在監控視窗裡
  Serial.print(h);
  Serial.print(',');
  Serial.print(t);
  if (h>30){
    digitalWrite(fan, HIGH);
  }
  else{
    digitalWrite(fan, LOW);
  }
}
void auto_watering(){
  int waterLevel = analogRead(moisture);
  Serial.print(',');
  Serial.println(waterLevel);
  if(waterLevel <= 500) {
    digitalWrite(relay, HIGH);
    delay(60000);
  }
  else {
    digitalWrite(relay, LOW);
  }
  delay(100);
}
void lighting(){
  value = analogRead(sensorPin);
  Serial.print(',');
  Serial.println(value, DEC);
  delay(50);
  //遠端控制
  if(value<900) { //未完成，要接收訊號

    digitalWrite(pinLightRelay, HIGH);

    Serial.print("Power on the Light.");

  }else if(value>1200) {//未完成，要接收訊號

    digitalWrite(pinLightRelay, LOW);

    Serial.print("Power off the Light.");

  }
}

void loop() {
  // put your main code here, to run repeatedly:
  D();
  auto_watering();
  lighting();
}
