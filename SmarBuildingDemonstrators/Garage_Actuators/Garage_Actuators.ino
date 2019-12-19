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
const char* mqtt_server = "192.168.4.1";
const char* mqtt_topic_RGB = "garage/rgb";
const char* mqtt_topic_citerne = "garage/citerne";
const char* mqtt_topic_door = "garage/bigdoor";
const char* mqtt_topic_eclairage = "garage/eclairage";
const char* mqtt_topic_bigdoor_manuel = "garage/door/manuel";


// The client id identifies the ESP8266 device. Think of it a bit like a hostname (Or just a name, like Greg).
const char* clientID = "esp32_Garage_act";

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, wifiClient); // 1883 is the listener port for the Broker


Servo servo;//for the bigdoor
//LED RGB
int ledJ=12;
int red=2;
int bleue=16;
int green=4;
//WATER_LEVEL cITERNE
int faible=25;
int moyen=35;
int bon=34;
int ecl=32;
// Données à envoyées en plus
int auto_bigdoor=1;
int temps_arret_bigdoor=0;
int eclairage=0;
int door=0;
int citerne=0;
int rgb=0;
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
 eclairage=(int)val;
  }

/*else if(strcmp(top,mqtt_topic_door)==0){
  if(auto_bigdoor==1)
 door=(int)val;
}*/
else if(strcmp(top,mqtt_topic_RGB)==0){
  rgb=(int)val;
}
else if(strcmp(top,mqtt_topic_citerne)==0){
 citerne=(int)val;
}
else if(strcmp(top,mqtt_topic_bigdoor_manuel)==0){
  if(auto_bigdoor==1){
  auto_bigdoor=0;
  temps_arret_bigdoor=time(NULL);
  door=(door==0)?1:0;
  }
  else
  auto_bigdoor=1;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(ecl, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(bleue, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(faible, OUTPUT);
  pinMode(moyen, OUTPUT);
  pinMode(bon, OUTPUT);
  pinMode(ledJ, OUTPUT);


  servo.attach(18);
  servo.write(0);
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
client.subscribe("garage/rgb");
client.subscribe("garage/citerne");
client.subscribe("garage/bigdoor");
client.subscribe("garage/eclairage");
client.subscribe("garage/door/manuel");
}

void loop() {
  if(time(NULL)-temps_arret_bigdoor>60){
  auto_bigdoor=1;
  door=0;
  }
  //eclairage 
  if(eclairage==1){
    digitalWrite(ecl, HIGH);
    }
    else {
    digitalWrite(ecl,LOW);
    }
   //bigdoor
  if(door==1){
    servo.write(90);
    digitalWrite(ledJ,HIGH);
    } else{
      if(door==0){
      servo.write(0);
      digitalWrite(ledJ,LOW);
      }
      }
rgb_function(rgb);
water_level(citerne);

client.loop();    
}
void rgb_function(int rgb){
  if(rgb==1){
    digitalWrite(red,LOW);
    digitalWrite(bleue,LOW);
    digitalWrite(green,HIGH);
    }
    else if(rgb==2){
    digitalWrite(red,HIGH);
    digitalWrite(bleue,LOW);
    digitalWrite(green,LOW);
    }
    else if(rgb==3){
    digitalWrite(red,LOW);
    digitalWrite(bleue,HIGH);
    digitalWrite(green,LOW);
    }
  }

 void water_level(int val){
  if(val==1){
     digitalWrite(faible,HIGH);
    digitalWrite(moyen,LOW);
    digitalWrite(bon,LOW);
    }
    else if(moyen==2){
    digitalWrite(faible,HIGH);
    digitalWrite(moyen,HIGH);
    digitalWrite(bon,LOW);
    }
    else if(bon==3){
     digitalWrite(faible,HIGH);
    digitalWrite(moyen,HIGH);
    digitalWrite(bon,HIGH);
    }
  }
