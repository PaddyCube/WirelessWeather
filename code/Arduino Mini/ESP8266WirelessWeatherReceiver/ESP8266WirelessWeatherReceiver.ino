#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "settings.h"
#include <RH_ASK.h>
#include <SPI.h> // may be not relevant at all

WiFiClient espClient;
PubSubClient client(espClient);

uint8_t msg[90]; // buffer for incoming messages

RH_ASK radioHead(2000, 5, 5, 0);

void setup()
{

  Serial.begin(19200);
  setup_wifi();
  client.setServer(MQTT_BROKER, MQTT_PORT);
  Serial.println("alive");
  if (!radioHead.init())
  {
    Serial.println("Failed to init 433MHz receiver");
  }
}

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, PSK);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Reconnecting...");
    if (!client.connect("ESP8266Client"))
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void loop()
{
  float lv_temp;
  float lv_pressure;
  float lv_humidity;
  unsigned int lv_wind;
  unsigned int lv_rain;
  unsigned int lv_battery;
  uint8_t buflen = sizeof(msg);

  char *messageparts[11];
  char *savepart;
  char *endpart;

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  if (radioHead.recv(msg, &buflen))
  {
    Serial.print("New message arrived: ");
    Serial.println((char *)msg);

    // move message to struct
    int fieldcount = 0;
    int charcount = 0;
    char fieldvalue[10];

    // Split imcoming message into parts
    for (int i = 0; i < 11; i++) {

      // this is the first run, so we need to call STRTOK_R with input char array
      if (i == 0) {
        messageparts[i] = strtok_r((char *)msg, "|", &savepart);
      } else {
        // this is a subsequent call of STTOK_R, so we need to call with NULL. This way  the function
        // use the same string like before
        messageparts[i] = strtok_r(NULL, "|", &savepart);
      }
      Serial.println(messageparts[i]);
    }

    endpart = strstr(messageparts[10], "END");

    if (strcmp(messageparts[0], "WW" ) == 0 && endpart )
    {
      Serial.println("message valid");
      //send by MQTT
      client.publish("/WirelessWeather/MessageID", messageparts[1]);
      client.publish("/WirelessWeather/Temperature", messageparts[2]);
      client.publish("/WirelessWeather/Pressure", messageparts[3]);
      client.publish("/WirelessWeather/Humidity", messageparts[4]);
      client.publish("/WirelessWeather/Wind", messageparts[5]);
      client.publish("/WirelessWeather/Rain", messageparts[6]);
      client.publish("/WirelessWeather/WindDir", messageparts[7]);
      client.publish("/WirelessWeather/Battery", messageparts[8]);
      client.publish("/WirelessWeather/Solar", messageparts[9]);
    }
    else
    {
      Serial.println("invalid message");
    }
  }
}
