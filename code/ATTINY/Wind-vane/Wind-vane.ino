
// Code for the ATtiny24

#include <TinyWireS.h>
#define I2C_SLAVE_ADDRESS 0x5 // Address of the slave

#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

// pins of wind vane positions
#define pinPos1  9
#define pinPos2  10
#define pinPos3  8
#define pinPos4  1
#define pinPos5  7
#define pinPos6  2
#define pinPos7  5
#define pinPos8  3


const byte chksum1 = 200; // begin of transfer
const byte chksum2 = 251; // end of transfer

void reboot() {
  cli();
  WDTCSR = 0xD8 | WDTO_1S;
  sei();

  wdt_reset();
  while (true) {}

} //reboot

void setup()
{
  MCUSR &= ~(1 << WDRF); // reset status flag
  wdt_disable();

  TinyWireS.begin(I2C_SLAVE_ADDRESS); // join i2c network
  USICR |= 1 << USIWM0;
  TinyWireS.onReceive(receiveEvent);
  TinyWireS.onRequest(requestEvent);

  pinMode(pinPos1, INPUT_PULLUP);
  pinMode(pinPos2, INPUT_PULLUP);
  pinMode(pinPos3, INPUT_PULLUP);
  pinMode(pinPos4, INPUT_PULLUP);
  pinMode(pinPos5, INPUT_PULLUP);
  pinMode(pinPos6, INPUT_PULLUP);
  pinMode(pinPos7, INPUT_PULLUP);
  pinMode(pinPos8, INPUT_PULLUP);

} // setup


void sleep() {

  ADCSRA &= ~_BV(ADEN);                   // ADC off
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement

  sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sei();                                  // Enable interrupts
  sleep_cpu();                            // sleep

  cli();                                  // Disable interrupts
  sleep_disable();                        // Clear SE bit

  sei();                                  // Enable interrupts
} // sleep


void loop()
{
  // Everything is handeled by I2C request function
   tws_delay(10);

  TinyWireS_stop_check();
  // Go back to sleep
  sleep();

}

/*-------------------- Get Wind Vane Position ----------------------*/
uint8_t getWindVanePosition() {

  uint8_t pos = 0;

  if (digitalRead(pinPos1) == LOW)
    pos += 1;

  if (digitalRead(pinPos2) == LOW)
    pos += 2;

  if (digitalRead(pinPos3) == LOW)
    pos += 4;

  if (digitalRead(pinPos4) == LOW)
    pos += 8;

  if (digitalRead(pinPos5) == LOW)
    pos += 16;

  if (digitalRead(pinPos6) == LOW)
    pos += 32;

  if (digitalRead(pinPos7) == LOW)
    pos += 64;

  if (digitalRead(pinPos8) == LOW)
    pos += 128;

  return pos;
}

// Gets called when the ATtiny receives an i2c request
void requestEvent()
{

  uint8_t pos;
  pos = getWindVanePosition();
  TinyWireS.send(chksum1);
  TinyWireS.send(pos);
  TinyWireS.send(chksum2);
}



// Gets called when the ATtiny receives an i2c write slave request
void receiveEvent(uint8_t howMany)
{
  uint8_t  request = 0;

  // Transmission codes:
  // 3 = Reboot Sensor

  while (0 < TinyWireS.available()) {

    request = TinyWireS.receive();

    switch (request) {

      case 3:
        reboot();
        break;
    }
  }
}
