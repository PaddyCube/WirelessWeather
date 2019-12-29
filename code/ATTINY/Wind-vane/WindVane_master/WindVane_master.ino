//Code for the Arduino Mega<
#include <Wire.h>
#include "WindVane.h"
#include "Battery.h"

char windpos[4];

WindVane wind;
Battery bat;

void setup()
{
  Wire.begin(); // join i2c bus (address optional for master)
Wire.setClockStretchLimit(1500); 
  Serial.begin(9600); // start serial for output

  Serial.println("alive");
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

wind.begin();
bat.begin();

}

void loop()
{
  Serial.println("loop");
  Serial.println(bat.getBattery());
  wind.getCharPosition(windpos);
 Serial.println(windpos);

  delay(2000);
}



