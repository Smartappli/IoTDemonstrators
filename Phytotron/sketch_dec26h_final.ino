/*
   Phytotron UMons
   Rachida Ait Abelouahid
   Olivier Debauche
   
   MIT License

   Copyright (c) 2017 Sven Henkel
   Multiple units reading by Grega Lebar 2018

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
#include <LiquidCrystal_I2C.h>
#include "DHTesp.h"
#include "Ticker.h"
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h> 
#include <RtcDS3231.h>
#include <PubSubClient.h>
#include "config.h"

// set the LCD number of columns and rows
int lcdColumns = 20;
int lcdRows = 4;
int value;  
int val;
int val2;
int val3;
int val4;
bool boucle;
bool boucle2;
char buffer[64];  
                                                                                                                                                                                                                                                                                
LiquidCrystal_I2C lcd(0x3f, lcdColumns, lcdRows);  
DHTesp dht;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "us.pool.ntp.org", -7*3600);
RtcDS3231<TwoWire> Rtc(Wire);
WiFiClient espClient;
PubSubClient client(espClient);
PubSubClient client2(espClient);

#define fanRelay 23                   // Definition Fan Relay on pin 23
#define pumpRelay 17                  // Definition Pump Relay on pin 17
#define lightRelay1 18                // Definition Light1 Relay on pin 18
#define lightRelay2 19                // Definition Light2 Relay on pin 19
#define sensorPin 36                  // Definition Sensor Pin on pin 36 (GPIO 36)
#define dhtPin 27                     // Definition DHT sensor pin on 27 (GPIO 27)
#define pumpManualPin 34              // Definition pump manual Pin on 24
#define fanManualPin 35               // Definition fan manual Pin on 25

//MQTT
const char* mqtt_topic_fan = "phytotron/fan_manual";
const char* mqtt_topic_pump = "phytotron/pump_manual";
const char* mqtt_topic_light1 = "phytotron/light1_manual";
const char* mqtt_topic_light2 = "phytotron/light2_manual";

String baseTopic = "phytotron/";

// Level on irrigation firing
int moisture_trigger = 2000; // set the level
int temperature_trigger = 25;
int humidity_trigger = 50;

void callback(char* topic, byte* payload, unsigned int length) {
  float val=0;// pour sauvgarder la valeur reçu aprés la convertir en float
  String result="";
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    result+=(char)payload[i];

  }
  //payload to float
  const char *c = result.c_str();
  val=atof(c);
  Serial.println();
  Serial.println("-----------------------");

  const char* top=topic;

  if(strcmp(top,mqtt_topic_fan)==0){
    if (val == 1) {
      digitalWrite(fanRelay, LOW);
      lcd.setCursor(10,3);
      lcd.print("         ");
      lcd.setCursor(10,3);
      lcd.print("Fan:ON");
    } else {
      digitalWrite(fanRelay, HIGH);
      lcd.setCursor(10,3);
      lcd.print("         ");
      lcd.setCursor(10,3);
      lcd.print("Fan:OFF");      
    }
  }

  if(strcmp(top,mqtt_topic_pump)==0){
    if (val == 1) {
      digitalWrite(pumpRelay, LOW);
      lcd.setCursor(0,3);
      lcd.print("         ");
      lcd.setCursor(0,3);
      lcd.print("Pump:ON");
    } else {
      digitalWrite(pumpRelay, HIGH);
      lcd.setCursor(0,3);
      lcd.print("         ");
      lcd.setCursor(0,3);
      lcd.print("Pump:OFF");    
    }
  }

  if(strcmp(top,mqtt_topic_light1)==0){
    if (val == 1) {
      digitalWrite(lightRelay1, LOW);
      lcd.setCursor(0,2);
      lcd.print("          ");
      lcd.setCursor(0,2);
      lcd.print("Light1:ON");
    } else {
      digitalWrite(lightRelay1, HIGH);
      lcd.setCursor(0,2);
      lcd.print("          ");
      lcd.setCursor(0,2);
      lcd.print("Light1:OFF");    
    }
  }  

  if(strcmp(top,mqtt_topic_light2)==0){
    if (val == 1) {
      digitalWrite(lightRelay2, LOW);
      lcd.setCursor(10,2);
      lcd.print("          ");
      lcd.setCursor(10,2);
      lcd.print("Light2:ON");
      delay(1000);
    } else {
      digitalWrite(lightRelay2, HIGH);
      lcd.setCursor(10,2);
      lcd.print("          ");
      lcd.setCursor(10,2);
      lcd.print("Light2:OFF");    
    }
  }  
}

void connectWifi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
}

void disconnectWifi() {
  WiFi.disconnect(true);
  Serial.println("WiFi disonnected");
}

void connectMqtt() {
  Serial.println("Connecting to MQTT1...");
  client.setServer(MQTT_HOST, MQTT_PORT);

  while (!client.connected()) {
    if (!client.connect(MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.print("MQTT connection failed:");
      Serial.print(client.state());
      Serial.println("Retrying...");
      delay(MQTT_RETRY_WAIT);
    }
  }

  Serial.println("MQTT1 connected");
  Serial.println("");
}

void connectMqtt2() {
  Serial.println("Connecting to MQTT2...");
  client2.setServer(MQTT_HOST, MQTT_PORT);

  while (!client2.connected()) {
    if (!client2.connect(MQTT_CLIENTID2, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.print("MQTT connection failed:");
      Serial.print(client2.state());
      Serial.println("Retrying...");
      delay(MQTT_RETRY_WAIT);
    }
  }

  Serial.println("MQTT2 connected");
  Serial.println("");

  client2.setCallback(callback);
  client2.subscribe("phytotron/fan_manual");
  client2.subscribe("phytotron/pump_manual");
  client2.subscribe("phytotron/light1_manual");
  client2.subscribe("phytotron/light2_manual");
}

void disconnectMqtt() {
  client.disconnect();
  Serial.println("MQTT disconnected");
}

void RTC_Update(){
  // Do udp NTP lookup, epoch time is unix time - subtract the 30 extra yrs (946684800UL) library expects 2000
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime()-946684800UL;
  Rtc.SetDateTime(epochTime); 
}

bool RTC_Valid(){
  bool boolCheck = true; 
  if (!Rtc.IsDateTimeValid()){
    Serial.println("RTC lost confidence in the DateTime!  Updating DateTime");
    boolCheck = false;
    RTC_Update();    
  }
  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now.  Updating Date Time");
    Rtc.SetIsRunning(true);
    boolCheck = false;
    RTC_Update();
  }
}

void setup(){
  Serial.begin(115200);
  /*
  // Start NTP Time Client
  timeClient.begin();
  delay(2000);
  timeClient.update();
  Rtc.Begin();
  RTC_Update();
  */  
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
  // PinMode Definition
  pinMode(fanRelay, OUTPUT);                // Fan in digital output
  pinMode(pumpRelay, OUTPUT);               // pump in digital output
  pinMode(lightRelay1, OUTPUT);             // light1 in digital output
  pinMode(lightRelay2, OUTPUT);             // light2 in digital output
  pinMode(sensorPin, INPUT);                // moisture sensor input
  pinMode(pumpManualPin, INPUT);            // Button pump manual input
  pinMode(fanManualPin, INPUT);             // Button fan munal input
  
  digitalWrite(lightRelay1, LOW);          // Initialization of Light1 (turn on)
  digitalWrite(lightRelay2, LOW);          // Initialization of Light2 (turn on)
  digitalWrite(fanRelay, HIGH);             // Initialization of fan (turn off)
  digitalWrite(pumpRelay, HIGH);            // Initialization of pump (turn off)
  
  // Initialization of DHT sensor (Temperature + Relative Humidity)
  dht.setup(dhtPin, DHTesp::DHT11);

  lcd.setCursor(0,0);
  lcd.print(".. Initialization ..");
  connectWifi();
  lcd.setCursor(0,1);
  lcd.print("Wi-Fi connected");
  lcd.setCursor(0,2);
  lcd.print("SSID: ");
  lcd.print(WIFI_SSID);
  lcd.setCursor(0,3);
  lcd.print("IP: ");
  lcd.print(WiFi.localIP());

  delay(20000);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(".. Initialization ..");
  connectMqtt();
  lcd.setCursor(0,1);
  lcd.print("MQTT 1 connected");
  connectMqtt2();
  lcd.setCursor(0,2);
  lcd.print("MQTT 2 connected");
  lcd.setCursor(0,3);
  lcd.print(MQTT_HOST);
  lcd.print(":");
  lcd.print(MQTT_PORT);
  
  delay(20000);

  lcd.clear();
  lcd.setCursor(0,2);
  lcd.print("         ");
  lcd.setCursor(0,2);
  lcd.print("Light1:ON");
  snprintf(buffer, 64, "%s", "ON");
  client.publish((baseTopic + "Light1").c_str(), buffer); 
  lcd.setCursor(10,2);
  lcd.print("         ");
  lcd.setCursor(10,2);
  lcd.print("Light2:ON");
  snprintf(buffer, 64, "%s", "ON");
  client.publish((baseTopic + "Light2").c_str(), buffer);
}

