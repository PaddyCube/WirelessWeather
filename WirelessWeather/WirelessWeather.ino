#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#include "WindSpeed.h"
#include "Battery.h"
#include "WindVane.h"

WindSpeed wind;
Battery battery;
WindVane windvane;

Adafruit_BME280 bme;

WiFiClient espClient;
PubSubClient client(espClient);

const char* SSID = "YOUR-SSID";
const char* PSK = "YourPassword";
const char* MQTT_BROKER = "192.168.1.1";



long lastMsg = 0;
char msg[50];
int value = 0;
int pinVCC = 15;
char temp[10];
char pressure[10];
char humidity[10];
char windvalue[10];
char rainvalue[10];
char windpos[4];
char batvalue[10];

void setup() {
  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(MQTT_BROKER, 1883);

  Wire.begin(); // join i2c bus (address optional for master)
  Wire.setClockStretchLimit(1500); // for some reason needed for ATTINY85

  wind.begin();
  battery.begin();
  windvane.begin();
  Serial.println(bme.begin(0x76));
  Wire.setClockStretchLimit(1500); // for some reason needed for ATTINY85
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, PSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Reconnecting...");
    if (!client.connect("ESP8266Client")) {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

float measureVoltage() {

  // Voltage divider R1 = 220k+100k+220k =540k and R2=100k
  float calib_factor = 5.6; //5.28; // change this value to calibrate the battery voltage
  unsigned long raw = analogRead(A0);
  float volt = raw * calib_factor / 1024;

  Serial.print( " m. \nVoltage = ");
  Serial.print(volt, 2); // print with 2 decimal places
  Serial.println (" V");

  return volt;

}
void loop() {

  float lv_temp;
  float lv_pressure;
  float lv_humidity;
  unsigned int lv_wind;
  unsigned int lv_rain;
  unsigned int lv_battery;

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  snprintf (msg, 50, "Alive since %ld milliseconds", millis());
  Serial.print("Publish message: ");
  Serial.println(msg);

  // Read data from BME280
  lv_temp = bme.readTemperature();
  lv_pressure = bme.readPressure();
  lv_humidity = bme.readHumidity();


  // convert results to char
  dtostrf(lv_temp, 6, 2, temp);
  dtostrf(lv_pressure, 6, 2, pressure);
  dtostrf(lv_humidity, 6, 2, humidity);

  //send my MQTT
  client.publish("/WirelessWeather/Temperature", temp );
  client.publish("/WirelessWeather/Pressure", pressure );
  client.publish("/WirelessWeather/Humidity", humidity );


  delay(20);
  // send Wind speed and rain count
  lv_wind = wind.getSpeed(false);
  lv_rain = wind.getRain(true);
  windvane.getCharPosition(windpos);

  itoa(lv_wind, windvalue, 10);
  itoa(lv_rain, rainvalue, 10);
  client.publish("/WirelessWeather/Wind", windvalue );
  client.publish("/WirelessWeather/Rain", rainvalue );
  client.publish("/WirelessWeather/WindDir", windpos );


  delay(20);
  // battery level
  lv_battery = battery.getBattery(false);
  itoa(lv_battery, batvalue, 10);
  client.publish("/WirelessWeather/Battery", batvalue);


  //  dtostrf(measureVoltage(), 6, 2, msg);
  //  client.publish("/WirelessWeather/Battery", msg);

  // Sleep for 3 Minute
  //  ESP.deepSleep(3 * 60 * 1000000);

  delay(5000);



}




