
#include <WiFi.h> // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h> // Allows us to connect to, and publish to the MQTT broker
#include<time.h>
#include<string.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"
// WiFi
// Make sure to update this for your own WiFi network!
const char* ssid = "IoTWifi";
const char* wifi_password = "SmartBuilding";

// MQTT
// MQTT
const char* mqtt_server = "192.168.4.1";
const char* mqtt_topic_pluie = "Home/pluie";
const char* mqtt_topic_people = "sdb/people";
const char* mqtt_topic_people1 = "sdb/bedroom/people";
const char* mqtt_topic_humidite = "sdb/humidite";
const char* mqtt_topic_temperature = "sdb/temperature";
const char* mqtt_topic_Buzzer = "sdb/Buzzer";
const char* mqtt_topic_Laser = "sdb/Laser";
const char* mqtt_topic_Ventilo = "sdb/Ventilo";
const char* mqtt_topic_hum_ref = "sdb/refHum";
const char* mqtt_topic_eclairage = "sdb/eclairage";
const char* mqtt_topic_eclairage_manuel = "sdb/eclairage/manuel";
// The client id identifies the ESP8266 device. Think of it a bit like a hostname (Or just a name, like Greg).
const char* clientID = "esp32_sdb";
// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, wifiClient); // 1883 is the listener port for the Broker
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)



//temperature/humidite sensor
#include "DHT.h"
#define DHTPIN 26
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Declaration des variables

#define ANALOG_PIN_7 35
int tempR=0;
int Ventilo = 0;
int Laser = 0;
int Buzzer = 0;
byte humiditeReference=0;
int ecarthumidite=3; //Différence d'humidité pour que le ventilateur s'active
int order = 0;
int waterlevelRef = 0;
int ecartWater=200; //Différence du niveau d'eau pour que le buzzer s'active
int v=0;
int lum=0;
int persons_bedroom = 0;
int persons = 0;
int Sortir = 0; 
int Entrer = 0;
int pinInfraRouge1=16;
int pinInfraRouge2=17;
int bouton_eclairage=4;
int etat_bouton_eclairage=0;
int automatisation_eclairage=1;
int eclairage=0;
int temps_arret1=0;
// Données à envoyer

int hum_send = 0;
int waterlvl_send = 0;

//functions
int advancedRead(void)
{
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  int val=0;
  // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
  // That way you can do whatever math and comparisons you want!
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  Serial.print(F("[ ")); Serial.print(millis()); Serial.print(F(" ms ] "));
  Serial.print(F("IR: ")); Serial.print(ir);  Serial.print(F("  "));
  Serial.print(F("Full: ")); Serial.print(full); Serial.print(F("  "));
  Serial.print(F("Visible: ")); Serial.print(full - ir); Serial.print(F("  "));
  Serial.print(F("Lux: ")); Serial.println(tsl.calculateLux(full, ir), 6);
  if(full>3000)
  val=1;
  else val=0;
  return val;
}
//*********************fonction d'eclairage************************************************
int eclairage_auto(int persons,int lum){
    int eclairage=0;
    if(persons>0 && lum==0)
    eclairage=1;
    else
    eclairage=0;
    
    return eclairage;
  }
//**********************************************************************************************
//**********************************************************************************************
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
    // envoie des informations au thingsboard
