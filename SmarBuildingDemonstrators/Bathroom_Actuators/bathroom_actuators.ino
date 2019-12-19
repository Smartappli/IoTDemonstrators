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
// WiFi
// Make sure to update this for your own WiFi network!
const char* ssid = "IoTWifi";
const char* wifi_password = "SmartBuilding";

// MQTT
// Make sure to update this for your own MQTT Broker!
//const char* mqtt_server = "192.168.0.50";
const char* mqtt_server = "192.168.4.1";
const char* mqtt_topic_Buzzer = "sdb/Buzzer";
const char* mqtt_topic_Ventilo = "sdb/Ventilo";
const char* mqtt_topic_eclairage = "sdb/eclairage";
const char* mqtt_topic_eclairage_manuel = "sdb/eclairage/manuel";

// The client id identifies the ESP8266 device. Think of it a bit like a hostname (Or just a name, like Greg).
const char* clientID = "esp32_SdB";

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, wifiClient); // 1883 is the listener port for the Broker



// Declaration des variables

#define ANALOG_PIN_7 35
int pinVentilo = 18;
int pinLaser = 25;
int pinBuzzer = 15;
int eclairage=0;
int auto_eclairage=1;
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
else if(strcmp(top,mqtt_topic_Buzzer)==0){
  buzzer_alarm((int)val);
}

else if(strcmp(top,mqtt_topic_Ventilo)==0){
  ventilo((int)val);
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

  // Connect to MQTT Broker
  // client.connect returns a boolean value to let us know if the connection was successful.
  if (client.connect(clientID)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
  client.setCallback(callback);

  
  pinMode(pinLaser, OUTPUT);
  pinMode(pinVentilo, OUTPUT);
  pinMode(pinBuzzer, OUTPUT);
  pinMode(12,OUTPUT);
  client.subscribe("sdb/eclairage");
  client.subscribe("sdb/Buzzer");
  client.subscribe("sdb/Ventilo");
  client.subscribe("sdb/eclairage/manuel");
}


void loop() {
  if(time(NULL)-temps_arret_ecl>60)
  auto_eclairage=1;
   if(eclairage==1){
    digitalWrite(12, HIGH);
    }
    else {
    digitalWrite(12,LOW);
    }
client.loop();
}
  //Passive Buzzer
void buzzer_alarm(int buzzer){
  if (buzzer==1){
 unsigned char i, j ;
 Serial.println("Buzz");
  digitalWrite (pinBuzzer, HIGH) ; //send tone
      delay (1) ;
    for (i = 0; i <80; i++) // When a frequency sound
    {
      
      digitalWrite (pinBuzzer, LOW) ; //no tone
      delay (1) ;
    }
    for (i = 0; i <100; i++) // When a frequency sound
    {
      digitalWrite (pinBuzzer, HIGH) ; //send tone
      delay (2) ;
      digitalWrite (pinBuzzer, LOW) ; //no tone
      delay (2) ;
    }
  }
}


//ventilo
void ventilo(int val){
  
  if(val==1){
    digitalWrite(pinLaser, HIGH);
    digitalWrite(pinVentilo, HIGH);
    }
    else { 
    digitalWrite(pinLaser, LOW);
    digitalWrite(pinVentilo, LOW);
      }
  }
