#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "settings.h"
#include <RH_ASK.h>
#include <SPI.h> // may be not relevant at all

WiFiClient espClient;
PubSubClient client(espClient);

uint8_t msg[63]; // buffer for incoming messages
incomingmessage * message;

RH_ASK radioHead;

void setup()
{

  Serial.begin(115200);
  setup_wifi();
  client.setServer(MQTT_BROKER, MQTT_PORT);

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
    message = reinterpret_cast<incomingmessage*>((char *)&msg);

    if (message->type == "WW")
    {
      //send by MQTT
      client.publish("/WirelessWeather/Temperature", message->temp);
      client.publish("/WirelessWeather/Pressure", message->pressure);
      client.publish("/WirelessWeather/Humidity", message->humidity);
      client.publish("/WirelessWeather/Wind", message->windvalue);
      client.publish("/WirelessWeather/Rain", message->rainvalue);
      client.publish("/WirelessWeather/WindDir", message->windpos);
      client.publish("/WirelessWeather/Battery", message->batvalue);
      client.publish("/WirelessWeather/Solar", message->solar);
    }
    else
    {
      Serial.println("invalid message");
    }
  }
}