void manualPumping() {
  val = digitalRead(pumpManualPin);
  delay(10);
  val2 = digitalRead(pumpManualPin);
    
  if (val == val2 && val == 0) {
    lcd.setCursor(0,3);
    lcd.print("         ");
    lcd.setCursor(0,3);
    lcd.print("Pump:ON");
    boucle = true;
    while (boucle == true) {
      digitalWrite(pumpManualPin, LOW);
      delay(500);
      val = digitalRead(pumpManualPin);
      delay(10);
      val2 = digitalRead(pumpManualPin);
      if (val == val2 && val == 1) { 
        boucle = false;
        digitalWrite(pumpManualPin, HIGH);
        lcd.setCursor(0,3);
        lcd.print("Pump:OFF");
      } 
    }
  }
}

void manualFan() {
  val3 = digitalRead(fanManualPin);
  delay(10);
  val4 = digitalRead(fanManualPin);
    
  if (val3 == val4 && val3 == 0) {
    lcd.setCursor(10,3);
    lcd.print("         ");
    lcd.setCursor(10,3);
    lcd.print("Fan:ON");
    boucle2 = true;
    while (boucle2 == true) {
      digitalWrite(fanManualPin, LOW);
      delay(500);
      val3 = digitalRead(fanManualPin);
      delay(10);
      val4 = digitalRead(fanManualPin);
      if (val == val2 && val == 1) { 
        boucle2 = false;
        digitalWrite(fanManualPin, HIGH);
        lcd.setCursor(10,3);
        lcd.print("Fan:OFF");
      } 
    }
  }
}

