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
const char* mqtt_topic_mail = "hall/courrier";
const char* mqtt_topic_eclairage = "hall/eclairage";
const char* mqtt_topic_porte = "hall/porte";
const char* mqtt_topic_porte_manuel = "hall/porte/manuel";
const char* mqtt_topic_eclairage_manuel = "hall/eclairage/manuel";
// The client id identifies the ESP8266 device. Think of it a bit like a hostname (Or just a name, like Greg).
const char* clientID = "esp32_Hall_capt";

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, wifiClient); // 1883 is the listener port for the Broker
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)

// Sensors

int lum=0;
int persons = 0;
int Sortir = 0; 
int Entrer = 0;
int pinInfraRouge1=16;
int pinInfraRouge2=17;
int bouton_porte=34;
int bouton_eclairage=4;
int etat_bouton_eclairage=0;
int automatisation_eclairage=1;
int automatisation_porte=1;
int temp_eclairage=0;
int temp_porte=0;
int post_pin=18;
int porte=0;


// Données à envoyer 
int eclairage=0;
int courrier = 0;
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
void getAndSendTemperatureAndHumidityDataTB(int courrier,int persons,int eclairage)
{
char TOKEN[] ="MapnPaeL9RGEKBVrcrRy"; //Access token of device Display
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
  payload += "\"Courrier\":";payload += courrier;payload += ","; 
  payload += "\"personnes\":";payload += persons; payload += ",";
  payload += "\"Ecl\":";payload += eclairage;payload;
  payload += "}";

  char attributes[1000];
  payload.toCharArray( attributes, 1000 );
  client.publish( "v1/devices/me/telemetry",attributes);
  Serial.println( attributes );
   
} 

//**************************
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

  // Connect to MQTT Broker
  // client.connect returns a boolean value to let us know if the connection was successful.
  if (client.connect(clientID)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
  
// autres setup à faire

  pinMode(bouton_eclairage, INPUT);
  pinMode(bouton_porte, INPUT);
  pinMode(pinInfraRouge1, INPUT);
  pinMode(pinInfraRouge2, INPUT);
  pinMode(post_pin, INPUT);


  
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
  int detect1 = digitalRead(pinInfraRouge1);
  int detect2 = digitalRead(pinInfraRouge2);
  int button = digitalRead(bouton_porte);
  int post= digitalRead(post_pin);
  lum=advancedRead();

  //le bouton eclairage
etat_bouton_eclairage = digitalRead(bouton_eclairage);
if (etat_bouton_eclairage== HIGH) { 
     envoie_to_mqttbroker(mqtt_topic_eclairage_manuel ,automatisation_eclairage);
   }
  
  if(post==HIGH){
    courrier = 1;
    }
  if(post==LOW){
    courrier = 0;
  }

 
  if(button==HIGH ){
   envoie_to_mqttbroker(mqtt_topic_porte_manuel ,automatisation_porte);
  }
  
    
   if (detect1 == LOW  && detect2 == HIGH){
      Entrer = 1;

      if(Sortir==1){
        persons--;
        Entrer=0;
        Sortir=0;
        if(persons<0) {persons =0;}
    delay(1000);
      }    
    }
    
  if (detect2 == LOW && detect1 == HIGH){
      //Serial.println("Détecter2");
      Sortir = 1;

      if(Entrer == 1){
        persons++;
        Entrer=0;
        Sortir=0;
     delay(1000);
      }
    }
eclairage=eclairage_auto(persons,lum);
//envoie des données
envoie_to_mqttbroker(mqtt_topic_mail,courrier);
envoie_to_mqttbroker(mqtt_topic_eclairage,eclairage);
// Envoi des données vers thingsboard
getAndSendTemperatureAndHumidityDataTB(courrier,persons,eclairage);
client.loop();
}
