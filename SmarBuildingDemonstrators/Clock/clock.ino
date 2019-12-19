/* TSL2591 Digital Light Sensor */
/* Dynamic Range: 600M:1 */
/* Maximum Lux: 88K */
#include <WiFi.h> // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h> // Allows us to connect to, and publish to the MQTT broker
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"
// WiFi
// Make sure to update this for your own WiFi network!
const char* ssid = "IoTWifi";
const char* wifi_password = "SmartBuilding";
// MQTT
const char* mqtt_topic_temp="chambre/temperature";
const char* mqtt_server = "192.168.4.1";
const char* mqtt_topic_pluie = "Home/pluie";
const char* mqtt_topic_horloge_heure = "Home/heure";
const char* mqtt_topic_horloge_minute = "Home/minute";
const char* mqtt_topic_horloge_seconde = "Home/seconde";
const char* mqtt_topic_horloge_time = "Home/time";



// The client id identifies the ESP8266 device. Think of it a bit like a hostname (Or just a name, like Greg).
const char* clientID = "esp32_home";
// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
PubSubClient client(wifiClient); // 1883 is the listener port for the Broker

/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
*/
/**************************************************************************/
// Pin water sensor
#define ANALOG_PIN_0 33
#include <NewPing.h>

#define TRIGGER_PIN 18  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN 5  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
RTC_DS1307 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

int val = 0;
int pluie=0; 

void setup(void) 
{
Serial.begin(115200);
  delay(1000); // give me time to bring up serial monitor
  Serial.println("ESP32 Analog IN Test");
  
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Connect to the WiFi
  WiFi.begin(ssid, wifi_password);

  // Wait until the connection has been confirmed before continuing
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Debugging - Output the IP Address of the ESP8266
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  
  Serial.println(F("Starting Adafruit TSL2591 Test!"));
   client.setServer(mqtt_server,1883);
    // Connect to MQTT Broker
  // client.connect returns a boolean value to let us know if the connection was successful.
  if (client.connect(clientID)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
   
 if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC lost power, lets set the time!");
  
  // Comment out below lines once you set the date & time.
    // Following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
    // Following line sets the RTC with an explicit date & time
    // for example to set January 27 2017 at 12:56 you would call:
    // rtc.adjust(DateTime(2017, 1, 27, 12, 56, 0));
  }
  client.subscribe("chambre/temperature");
  client.subscribe("testtest");

}


//****************************************************************************
void envoie_to_mqttbroker(const char* topic, float donnee){
     char temp[16];
    //convertion vers un tableau de char 
     itoa((int)donnee, temp, 10);
    if (client.publish(topic, temp)){
    Serial.print("la valeur envoyé pour le topic:");
    Serial.println(topic);
    Serial.println(temp);
    // client.publish(mqtt_topic, cstr);
    // Again, client.publish will return a boolean value depending on whether it succeded or not.
    // If the message failed to send, we will try again, as the connection may have broken.
  }else {
        Serial.println("Message failed to send. Reconnecting to MQTT Broker and trying again");
        client.connect(clientID);
        delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
        client.publish(topic, "Button pressed!");
  }
    }

void loop(void) 
{ 
 val = analogRead(33); // read input value
  Serial.print("Pin Value   ");
  Serial.println(val);
  if(val>0)
  pluie=1;
  else
  pluie=0;
  envoie_to_mqttbroker(mqtt_topic_pluie,pluie);
delay(1000);
DateTime now = rtc.now();
    
    Serial.println("Current Date & Time: ");
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    int time=now.hour()*100+now.minute();
    Serial.println("Unix Time: ");
    Serial.print("elapsed ");
    Serial.print(now.unixtime());
    Serial.print(" seconds/");
    Serial.print(now.unixtime() / 86400L);
    Serial.println(" days since 1/1/1970");
    
    // calculate a date which is 7 days & 30 seconds into the future
    DateTime future (now + TimeSpan(7,0,0,30));
    
    Serial.println("Future Date & Time (Now + 7days & 30s): ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
    Serial.println();   
     envoie_to_mqttbroker(mqtt_topic_horloge_time,time);
     envoie_to_mqttbroker(mqtt_topic_horloge_seconde,now.second());

     
    delay(3000);
}
