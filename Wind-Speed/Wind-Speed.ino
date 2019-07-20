
// Code for the ATtiny85

#include <TinyWireS.h>
#define I2C_SLAVE_ADDRESS 0x4 // Address of the slave

#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

// pins of wind vane positions
const int pinWindCount = 3;
const int pinRainCount = 4;

const byte chksum1 = 200; // begin of transfer
const byte chksum2 = 251; // end of transfer

uint8_t windcount1 = 0;
uint8_t windcount2 = 0; // Overflow variable 1
uint8_t windcount3 = 0; // Overflow variable 2
uint8_t raincount1 = 0;
uint8_t raincount2 = 0; // Overflow variable 1
uint8_t raincount3 = 0; // Overflow variable 2
volatile bool thiswind;
volatile bool lastwind = HIGH;
volatile bool thisrain;
volatile bool lastrain = HIGH;

void reboot() {
  cli();
  WDTCR = 0xD8 | WDTO_1S;
  sei();

  wdt_reset();
  while (true) {}

} //reboot

void setup()
{
  MCUSR &= ~(1 << WDRF); // reset status flag
  wdt_disable();

  TinyWireS.begin(I2C_SLAVE_ADDRESS); // join i2c network
  USICR |= 1 << USIWM0; // Clock stretching?
  TinyWireS.onRequest(requestEvent);
  TinyWireS.onReceive(receiveEvent);

  pinMode(pinWindCount, INPUT_PULLUP);
  digitalWrite(pinWindCount, HIGH);

  pinMode(pinRainCount, INPUT_PULLUP);
  digitalWrite(pinRainCount, HIGH);
} // setup


void sleep() {

  GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT3);                   // Use PB3 as interrupt pin
  PCMSK |= _BV(PCINT4);                   // Use PB4 as interrupt pin

  ADCSRA &= ~_BV(ADEN);                   // ADC off
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement

  sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sei();                                  // Enable interrupts
  sleep_cpu();                            // sleep

  cli();                                  // Disable interrupts
  PCMSK &= ~_BV(PCINT3);                  // Turn off PB3 as interrupt pin
  PCMSK &= ~_BV(PCINT4);                  // Turn off PB4 as interrupt pin
  sleep_disable();                        // Clear SE bit
  ADCSRA |= _BV(ADEN);                    // ADC on
  //WDTCR |= _BV(WDIE);                    // Watchdog on

  sei();                                  // Enable interrupts
} // sleep


ISR(PCINT0_vect) {
  // DO NOTHING HERE, CATCH EVERYTHING IN LOOP
}



void loop()
{

  // Check wind count
  thiswind = digitalRead(pinWindCount);
  if (thiswind != lastwind) {  //check if value has changed since last time
    if (thiswind == HIGH)  {  //value has changed - was it rising edge?

      // do we get an overflow at windcount 1?
      if (windcount1 == 255 )
      {
        // do we get an overflow on windcount 2?
        if (windcount2 == 255)
        {
          windcount3++;
        }
        windcount2++;
      }
      windcount1++;
    }
    lastwind = thiswind; //switch state ready for next time
  }

  // Check rain count
  thisrain = digitalRead(pinRainCount);
  if (thisrain != lastrain) {  //check if value has changed since last time
    if (thisrain == HIGH)  {  //value has changed - was it rising edge?

      // do we get an overflow at windcount 1?
      if (raincount1 == 255 )
      {
        // do we get an overflow on windcount 2?
        if (raincount2 == 255)
        {
          raincount3++;
        }
        raincount2++;
      }
      raincount1++;
    }
    lastrain = thisrain; //switch state ready for next time
  }
  // Go back to sleep
  tws_delay(1); // wait 2ms for software debouncing
  sleep();

}



// Gets called when the ATtiny receives an i2c request
void requestEvent()
{

  TinyWireS.send(chksum1);
  TinyWireS.send(windcount1);
  TinyWireS.send(windcount2);
  TinyWireS.send(windcount3);
  TinyWireS.send(raincount1);
  TinyWireS.send(raincount2);
  TinyWireS.send(raincount3);
  TinyWireS.send(chksum2);
}

// Gets called when the ATtiny receives an i2c write slave request
void receiveEvent(uint8_t howMany)
{

  uint8_t
  request = 0;
  // cli();
  while (0 < TinyWireS.available()) {

    // Transmission codes:
    // 1 = Reset Rain Sensor
    // 2 = Reset Wind Sensor
    // 3 = Reboot Rain Sensor

    request = TinyWireS.receive();

    switch (request) {
      case 1:
        raincount1 = 0;
        raincount2 = 0;
        raincount3 = 0;
        break;

      case 2:
        windcount1 = 0;
        windcount2 = 0;
        windcount3 = 0;
        break;

      case 3:
        reboot();
        break;
    }
  }
  // sei();
}

