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
#include <LiquidCrystal_I2C.h>
#include <SevSeg.h>
#include <Wire.h>
 
// WiFi
// Make sure to update this for your own WiFi network!
const char* ssid = "IoTWifi";
const char* password ="SmartBuilding";
const char* mqttServer = "192.168.4.1";
const int mqttPort = 1883;

// MQTT
const char* mqtt_topic_time = "Home/time";
const char* mqtt_topic_horloge_seconde = "Home/seconde";
const char* mqtt_topic_temperature = "bedroom/temperature";
const char* mqtt_topic_humidite = "bedroom/humidite";
const char* mqtt_topic_window_state = "bedroom/window/state";
const char* mqtt_topic_eclairage = "bedroom/eclairage";
const char* mqtt_topic_eclairage_manuel = "bedroom/eclairage/manuel";
// The client id identifies the ESP8266 device. Think of it a bit like a hostname (Or just a name, like Greg).
const char* clientID = "esp32_bedroom_act";

// Initialise the WiFi and MQTT Client objects
WiFiClient espClient;
PubSubClient client(mqttServer,1883,espClient); // 1883 is the listener port for the Broker
//Servo servo;
SevSeg sevseg;
int temperature=0;
int humidite=0;
int temps=0;
int seconde_t=0;
int fenetre_ouverte = 0;
int eclairage=0;
int lcdColumns = 16;
int lcdRows = 2;
int LEDR = 2;
int LEDJ = 23;
int LEDB = 15;
int ecl=12;
int temps_arret_ecl=0;
int auto_eclairage=1;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 
void affichage_temphum(int temperature, int humidite){
  
  Serial.println("la valeur de la temp et hum est :");
     Serial.println(temperature);
      Serial.println(humidite);
    // set cursor to first column, first row
    lcd.setCursor(0, 0);
    // print message
    lcd.clear();
    lcd.print("T ");
    lcd.print((int)temperature);
    lcd.print(" *C ");
    lcd.print("H ");
    lcd.print((int)humidite);
    lcd.print(" % ");
    
   // set cursor to first column, second row
    lcd.setCursor(0,1);
    if((int)temperature <= 23){
      lcd.setCursor(0,1);
      lcd.print("Mettez une veste");
    }
    else if((int)temperature > 23){
      lcd.setCursor(0,1);
      lcd.print("Mettez T-shirt");
    } 
  }
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
/*else if(strcmp(top,mqtt_topic_window_state)==0){
fenetre_ouverte=(int)val;
  }*/
else if(strcmp(top,mqtt_topic_temperature)==0){
  if(val!=temperature){
  temperature=(int)val;
 affichage_temphum(temperature,humidite);
  }
}
else if(strcmp(top,mqtt_topic_humidite)==0){
  if(val!=humidite){
  humidite=(int)val;
  affichage_temphum(temperature,humidite);
  }
}
else if(strcmp(top,mqtt_topic_time)==0){
  temps=(int)val;
}
else if(strcmp(top,mqtt_topic_horloge_seconde)==0){
  seconde_t=(int)val;
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
//**********************************************************************************************
void setup() {
  Serial.begin(115200);
    delay(1000); // give me time to bring up serial monitor
  //servo.attach(13);
  Serial.println("ESP32 Analog IN Test");

  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Connect to the WiFi
  WiFi.begin(ssid, password);

  // Wait until the connection has been confirmed before continuing
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Debugging - Output the IP Address of the ESP8266
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 
    client.setServer(mqttServer,1883);
if (client.connect(clientID)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }

  client.setCallback(callback);
  client.subscribe("bedroom/temperature");
  client.subscribe("bedroom/humidite");
  client.subscribe("bedroom/window/state");
  client.subscribe("bedroom/eclairage");
  client.subscribe("Home/time");
  client.subscribe("Home/seconde");
  client.subscribe("bedroom/eclairage/manuel");
// initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
  Serial.println("Temp and hum");

   byte numDigits = 4;
   byte digitPins[] = {19, 18, 13, 4};
   byte segmentPins[] = {27, 26, 33, 16, 17, 14, 25, 32};
   sevseg.begin(COMMON_ANODE, numDigits, digitPins, segmentPins);
  //Set the desired brightness (0 to 100);
  sevseg.setBrightness(90);


  /* Initialise le port I2C */
  Wire.begin();
  pinMode(ecl,OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDJ, OUTPUT);
  pinMode(LEDB, OUTPUT);

}

void loop() {
  client.setServer(mqttServer,1883);
  if (client.connect(clientID)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
    client.setCallback(callback);

 //affichage_temphum(temperature, humidite);
 //horloge(temps);
 seconde((int)seconde_t);
if (time(NULL)-temps_arret_ecl>60)
auto_eclairage=1;
//ouverture window
/* if(fenetre_ouverte==1 ){
    servo.write(90);
    //temp=0;
    } else{
     
      servo.write(0);
      //temp=1;}   
}*/
//eclairage
if(eclairage==1){
    digitalWrite(ecl, HIGH);
    }
    else {
    digitalWrite(ecl,LOW);
    }
client.loop();    
}

 void horloge(int Time){
  sevseg.setNumber(Time);
  sevseg.refreshDisplay();
  }
void seconde(int seconde){
  
  if (seconde < 40){
    if (seconde > 20){
      digitalWrite(LEDJ, LOW);
      digitalWrite(LEDR, LOW);
      digitalWrite(LEDB, HIGH);
      
    } else {
      digitalWrite(LEDJ, HIGH);
      digitalWrite(LEDR, LOW);
      digitalWrite(LEDB, LOW);
    }
  }
  if (seconde > 40){
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDJ, LOW);
    digitalWrite(LEDB, LOW);
  }
  }
  
