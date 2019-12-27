//CJMCU-8128 CCS811 + HDC1080 + BMP280 (banggood.com)

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"
#include <SparkFunCCS811.h>     
#include <SparkFunBME280.h>     
#include <ClosedCube_HDC1080.h> 

//#define CCS811_ADDR 0x5B  //Default I2C Address
#define CCS811_ADDR 0x5A    //Alternate I2C Address

long lastMsg = 0;   

const char* ssid = "IoTWifi";
const char* password = "SmartBuilding";
const char* mqtt_server = "192.168.4.1";
//const char* thingboard_server = "thingsboard.ig.umons.ac.be";
//const char* thingboard_token = "fsLDuFIUHgiKuo2PoT7H";
const char* thingboard_server = "demo.thingsboard.io";
const char* thingboard_token = "fsLDuFIUHgiKuo2PoT7H";
const char* mqtt_topic_temperature = "umons/iglab/temperature";
const char* mqtt_topic_humidity = "umons/iglab/humidity";
const char* mqtt_topic_co2 = "umons/iglab/co2";
const char* mqtt_topic_voc = "umons/iglab/voc";
const char* mqtt_topic_pressure = "umons/iglab/pressure";
const char* mqtt_topic_altitude = "umons/iglab/altitude";
const char* mqtt_topic_lux = "umons/iglab/lux";
const char* mqtt_topic_ir = "umons/iglab/ir";
const char* mqtt_topic_full = "umons/iglab/full";
const char* mqtt_topic_moisture1 = "umons/iglab/visible";
const char* clientID = "esp32_iglab";

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
CCS811 myCCS811(CCS811_ADDR);
ClosedCube_HDC1080 myHDC1080;
BME280 myBME280;

WiFiClient espClient;
PubSubClient client1(mqtt_server, 1883, espClient);
PubSubClient client2(thingboard_server, 951, espClient);

float hum_weighting = 0.25; // so hum effect is 25% of the total air quality score
float gas_weighting = 0.75; // so gas effect is 75% of the total air quality score

int   humidity_score, gas_score;
float gas_reference = 2500;
float hum_reference = 40;
int   getgasreference_count = 0;
int   gas_lower_limit = 10000;  // Bad air quality limit
int   gas_upper_limit = 300000; // Good air quality limit

void advancedRead(uint16_t &ir, uint16_t &full, float &visible, float &lux)
{
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  int val=0;
  // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
  // That way you can do whatever math and comparisons you want!
  uint32_t lum = tsl.getFullLuminosity();
  //uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  visible = full-ir;
  lux = tsl.calculateLux(full, ir);
  Serial.print(F("[ ")); Serial.print(millis()); Serial.print(F(" ms ] "));
  Serial.print(F("IR: ")); Serial.print(ir);  Serial.print(F("  "));
  Serial.print(F("Full: ")); Serial.print(full); Serial.print(F("  "));
  Serial.print(F("Visible: ")); Serial.print(visible); Serial.print(F("  "));
  Serial.print(F("Lux: ")); Serial.println(lux, 6);
}

void send_to_mqttbroker(const char* topic, float donnee){
    char temp[16];
    //convertion vers un tableau de char 
    itoa((int)donnee, temp, 10);
    if (!client1.publish(topic, temp)){
        Serial.println("Message failed to send. Reconnecting to MQTT Broker and trying again");
        client1.connect(clientID);
        delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
        client1.publish(topic, temp);
    }
    Serial.print("la valeur envoyé pour le topic: ");
    Serial.print(topic);
    Serial.print(" ");
    Serial.println(temp);
}

void send_to_thingsboard(const char* topic, float donnee) {
    
}

void reconnect() {
  while (!client1.connected()) {
    Serial.print("Connexion au Broker MQTT...");
    if (client1.connect(clientID)) {
      Serial.println("OK");
    } else {
      Serial.print("KO, erreur : ");
      Serial.print(client1.state());
      Serial.println(" On attend 5 secondes avant de recommencer");
      delay(5000);
    }
  }
}

void reconnect2() {  
   while (!client2.connected()) {
    Serial.print("Connexion au serveur Thingboard...");
    if (client2.connect(clientID, thingboard_token, NULL)) {
      Serial.println("OK");
    } else {
      Serial.print("KO, erreur : ");
      Serial.print(client2.state());
      Serial.println(" On attend 5 secondes avant de recommencer");
      delay(5000);
    }
  } 
}

