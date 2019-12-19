/* Dans le cadre du projet Smart Ohm'e de BA3 ingénieur civil en informatique et gestion
 *  AFENZOUAR Farid 
 *  email : Farid.AFENZOUAR@student.umons.ac.be
 *  FISICARO Federico
 *  email: Federico.FISICARO@student.umons.ac.be
 *  NGUYEN Harry
 *  email : Harry.NGUYEN@student.umons.ac.be
 *  VANSNICK Loïc
 *  email : Loic.VANSNICK@student.umons.ac.be
 */
 
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
const char* mqtt_server = "192.168.4.1";
const char* mqtt_topic_pluie = "Home/pluie";
const char* mqtt_topic_people = "cuisine/people";
const char* mqtt_topic_people1 = "cuisine/sallon/people";
const char* mqtt_topic_temperature = "cuisine/temperature";
const char* mqtt_topic_window_manuel = "cuisine/window/manuel";
const char* mqtt_topic_eclairage_manuel = "cuisine/eclairage/manuel";
const char* mqtt_topic_temp_ref = "cuisine/refTemp";
const char* mqtt_topic_window_state = "cuisine/window/state";
const char* mqtt_topic_eclairage = "cuisine/eclairage";
// The client id identifies the ESP8266 device. Think of it a bit like a hostname (Or just a name, like Greg).
const char* clientID = "esp32_Cuisine_capt";

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
PubSubClient client(wifiClient); // 1883 is the listener port for the Broker
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)

// Données à envoyées en plus


// Pour les capteurs

#include "DHT.h"
#define DHTPIN 26
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
//declarations des variables:
float temp_ref=0;
int pluie=0;
int lum=0;
int persons = 0;
int persons_sallon = 0;
int Sortir = 0; 
int Entrer = 0;
int pinInfraRouge1=16;
int pinInfraRouge2=17;
int bouton_window = 12;     // Numéro de la broche à laquelle est connecté le bouton poussoir
int etat_Bouton_window=0;
int automatisation_window=1;
int fenetre_ouverte = 0;
int bouton_eclairage=15;
int etat_bouton_eclairage=0;
int automatisation_eclairage=1;
int eclairage=0;
int temps_arret1=0;
int temps_arret2=0;

//funtions
//tsl2591 function
/**************************************************************************/
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
//*********************fonction des fenetres****************************************************
int window(int t,int pluie,int temp_ref){
// Ouvre et ferme la fenetre
int fenetre_ouverte=0;

   if (pluie==1)
   {
    fenetre_ouverte=0;
    return fenetre_ouverte;
   } 
   else{
    if(t>=temp_ref){
    fenetre_ouverte = 1;
    } else if(t<temp_ref){
      fenetre_ouverte = 0;
      }
       return fenetre_ouverte;
    }
  }
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
void getAndSendTemperatureAndHumidityDataTB(float temp,int fenetre_ouverte,int persons_cuisine,int temp_ref,int eclairage)
{
char TOKEN[] ="fmqHuhFyOQU9rgl04h7i"; //Access token of device Display
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
  payload += "\"Ouverture_f\":";payload += fenetre_ouverte; payload += ",";
  payload += "\"personnes\":";payload += persons_cuisine; payload += ",";
  payload += "\"temp_ref\":";payload += temp_ref; payload += ",";
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
//si le topic arrivé est celui de la pluie
if(strcmp(top,mqtt_topic_pluie)==0){
    pluie=(int)val;
}
else if(strcmp(top,mqtt_topic_temp_ref)==0){
temp_ref=(int)val;
}
}   
void setup() {
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
// setup pour les capteurs
 pinMode(bouton_eclairage, INPUT);
  pinMode(bouton_window, INPUT);     
  pinMode(pinInfraRouge1, INPUT);// set pin as input
 pinMode(pinInfraRouge2, INPUT);
 dht.begin();
 
 if (isnan(dht.readTemperature())) {
  Serial.println("ERREUR REF");
  } else{
    temp_ref =(int)dht.readTemperature()+1;
    }
  client.subscribe("Home/pluie");
  client.subscribe("cuisine/refTemp");
}

void loop() {
    delay(200);
  client.setServer(mqtt_server,1883);
    // Connect to MQTT Broker
  // client.connect returns a boolean value to let us know if the connection was successful.
  if (client.connect(clientID)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }

  //recuparation de la valeur de la méteo
 Serial.print("la valeur de la pluie: ");
  Serial.println(pluie);
  //recuperation de la valeur de luminostité et pluie
  lum=advancedRead();
  Serial.print("la valeur de la luminosité: ");
  Serial.println(lum);
  
  float t = dht.readTemperature();

//le bouton window
etat_Bouton_window = digitalRead(bouton_window);
if (etat_Bouton_window == HIGH) {     
   envoie_to_mqttbroker(mqtt_topic_window_manuel ,automatisation_window);

}
  //le bouton eclairage
etat_bouton_eclairage = digitalRead(bouton_eclairage);
if (etat_bouton_eclairage== HIGH) { 
     envoie_to_mqttbroker(mqtt_topic_eclairage_manuel ,automatisation_eclairage);
   }
   
  int detect1 = digitalRead(pinInfraRouge1);
  int detect2 = digitalRead(pinInfraRouge2);
  persons_sallon=0;
  if (detect1 == LOW  && detect2 == HIGH){
    Serial.println("Détecter2");
      Entrer = 1;

      if(Sortir==1){
        persons--;
        persons_sallon=1;
        envoie_to_mqttbroker(mqtt_topic_people1,persons_sallon);
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
        persons_sallon= 2;
        envoie_to_mqttbroker(mqtt_topic_people1,persons_sallon);
        Entrer=0;
        Sortir=0;
        delay(1000);
      }
    }
  
// Ouvre et ferme la fenetre
fenetre_ouverte=window(t,pluie,temp_ref);

//eclairage
eclairage=eclairage_auto(persons,lum);

// Envoie des données toutes les demis seconde vers le mqtt broker
envoie_to_mqttbroker(mqtt_topic_people,persons);
envoie_to_mqttbroker(mqtt_topic_temperature,t);
envoie_to_mqttbroker(mqtt_topic_eclairage,eclairage);
envoie_to_mqttbroker(mqtt_topic_window_state,fenetre_ouverte);

// Envoi des données vers thingsboard
getAndSendTemperatureAndHumidityDataTB(t,fenetre_ouverte,persons,temp_ref,eclairage);

client.loop();
}
