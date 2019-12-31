// array of different xiaomi flora MAC addresses
char* FLORA_DEVICES[] = {
    "C4:7C:8D:67:11:11", 
    "C4:7C:8D:67:22:22", 
    "C4:7C:8D:67:33:33" };

// sleep between to runs in seconds
#define SLEEP_DURATION 30 * 60
// emergency hibernate countdown in seconds
#define EMERGENCY_HIBERNATE 3 * 60
// how often should the battery be read - in run count
#define BATTERY_INTERVAL 6
// how often should a device be retried in a run when something fails
#define RETRY 3

const char*   WIFI_SSID       = "telenet-44602"; //IoTWifi
const char*   WIFI_PASSWORD   = "JWYtTUdwu0ap"; //SmartBuilding

// MQTT topic gets defined by "<MQTT_BASE_TOPIC>/<MAC_ADDRESS>/<property>"
// where MAC_ADDRESS is one of the values from FLORA_DEVICES array
// property is either temperature, moisture, conductivity, light or battery

const char*   MQTT_HOST       = "192.168.0.123";
const int     MQTT_PORT       = 1883;
const char*   MQTT_CLIENTID   = "miflora-client";
const char*   MQTT_CLIENTID2  = "esp32";
const char*   MQTT_USERNAME   = "";
const char*   MQTT_PASSWORD   = "";
const String  MQTT_BASE_TOPIC = "flora"; 
const int     MQTT_RETRY_WAIT = 5000;
