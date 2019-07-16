/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
#define WINDSPEED_ADDRESS                (0x4)
/*=========================================================================*/

/*--- Class Definition  */
class WindSpeed {

  public:
    WindSpeed(void);
    bool  begin(uint8_t addr = WINDSPEED_ADDRESS);
    unsigned long getSpeed(boolean buff_read = false);
    unsigned long getRain(boolean buff_read = false);
    unsigned int getByte();

  private:
    void i2c_read();
    void resetData();
    uint8_t   _i2caddr;
    unsigned int windcount1 = 0;
    unsigned int windcount2 = 0;
    unsigned int windcount3 = 0;
    unsigned int raincount1 = 0;
    unsigned int raincount2 = 0;
    unsigned int raincount3 = 0;
};

/*------------ Class Implementation ------*/

//----------------------------
// Class Constructor
//----------------------------
WindSpeed::WindSpeed()
{ }

//----------------------------------
// Set I2C addres of Wind speed sensor
//----------------------------------
bool WindSpeed::begin(uint8_t a)
{
  _i2caddr = a;

  return true;
}

//----------------------------------
// Get wind speed as integer
//----------------------------------
unsigned long WindSpeed::getSpeed(boolean buff_read)
{
  unsigned long speed = 0;

  if (buff_read == false)
  {
    i2c_read();
  }

  speed = ( windcount3 * 255 * 255) + (windcount2 * 255) + windcount1;
  return speed;

}

//----------------------------------
// Get rain count as integer
//----------------------------------
unsigned long WindSpeed::getRain(boolean buff_read)
{
  unsigned long rain = 0;

  if (buff_read == false)
  {
    i2c_read();
  }

  rain = ( raincount3 * 255 * 255) + (raincount2 * 255) + raincount1;
  return rain;

}
//----------------------------------
// Read single byte
//----------------------------------
unsigned int WindSpeed::getByte()
{
  Wire.requestFrom(_i2caddr, 1);
  while (Wire.available())
  {
    return Wire.read();
  }
}

//----------------------------------
// Read i2c Data
//----------------------------------
void WindSpeed::i2c_read()
{
  unsigned int data = 0;
  unsigned int startdata = 0;
  unsigned int enddata = 0;
  byte received_byte;
  int i = 0;

  Wire.requestFrom(_i2caddr, 8);
  while (Wire.available())
  {
    received_byte = Wire.read();

    switch (i) {
      case 0:
        startdata = received_byte;
        break;

      case 1:
        windcount1 = received_byte;
        break;

      case 2:
        windcount2 = received_byte;
        break;

      case 3:
        windcount3 = received_byte;
        break;

      case 4:
        raincount1 = received_byte;
        break;

      case 5:
        raincount2 = received_byte;
        break;

      case 6:
        raincount3 = received_byte;
        break;

      case 7:
        enddata = received_byte;
        break;
    }

    i++;
  }  // while Wire.available

  // check for valid start and stop signs
  if (startdata != 200 or enddata != 251)
  {
    windcount1 = 255;
    windcount2 = 255;
    windcount3 = 255;
    raincount1 = 255;
    raincount2 = 255;
    raincount3 = 255;
  }
  else // data was valid, reset sensor
  {
    resetData();
  }
}


void WindSpeed::resetData()
{

  uint8_t senddata;

  // Transmission codes:
  // 1 = Reset Rain Sensor
  // 2 = Reset Wind Sensor
  // 3 = Reboot Rain Sensor

  // clear data
  Wire.beginTransmission(_i2caddr);

  senddata = 1; // reset rain sensor
  Wire.write(senddata);

  senddata = 2; // reset wind sensor
  Wire.write(senddata);

  Wire.endTransmission();
  delay(10);
}

