/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
#define WINDVANE_ADDRESS                (0x5)
/*=========================================================================*/

/*--- Class Definition  */
class WindVane {

  public:
    WindVane(void);
    bool  begin(uint8_t addr = WINDVANE_ADDRESS);
    int getIntPosition();
    void getCharPosition(char* charpos);
    unsigned int getByte();

  private:
    uint8_t   _i2caddr;
};

/*------------ Class Implementation ------*/

//----------------------------
// Class Constructor
//----------------------------
WindVane::WindVane()
{ }

//----------------------------------
// Set I2C addres of Wind vane
//----------------------------------
bool WindVane::begin(uint8_t a)
{
  _i2caddr = a;

  return true;
}

//----------------------------------
// Read raw byte
//----------------------------------
unsigned int WindVane::getByte()
{
  Wire.requestFrom(_i2caddr, 3);
  while (Wire.available())
  {
    Serial.println(Wire.read());
  }
  return 1;
}
//----------------------------------
// Get wind direction as integer
//----------------------------------
int WindVane::getIntPosition()
{

  unsigned int pos = 0;
  unsigned int startdata = 0;
  unsigned int enddata = 0;
  byte received_byte;
  int i = 0;

  Wire.requestFrom(_i2caddr, 3);
  while (Wire.available())
  {
    received_byte = Wire.read();

    switch (i) {
      case 0:
        startdata = received_byte;
        break;

      case 1:
        pos = received_byte;
        break;

      case 2:
        enddata = received_byte;
        break;
    }

    i++;
  }  // while Wire.available

  // check for valid start and stop signs
  if (startdata != 200 or enddata != 251)
  {
    pos = 999;
  }

  return pos;

}

//----------------------------------
// Get wind direction as Char
//----------------------------------
void WindVane::getCharPosition(char * charpos)
{
  int pos;

  pos = getIntPosition();

  switch (pos) {
    case 1:
      strncpy(charpos, "N", 4);
      break;
    case 3:
      strncpy(charpos, "NNO", 4);
      break;
    case 2:
      strncpy(charpos, "NO", 4);
      break;
    case 6:
      strncpy(charpos,  "ONO", 4);
      break;
    case 4:
      strncpy(charpos, "O", 4);
      break;
    case 12:
      strncpy(charpos,  "OSO", 4);
      break;
    case 8:
      strncpy(charpos,  "SO", 4);
      break;
    case 24:
      strncpy(charpos, "SSO", 4);
      break;
    case 16:
      strncpy(charpos,  "S", 4);
      break;
    case 48:
      strncpy(charpos, "SSW", 4);
      break;
    case 32:
      strncpy(charpos, "SW", 4);
      break;
    case 96:
      strncpy(charpos, "WSW", 4);
      break;
    case 64:
      strncpy(charpos,  "W", 4);
      break;
    case 192:
      strncpy(charpos,  "WNW", 4);
      break;
    case 128:
      strncpy(charpos,  "NW", 4);
      break;
    case 129:
      strncpy(charpos,  "NNW", 4);
      break;
    default:
      strncpy(charpos,  "ERR", 4);
      break;
  }


}