void loop(){
  lcd.setCursor(0, 0);
  lcd.print("Phytotron de Rachida");
  
  TempAndHumidity lastValues = dht.getTempAndHumidity();
  
  value = analogRead(sensorPin);
  //value = map(value,550,0,0,100);
  lcd.setCursor(0,1);
  lcd.print("                   ");
  lcd.setCursor(0,1);
  lcd.print("Moisture: ");
  lcd.print(value);
  manualPumping();
  manualFan();
  delay(2000);
  lcd.setCursor(0,1);
  lcd.print("                   ");
  lcd.setCursor(0,1);
  lcd.print("Temperature: ");
  lcd.print(String(lastValues.temperature,1));
  lcd.print(" C");
  manualPumping();
  manualFan();
  delay(2000);
  lcd.setCursor(0,1);
  lcd.print("                   "); 
  lcd.setCursor(0,1);
  lcd.print("Humidity: ");
  lcd.print(String(lastValues.humidity,1));
  lcd.print(" %");
  lcd.setCursor(0,3);
  manualPumping();
  manualFan();
  delay(2000);
  lcd.print("Pump:OFF");
  snprintf(buffer, 64, "%s", "OFF");
  client.publish((baseTopic + "pump").c_str(), buffer); 
  lcd.setCursor(10,3);
  lcd.print("Fan:OFF");
  snprintf(buffer, 64, "%s", "OFF");
  client.publish((baseTopic + "pump").c_str(), buffer); 
  manualPumping();
  manualFan();
  
  if (value <= moisture_trigger) { 
    digitalWrite(pumpRelay, LOW);
    lcd.setCursor(0,3);
    lcd.print("         ");
    lcd.setCursor(0,3);
    lcd.print("Pump:ON");
    snprintf(buffer, 64, "%s", "ON");
    client.publish((baseTopic + "pump").c_str(), buffer); 
    delay(30000);
    digitalWrite(pumpRelay, HIGH);
    lcd.setCursor(0,3);
    lcd.print("Pump:OFF");
    snprintf(buffer, 64, "%s", "OFF");
    client.publish((baseTopic + "pump").c_str(), buffer); 
  }

  if (lastValues.temperature >= temperature_trigger || lastValues.humidity >= humidity_trigger) {
    digitalWrite(fanRelay, LOW);
    lcd.setCursor(10,3);
    lcd.print("         ");
    lcd.setCursor(10,3);
    lcd.print("Fan:ON");
    snprintf(buffer, 64, "%s", "ON");
    client.publish((baseTopic + "fan").c_str(), buffer); 
    delay(30000);
    digitalWrite(fanRelay, HIGH);
    lcd.setCursor(10,3);
    lcd.print("Fan:OFF");
    snprintf(buffer, 64, "%s", "OFF");
  client.publish((baseTopic + "fan").c_str(), buffer);
  }
  
  snprintf(buffer, 64, "%g", lastValues.temperature);
  client.publish((baseTopic + "temperature").c_str(), buffer); 
  snprintf(buffer, 64, "%g", lastValues.humidity); 
  client.publish((baseTopic + "humidity").c_str(), buffer);
  snprintf(buffer, 64, "%d", value); 
  client.publish((baseTopic + "moisture").c_str(), buffer);
  /*
  snprintf(buffer, 64, "%d", light);
  client.publish((baseTopic + "light").c_str(), buffer);
  snprintf(buffer, 64, "%d", conductivity);
  client.publish((baseTopic + "conductivity").c_str(), buffer);
  */
  client2.loop();
}