void setup()
{
  Serial.begin(115200);

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

  // Connect to MQTT Broker
  // client.connect returns a boolean value to let us know if the connection was successful.
  if (client1.connect(clientID)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
  
  Serial.println("CJMCU-8128 = CCS811 + HDC1080 + BMP280 - TSL2591");
  
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = 0x76;
  myBME280.settings.runMode = 3; //Normal mode
  myBME280.settings.tStandby = 0;
  myBME280.settings.filter = 4;
  myBME280.settings.tempOverSample = 5;
  myBME280.settings.pressOverSample = 5;
  myBME280.settings.humidOverSample = 5;

  tsl.begin();
  myBME280.begin();
  myHDC1080.begin(0x40);

  //It is recommended to check return status on .begin(), but it is not required.
  CCS811Core::status returnCode = myCCS811.begin();
  
  if (returnCode != CCS811Core::SENSOR_SUCCESS)
  {
    Serial.println(".begin() returned with an error.");
    while (1);      //No reason to go further
  }
  
  Serial.println("End Initialization");
}

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
}

void loop()
{
  if (!client1.connected()) {
    reconnect();
  }
  client1.loop();

  if (!client2.connected()) {
    reconnect2();
  }
  client2.loop();
 
  long now = millis();
   
  //Check if data is ready
  if (myCCS811.dataAvailable())
  {
    lastMsg = now;
    //If so, have the sensor read and calculate the results.
    //Get them later
    myCCS811.readAlgorithmResults();

    float pression = myBME280.readFloatPressure() * 0.00750062;
    Serial.print("P[");
    Serial.print(pression, 2);
    Serial.print("mmHg] ");
    send_to_mqttbroker(mqtt_topic_pressure, pression);

    float altitude = myBME280.readFloatAltitudeMeters();
    Serial.print("Alt[");
    Serial.print(altitude, 2);
    Serial.print("m] ");
    send_to_mqttbroker(mqtt_topic_altitude, altitude);

    float temperature = myHDC1080.readTemperature();
    Serial.print("Temp HDC1080[");
    Serial.print(temperature);
    send_to_mqttbroker(mqtt_topic_temperature, temperature);

    float humidity = myHDC1080.readHumidity();
    Serial.print("C] RH HDC1080[");
    Serial.print(humidity);
    send_to_mqttbroker(mqtt_topic_humidity, humidity);

    float co2 = myCCS811.getCO2();
    Serial.print("%] CO2 CCS811[");
    Serial.print(co2);  //Returns calculated CO2 reading
    send_to_mqttbroker(mqtt_topic_co2, co2);

    float voc = myCCS811.getTVOC();
    Serial.print("] tVOC CCS811[");
    Serial.print(voc); //Returns calculated TVOC reading
    send_to_mqttbroker(mqtt_topic_voc, voc);
    Serial.print("] sec[");
    
    Serial.print(millis()/1000); //seconds since start
    Serial.print("]");
    Serial.println();
    
    //compensating the CCS811 with humidity and temperature readings from the HDC1080
    myCCS811.setEnvironmentalData(humidity, temperature); 

    float lux = 0;
    advancedRead();
    
    Serial.println(lux);
    send_to_mqttbroker(mqtt_topic_lux, lux);

    // Prepare a JSON payload string
    String payload = "{";
    payload += "\"Pression\":";
    payload += pression;
    payload += ",";
    payload += "\"Altitude\":";
    payload += altitude;
    payload += ",";
    payload += "\"Temperature\":";
    payload += temperature;
    payload += ","; 
    payload += "\"Humidity\":";
    payload += humidity; 
    payload += ",";
    payload += "\"CO2\":";
    payload += co2;
    payload += ",";
    payload += "\"VOC\";";
    payload += voc;
    payload += "}";
  
    char attributes[1000]; 
    payload.toCharArray( attributes, 1000 );
    client2.publish( "v1/devices/me/telemetry",attributes );
    Serial.println( attributes );
  } else {
    Serial.println("Préparation des capteurs en cours.");
  }

  delay(60000); //Don't spam the I2C bus
}
