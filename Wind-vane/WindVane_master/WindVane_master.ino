//Code for the Arduino Mega<
#include <Wire.h>
#include "WindVane.h"


WindVane wind;

void setup()
{
  Wire.begin(); // join i2c bus (address optional for master)
  Serial.begin(9600); // start serial for output

  Serial.println("alive");
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

wind.begin();

}

void loop()
{
  Serial.println("loop");

 Serial.println(wind.getCharPosition());

  delay(2000);
}



