#include <Adafruit_BME280.h>
#include <Wire.h>
#include "WindVane.h"
#include "settings.h"
#include <LowPower.h>
#include <RH_ASK.h> // RadioHead
#include <SPI.h>    // maybe not needed

WindVane windvane;
Adafruit_BME280 bme;
RH_ASK radioHead;

char temp[6];
char pressure[6];
char humidity[6];
char windvalue[10];
char rainvalue[10];
char windpos[3];
char batvalue[6];
char solar[6];
char msg[63]; // message to send
volatile unsigned long windcount = 0;
volatile unsigned long raincount = 0;
volatile unsigned long WakeupCount = 0;

void setup()
{

  Serial.begin(115200);

  // init I2C communication
  Wire.begin();                    // join i2c bus (address optional for master)
  Wire.setClockStretchLimit(1500); // for some reason needed for ATTINY85
  windvane.begin();
  Serial.println(bme.begin(0x76));
  Wire.setClockStretchLimit(1500); // for some reason needed for ATTINY85
  Serial.println("Setup");

  // enable interrupts for wind and rain count
  pinMode(WindPin, INPUT);
  pinMode(RainPin, INPUT);

  // prepare send data over 433MHz
  radioHead = RH_ASK(2000, 11, SendPin, false);
  if (!radioHead.init())
    Serial.println("433MHz init failed");
}

void loop()
{
  float lv_temp;
  float lv_pressure;
  float lv_humidity;
  unsigned int lv_wind;
  unsigned int lv_rain;
  unsigned int lv_battery;
  unsigned int lv_solar;

  // check if we need to send data
  if (WakeupCount < (SLEEP_MIN * 60 / 8))
  {
    // no data needs to be send, go back to sleep
    Serial.println("nothing to do, go back to sleep");
  }
  else
  {
    // data needs to be send
    snprintf(msg, 50, "Alive since %ld milliseconds", millis());
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

    // send Wind speed and rain count
    windvane.getCharPosition(windpos);

    itoa(lv_wind, windvalue, 10);
    itoa(lv_rain, rainvalue, 10);

    // battery level
    lv_battery = measureBattery();
    itoa(lv_battery, batvalue, 10);

    // solar
    lv_solar = measureSolar();
    itoa(lv_solar, solar, 10);

    //send data over 433 MHz
    msg = "WW" + temp + '|' + pressure + '|' + humidity + '|' + windvalue + '|' + rainvalue + '|' +
          windpos + '|' + batvalue + '|' + solar;

    radioHead.send((uint8_t *)msg, strlen(msg));

    // reset wind and rain counts
    windcount = 0;
    raincount = 0;
  }

  // attach Interrupts
  attachInterrupt(digitalPinToInterrupt(WindPin), onWindPulse, FALLING);
  attachInterrupt(digitalPinToInterrupt(RainPin), onWindPulse, FALLING);

  //   Sleep for x Minute
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);

  // back from sleep
  detachInterrupt();
  onTimerWake();
}

float measureSolar()
{

  // Voltage divider R1 = 220k+100k+220k =540k and R2=100k
  float calib_factor = 18.3; //5.28; // change this value to calibrate the battery voltage
  unsigned long raw = analogRead(A0);
  float volt = raw * calib_factor / 1024;

  Serial.print(" m. \nVoltage Solar = ");
  Serial.print(volt, 2); // print with 2 decimal places
  Serial.println(" V");

  return volt;
}

float measureBattery()
{

  // Voltage divider R1 = 220k+100k+220k =540k and R2=100k
  float calib_factor = 18.3; //5.28; // change this value to calibrate the battery voltage
  unsigned long raw = analogRead(A1);
  float volt = raw * calib_factor / 1024;

  Serial.print(" m. \nVoltage Battery = ");
  Serial.print(volt, 2); // print with 2 decimal places
  Serial.println(" V");

  return volt;
}

void onWindPulse()
{
  windcount++;
  delay(2); // delay for debounce
}

void onWindPulse()
{
  raincount++;
  delay(2); // delay for debounce
}

void onTimerWake()
{
  WakeupCount++;
}
