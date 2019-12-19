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
// WiFi
// Make sure to update this for your own WiFi network!
const char* ssid = "IoTWifi";
const char* wifi_password = "SmartBuilding";

// MQTT
const char* mqtt_server = "192.168.4.1";
const char* mqtt_topic_RGB = "garage/rgb";
const char* mqtt_topic_citerne = "garage/citerne";
const char* mqtt_topic_door = "garage/bigdoor";
const char* mqtt_topic_eclairage = "garage/eclairage";
const char* mqtt_topic_bigdoor_manuel = "garage/door/manuel";


// The client id identifies the ESP8266 device. Think of it a bit like a hostname (Or just a name, like Greg).
const char* clientID = "esp32_Garage";

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, wifiClient); // 1883 is the listener port for the Broker


int persons = 0;
int temp_bigdoor=0;
int etat_bouton_bigdoor=0;
int detect=0;
int temp_det=0;
// Pin water sensor
#define ANALOG_PIN_0 33
int mouvement=32;
int bouton_bigdoor=4;
#include <NewPing.h>

#define TRIGGER_PIN 18  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN 5  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 300 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.


// Infos à envoyer 
int water_level=0;
int eclairage=0;//
int door=0;//1:open the door 0:close the door
int rgb=0;
//************************************************************************
int eclairage_auto(int *temps_arret1,int detection,int *eclairage){
  if(detection==1){ 
    *eclairage=1;
    *temps_arret1=time(NULL);
    }
     //detection==0
   else {
    int now=time(NULL);
    if(now-*temps_arret1>60){
      *eclairage=0;
      }  }
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
void getAndSendTemperatureAndHumidityDataTB(int water_level,int eclairage,int door,int rgb)
{
char TOKEN[] ="05LDpzCFgWu3LqUQU60J"; //Access token of device Display
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
  payload += "\"water_lev\":";payload += water_level;payload += ","; 
  payload += "\"door\":";payload += door; payload += ",";
  payload += "\"Ecl\":";payload += eclairage;payload += ",";
  payload += "\"RGB\":";payload += rgb;
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
  pinMode(ANALOG_PIN_0, INPUT);
  pinMode(mouvement, INPUT);
  pinMode(bouton_bigdoor, INPUT);

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
}

void loop() {
  client.setServer(mqtt_server,1883);
    // Connect to MQTT Broker
  // client.connect returns a boolean value to let us know if the connection was successful.
  if (client.connect(clientID)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
  delay(500);  // Wait 500ms between pings (about 2 pings/sec). 29ms should be the shortest delay between pings.
  
  int val = digitalRead(mouvement);  // read input value
  if (val == HIGH){
    detect=1;
    eclairage=1;
    temp_det=time(NULL);
  }
  else
  if(time(NULL)-temp_det>60){
  detect=0;
  eclairage=0;
  }
  
  unsigned int uS = sonar.ping(); // Send ping, get ping time in microseconds (uS).
  Serial.print("Ping: ");
  Serial.print(uS / US_ROUNDTRIP_CM); // Convert ping time to distance and print result (0 = outside set distance range, no ping echo)
  Serial.println("cm");
  etat_bouton_bigdoor=digitalRead(bouton_bigdoor);
  if(etat_bouton_bigdoor==HIGH){
    envoie_to_mqttbroker(mqtt_topic_bigdoor_manuel,(int) door);
    }
   
    //etat_bouton_bigdoor==LOW
    int distance=uS/US_ROUNDTRIP_CM;
    int t=time(NULL)-temp_bigdoor;
    
    if (distance<10){ /**ROUGE**/
    rgb=2;//
    } 
    else if(distance>=10 && distance<30){ /**VERT**/
   rgb=1;//red// la voiture est bien garré
   }
  else if(distance>=30){
    rgb=3;
    }
  
    
    
//Water lvl tank
  double tank_value = analogRead(ANALOG_PIN_0);
  Serial.println(tank_value);

  if(tank_value<1000){
    water_level = 1;
  }
   else if(tank_value >1900 && tank_value<2150)
        water_level = 2;
   else if( tank_value>=2150)
        water_level = 3;
    
  // Send data
envoie_to_mqttbroker(mqtt_topic_door,(int) door);
envoie_to_mqttbroker(mqtt_topic_citerne,(int) water_level);
envoie_to_mqttbroker(mqtt_topic_eclairage,(int) eclairage);
envoie_to_mqttbroker(mqtt_topic_RGB,(int) rgb);

getAndSendTemperatureAndHumidityDataTB(water_level,eclairage,door,rgb);

}
