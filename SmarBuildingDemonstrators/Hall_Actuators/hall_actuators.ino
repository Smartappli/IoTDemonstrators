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
#include <ESP32Servo.h>

// WiFi
// Make sure to update this for your own WiFi network!
const char* ssid = "IoTWifi";
const char* password ="SmartBuilding";
const char* mqttServer = "192.168.4.1";
const int mqttPort = 1883;

// MQTT
const char* mqtt_topic_mail = "hall/courrier";
const char* mqtt_topic_eclairage = "hall/eclairage";
const char* mqtt_topic_porte = "hall/porte";
const char* mqtt_topic_porte_manuel = "hall/porte/manuel";
const char* mqtt_topic_eclairage_manuel = "hall/eclairage/manuel";

// The client id identifies the ESP8266 device. Think of it a bit like a hostname (Or just a name, like Greg).
const char* clientID = "esp32_salon";

// Initialise the WiFi and MQTT Client objects
WiFiClient espClient;
PubSubClient client(mqttServer, 1883, espClient); // 1883 is the listener port for the Broker
Servo servo;
// Données à envoyées en plus
int servo_pin=32;
int light= 5;
int post_light=21;
int eclairage=0;
int auto_eclairage=1;
int porte_ouverte=0;
int auto_porte=0;
int mail=0;
int temps_arret_porte=0;
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
  if(auto_eclairage==1)
  eclairage=(int)val;
  }
else if(strcmp(top,mqtt_topic_mail)==0){
 mail=(int)val;
  }

else if(strcmp(top,mqtt_topic_porte_manuel)==0){
temps_arret_porte=time(NULL);
porte_ouverte=(porte_ouverte==0)?1:0; 
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
  pinMode(light, OUTPUT);
  pinMode(post_light, OUTPUT);
  servo.attach(servo_pin);
  servo.write(0);
WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
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
  
  client.subscribe("hall/courrier");
  client.subscribe("hall/eclairage");
      client.subscribe("hall/porte/manuel");
    client.subscribe("hall/eclairage/manuel");


}

void loop() {
  if(time(NULL)-temps_arret_porte>60)
  porte_ouverte=0;
  if(time(NULL)-temps_arret_ecl>60)
  auto_eclairage=1;
  //boite à lettres
  if(mail==1){
    digitalWrite(post_light, HIGH);
    }
    else {
    digitalWrite(post_light,LOW);
    }
    //eclairage
 if(eclairage==1){
    digitalWrite(light, HIGH);
    }
    else {
    digitalWrite(light,LOW);
    }
    //porte
    if(porte_ouverte==1){
    servo.write(90);
    } else{
      servo.write(0);
      }

client.loop();    
}
