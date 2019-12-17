#include <Adafruit_BMP280.h>
#include <Wire.h>
#include "WindVane.h"
#include "settings.h"
#include "LowPower.h"
#include <RH_ASK.h> // RadioHead
#include <SPI.h>    // maybe not needed

WindVane windvane;
Adafruit_BMP280 bme;

char temp[10];
char pressure[10];
char humidity[10];
char windvalue[10];
char rainvalue[10];
char windpos[3];
char batvalue[6];
char solar[6];
char msg[90]; // message to send
volatile unsigned long windcount = 0;
volatile unsigned long raincount = 0;
volatile unsigned long WakeupCount = 0;

RH_ASK radioHead(2000, 10, SendPin, false);

void setup()
{

  Serial.begin(19200);

  Serial.println("Initialize Wireless Weather on Arduino Pro Mini");
  // init I2C communication

  // enable interrupts for wind and rain count
  pinMode(WindPin, INPUT);
  pinMode(RainPin, INPUT);
  pinMode(SendPower, OUTPUT);
  pinMode(i2cPower, OUTPUT);
  delay(100);
}

void loop()
{
  float lv_temp;
  float lv_pressure;
  float lv_humidity;
  float lv_battery;
  float lv_solar;

  // check if we need to send data
  if (WakeupCount < (SLEEP_MIN * 60 / 8))
  {
    // no data needs to be send, go back to sleep
  }
  else
  {

    digitalWrite(SendPower, HIGH);
    digitalWrite(i2cPower, HIGH);
    delay(100);


    // prepare send data over 433MHz
    if (!radioHead.init())
      Serial.println("433MHz init failed");

    Wire.begin();                    // join i2c bus (address optional for master)
    // Wire.setClockStretchLimit(1500); // for some reason needed for ATTINY85
    windvane.begin();
    bme.begin(0x76);
    //  Wire.setClockStretchLimit(1500); // for some reason needed for ATTINY85


    WakeupCount = 0;
    // data needs to be send
    snprintf(msg, 50, "Alive since %ld milliseconds", millis());
    Serial.print("Publish message: ");
    Serial.println(msg);

    // Read data from BME280 multiple times to stabilize
    lv_temp = bme.readTemperature();
    delay(50);
    lv_temp = bme.readTemperature();
    delay(50);
    lv_temp = bme.readTemperature();

    lv_pressure = bme.readPressure();
    //    lv_humidity = bme.readHumidity();

    // convert results to char
    dtostrf(lv_temp, 6, 2, temp);
    dtostrf(lv_pressure, 6, 2, pressure);
    dtostrf(lv_humidity, 6, 2, humidity);

    // send Wind speed and rain count
    windvane.getCharPosition(windpos);

    itoa(windcount, windvalue, 10);
    itoa(raincount, rainvalue, 10);

    // battery level
    lv_battery = measureBattery();
    itoa(lv_battery * 100, batvalue, 10);

    // solar
    lv_solar = measureSolar();
    itoa(lv_solar * 100, solar, 10);

    //send data over 433 MHz
    sprintf(msg, "%s|%s|%s|%s|%s|%s|%s|%s|%s|END", "WW", temp, pressure, humidity, windvalue, rainvalue, windpos, batvalue, solar);
    Serial.println(msg);

    radioHead.send((uint8_t *)msg, strlen(msg));
    radioHead.waitPacketSent();

    // reset wind and rain counts
    windcount = 0;
    raincount = 0;

    delay(100); // wait to complete send

    digitalWrite(SendPower, LOW); // turn off external devices
    digitalWrite(i2cPower, LOW);
  }

  // attach Interrupts
  attachInterrupt(digitalPinToInterrupt(WindPin), onWindPulse, FALLING);
  attachInterrupt(digitalPinToInterrupt(RainPin), onRainPulse, FALLING);


  //   Sleep for x Minute
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);

  // back from sleep
  detachInterrupt(digitalPinToInterrupt(WindPin));
  detachInterrupt(digitalPinToInterrupt(RainPin));
  onTimerWake();
}

float measureSolar()
{

  // Voltage divider R1 = 1M and R2=560k
  float calib_factor = 9.4; // change this value to calibrate the battery voltage
  unsigned long raw = analogRead(A1);
  float volt = raw * calib_factor / 1024;
  return volt;
}

float measureBattery()
{

  // Voltage divider R1 = 1000k and R2=560k
  float calib_factor = 9.4; // change this value to calibrate the battery voltage
  unsigned long raw = analogRead(A0);
  float volt = raw * calib_factor / 1024;

  return volt;
}

void onWindPulse()
{
  windcount++;
  delay(2); // delay for debounce
}

void onRainPulse()
{
  raincount++;
  delay(2); // delay for debounce
}

void onTimerWake()
{
  WakeupCount++;
}
