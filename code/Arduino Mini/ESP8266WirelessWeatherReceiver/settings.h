
// WIFI -settings
const char *SSID = "YOUR_SSID";
const char *PSK = "YOUR_PASSWORD";

// MQTT Settings
const char *MQTT_BROKER = "IP of MQTT Broker";
const int MQTT_PORT = 1880;

// Sleep settings for ESP8266
const int SLEEP_MIN = 3;

typedef struct
{
    char type[2];
    char temp[6];
    char spacer1;
    char pressure[6];
    char spacer2;
    char humidity[6];
    char spacer3;
    char windvalue[10];
    char spacer4;
    char rainvalue[10];
    char spacer5;
    char windpos[3];
    char spacer6;
    char batvalue[6];
    char spacer7;
    char solar[6];
} incomingmessage;