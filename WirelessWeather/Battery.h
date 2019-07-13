/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
#define BATTERY_ADDRESS                (0x3)
/*=========================================================================*/

/*--- Class Definition  */
class Battery {

  public:
    Battery(void);
    bool  begin(uint8_t addr = BATTERY_ADDRESS);
    unsigned int getBattery(boolean buff_read = false);
    unsigned int getByte();

  private:
    void i2c_read();
    uint8_t   _i2caddr;
    unsigned int battery_raw = 0;
};

/*------------ Class Implementation ------*/

//----------------------------
// Class Constructor
//----------------------------
Battery::Battery()
{ }

//----------------------------------
// Set I2C addres of Battery monitor
//----------------------------------
bool Battery::begin(uint8_t a)
{
  _i2caddr = a;

  return true;
}

//----------------------------------
// Get battery voltage integer
//----------------------------------
unsigned int Battery::getBattery(boolean buff_read)
{
  unsigned int battery_vcc = 0;

  if (buff_read == false)
  {
    i2c_read();
  }

  battery_vcc = battery_raw * 1;
  return battery_vcc;

}

//----------------------------------
// Read single byte
//----------------------------------
unsigned int Battery::getByte()
{
  Wire.requestFrom(_i2caddr, 4);
  while (Wire.available())
  {
    Serial.println(Wire.read());
  }
  return 1;
}

//----------------------------------
// Read i2c Data
//----------------------------------
void Battery::i2c_read()
{
  unsigned int startdata = 0;
  unsigned int enddata = 0;
  byte received_byte;

  // read four bytes
  Wire.requestFrom(_i2caddr, 4);
  int i = 0;
  while (Wire.available())
  {
    received_byte = Wire.read();

    switch (i) {
      case 0:
        startdata = received_byte;
        break;

      case 1:
        battery_raw = received_byte;
        break;

      case 2:
        battery_raw = battery_raw << 8;
        battery_raw = battery_raw + received_byte;
        break;

      case 3:
        enddata = received_byte;
        break;
    }
    i++;

  } //while Wire.available

  if (startdata != 200 or enddata != 251)
  {
    battery_raw = 0;
  }
}


