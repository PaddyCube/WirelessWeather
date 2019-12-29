
// Code for the ATtiny85

#include <TinyWireS.h>
#define I2C_SLAVE_ADDRESS 0x3 // Address of the slave

#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#define ADC_PIN A3

const byte chksum1 = 200; // begin of transfer
const byte chksum2 = 251; // end of transfer

volatile int senddata = 1;

volatile byte reg_position = 0;    // counter for i2c send position
volatile unsigned int battery_raw; // raw value of ADC

// byte array representing battery_raw for i2c communication
volatile uint8_t i2c_regs[] =
{
  0, //older 8
  0 //younger 8
};

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

  analogReference(DEFAULT);

  pinMode(ADC_PIN, INPUT);

} // setup


void sleep() {

  ADCSRA &= ~_BV(ADEN);                   // ADC off
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement

  sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sei();                                  // Enable interrupts
  sleep_cpu();                            // sleep

  cli();                                  // Disable interrupts
  sleep_disable();                        // Clear SE bit
  ADCSRA |= _BV(ADEN);                    // ADC on

  sei();                                  // Enable interrupts
} // sleep

void loop()
{

  tws_delay(10);
    TinyWireS_stop_check();
    
  // read analog value of battery
  battery_raw = analogRead(ADC_PIN);
  if (battery_raw == 0)
    battery_raw = 1;

  // Shift read value to i2c register
  i2c_regs[0] = battery_raw >> 8;
  i2c_regs[1] = battery_raw & 0xFF;

  // Go back to sleep

  sleep();

}

// Gets called when the ATtiny receives an i2c request
void requestEvent()
{

  TinyWireS.send(chksum1);

  reg_position = 0;
  TinyWireS.send(i2c_regs[reg_position]);

  reg_position++;
  TinyWireS.send(i2c_regs[reg_position]);

  TinyWireS.send(chksum2);
}

// Gets called when the ATtiny receives an i2c write slave request
void receiveEvent(uint8_t howMany)
{

  uint8_t  request = 0;

  while (0 < TinyWireS.available()) {

    // Transmission codes:
    // 3 = Reboot Sensor

    request = TinyWireS.receive();

    switch (request) {

      case 3:
        reboot();
        break;
    }
  }
}