void getAndSendTemperatureAndhumiditeDataTB(float temp,int humidite,int persons,int persons_bedroom,int eclairage)
{
char TOKEN[] ="u06EByrrXS9d3D5MFtEm"; //Access token of device Display
char ThingsboardHost[] = "demo.thingsboard.io";
  WiFiClient wifiClient;
  PubSubClient client(wifiClient);
  client.setServer(ThingsboardHost,1883);
int status = WL_IDLE_STATUS;
//verifier connct
  if (!client.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(ssid, wifi_password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("Esp8266", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.println( " : retrying in 5 seconds]" );
      delay( 500 );
    }
  }
  // Prepare a JSON payload string
  String payload = "{";
  payload += "\"Temp\":";payload += temp;payload += ","; 
  payload += "\"Hum\":";payload += humidite; payload += ",";
  payload += "\"per_sdb\":";payload += persons; payload += ",";
  payload += "\"per_chm\":";payload += persons_bedroom; payload += ",";
  payload += "\"Ecl\":";payload += eclairage;
  payload += "}";

  char attributes[1000];
  payload.toCharArray( attributes, 1000 );
  client.publish( "v1/devices/me/telemetry",attributes);
  Serial.println( attributes );
   
} 
//**************************
void callback(char* topic, byte* payload, unsigned int length) {
  // pour sauvgarder la valeur reçu aprés la convertir en float
  int val=0;
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
//temperature
const char* top=topic;

 if(strcmp(top,mqtt_topic_hum_ref)==0){
 humiditeReference=(int)val;
}
}
void setup() {
  // put your setup code here, to run once:
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
 if (tsl.begin()) 
  {
   Serial.println(F("Found a TSL2591 sensor"));
  } 
  else 
  {
    Serial.println(F("No sensor found ... check your wiring?"));
    while (1);
  }
    client.setServer(mqtt_server,1883);
if (client.connect(clientID)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
  client.setCallback(callback);
  //Prendre la temperature et l'humidité de référence
   dht.begin();
 if (isnan(dht.readTemperature())|| isnan(dht.readHumidity())) {
  Serial.println("ERREUR REF");
  } else{
    humiditeReference = dht.readHumidity();
  }
 pinMode(bouton_eclairage, INPUT);
  pinMode(pinInfraRouge1, INPUT);// set pin as input
 pinMode(pinInfraRouge2, INPUT);
 client.subscribe("sdb/refHum");
}


void loop() {

  //Capteur température et humidité    
  float  temperature = dht.readTemperature();;
  float humidite = dht.readHumidity();
  lum=advancedRead();
  
  Serial.print((int)temperature); Serial.print(" *C, "); 
  Serial.print((int)humidite); Serial.println(" H");
  hum_send = (int)humidite;
  Serial.print((int)humiditeReference); Serial.println(" H de référence");


  if(humidite > (int)humiditeReference+ecarthumidite){
    Laser=1;
    Ventilo=1;
    }
    else { 
    Laser=0;
    Ventilo=0;
      }
  int detect1 = digitalRead(pinInfraRouge1);
  int detect2 = digitalRead(pinInfraRouge2);
  persons_bedroom=0;
  if (detect1 == LOW  && detect2 == HIGH){
    Serial.println("Détecter2");
      Entrer = 1;

      if(Sortir==1){
        persons--;
        persons_bedroom=1;//il faut incremonter le nombre des persons dans la bedroom
        envoie_to_mqttbroker(mqtt_topic_people1,persons_bedroom);
        Entrer=0;
        Sortir=0;
        if(persons<0) {persons =0;}
       delay(1000);
      }    
    }
    
  if (detect2 == LOW && detect1 == HIGH){
      Serial.println("Détecter1");
      Sortir = 1;

      if(Entrer == 1){
        persons++;
        persons_bedroom=2;//il faut décremonter le nombre des persons dans la bedroom
        envoie_to_mqttbroker(mqtt_topic_people1,persons_bedroom);
        Entrer=0;
        Sortir=0;
        delay(1000);
      }
    }
  
/*
  if (detect1 == LOW  && detect2 == HIGH){
    Serial.println("Détecter1");
    if(v==0){
      tempR=time(NULL);
      v=10;
    }
    else if(v==1){
      persons--;
      persons_bedroom=1;//il faut incremonter le nombre des persons dans la bedroom
      envoie_to_mqttbroker(mqtt_topic_people1,persons_bedroom);
      if(persons<0) {persons =0;}
      v=0;
      delay(1000);
    }
  }
    
  if (detect2 == LOW && detect1 == HIGH){
    Serial.println("Détecter2");
    if(v==0){
      v=1;
      tempR=time(NULL);
    }
    else if(v == 10){
        persons++;
        persons_bedroom=2;//il faut décremonter le nombre des persons dans la bedroom
        envoie_to_mqttbroker(mqtt_topic_people1,persons_bedroom);
        v=0;
        delay(1000);
     }
   }
    
   if(time(NULL)-tempR>12)
    v=0;
   if (detect2 == LOW && detect1 == LOW)
    Serial.println("****************************************************************************************************************");
   */ 
    delay(100);

  Serial.print("Personnes");
  Serial.println(persons);  
  //le bouton eclairage
  etat_bouton_eclairage = digitalRead(bouton_eclairage);
  if (etat_bouton_eclairage== HIGH) {     
    automatisation_eclairage=0;
   envoie_to_mqttbroker(mqtt_topic_eclairage_manuel,automatisation_eclairage);
  }
eclairage=eclairage_auto(persons,lum);
  //Capteur niveau d'eau
  double waterlevel = analogRead(ANALOG_PIN_7);
  int waterlvl_send = (int)waterlevel;
  Serial.println(waterlevelRef); 
  delay(100);
  //Passive Buzzer

  if (waterlvl_send>waterlevelRef+ecartWater){
 Serial.println("Buzz");
  Buzzer=1;}
  else 
  Buzzer=0;

  // Sending data to mqtt server

envoie_to_mqttbroker(mqtt_topic_eclairage, eclairage);
envoie_to_mqttbroker(mqtt_topic_Ventilo, Ventilo);
envoie_to_mqttbroker(mqtt_topic_Buzzer, Buzzer);
envoie_to_mqttbroker(mqtt_topic_Laser, Laser);
envoie_to_mqttbroker(mqtt_topic_people, persons);

  // Envoi des données vers thingsboard
getAndSendTemperatureAndhumiditeDataTB(temperature,humidite,persons,persons_bedroom,eclairage);
delay(1000);
client.loop();
}
