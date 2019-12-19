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
#include<string.h>
#include<time.h>
#include <ESP32Servo.h>

// WiFi
// Make sure to update this for your own WiFi network!
const char* ssid = "IoTWifi";
const char* password ="SmartBuilding";
const char* mqttServer = "192.168.4.1";
const int mqttPort = 1883;

// MQTT
const char* mqtt_topic_window_state = "cuisine/window/state";
const char* mqtt_topic_eclairage = "cuisine/eclairage";
const char* mqtt_topic_window_manuel = "cuisine/window/manuel";
const char* mqtt_topic_eclairage_manuel = "cuisine/eclairage/manuel";

// The client id identifies the ESP8266 device. Think of it a bit like a hostname (Or just a name, like Greg).
const char* clientID = "esp32_Cuisine_act";

// Initialise the WiFi and MQTT Client objects
WiFiClient espClient;
PubSubClient client(mqttServer, 1883, espClient); // 1883 is the listener port for the Broker
Servo servo;

int fenetre_ouverte=0;
int eclairage=0;
int temp=1;
int auto_window=1;
int auto_eclairage=1;
int temps_arret_win=0;
int temps_arret_ecl=0;
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
//temperature
const char* top=topic;

if(strcmp(top,mqtt_topic_eclairage)==0){
  Serial.print("la valeur de l'eclairage est:");
  Serial.println(val);
 if(auto_eclairage==1)
 eclairage=val;
}
else if(strcmp(top,mqtt_topic_window_state)==0){
 // Ouvre et ferme la fenetre
Serial.print("l'etat de la fenetre est:");
  Serial.println(val);
  if(auto_window==1)
  fenetre_ouverte=val;    
}
else if(strcmp(top,mqtt_topic_window_manuel)==0){
auto_window=0;
temps_arret_win=time(NULL);
fenetre_ouverte=(fenetre_ouverte==0)?1:0;  
}
else if(strcmp(top,mqtt_topic_eclairage_manuel)==0){
if(eclairage==0){
auto_eclairage=0;
temps_arret_ecl=time(NULL);
eclairage=(eclairage==0)?1:0;  
}
else
auto_eclairage=1;
}

}

void setup() {
  Serial.begin(115200);
  pinMode(25, OUTPUT);
  servo.attach(13);
// put your setup code here, to run once:
  
  Serial.println("ESP32 Analog IN Test");
  
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Connect to the WiFi
  WiFi.begin(ssid,password);

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
  client.setCallback(callback);
  client.subscribe("cuisine/window/state");
  client.subscribe("cuisine/eclairage");
  client.subscribe("cuisine/window/manuel");
  client.subscribe("cuisine/eclairage/manuel");
}

void loop() {
  client.setServer(mqttServer, mqttPort); 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP32Client")) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
  if(time(NULL)-temps_arret_win>60)
  auto_window=1;
  if(time(NULL)-temps_arret_ecl>60)
  auto_eclairage=1;
  if(eclairage==1){
    digitalWrite(25, HIGH);
    }
    else {
    digitalWrite(25,LOW);
    }
 if(fenetre_ouverte==1  ){
    servo.write(90);
   
    } else if(fenetre_ouverte==0 ){
      servo.write(0);
      }
client.loop();    
}
