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
    String getCharPosition();

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
// Get wind direction as integer
//----------------------------------
int WindVane::getIntPosition()
{
  int pos = 999;
  int data = 999;

  // Startzeichen lesen
  Wire.requestFrom(0x5, 1);
  while (Wire.available())
  {
    data = Wire.read();
  }

  delay(50);

  // Startzeichen war gültig, Nutzdaten lesen
  if (data == 200)
  {

    Wire.requestFrom(0x5, 1);
    while (Wire.available())
    {
      pos = Wire.read();
    }
  }

  delay(50);

  // Stopzeichen lesen
  Wire.requestFrom(0x5, 1);
  while (Wire.available())
  {
    data = Wire.read();
  }

  if (data == 251)
  {
    return pos;
  }

  return 999;

}

//----------------------------------
// Get wind direction as Char
//----------------------------------
String WindVane::getCharPosition()
{
  int pos;

  pos = getIntPosition();

  switch (pos) {
    case 1:
      return "N";
      break;
    case 3:
      return "NNO";
      break;
    case 2:
      return "NO";
      break;
    case 6:
      return "ONO";
      break;
    case 4:
      return "O";
      break;
    case 12:
      return "OSO";
      break;
    case 8:
      return "SO";
      break;
    case 24:
      return "SSO";
      break;
    case 16:
      return "S";
      break;
    case 48:
      return "SSW";
      break;
    case 32:
      return "SW";
      break;
    case 96:
      return "WSW";
      break;
    case 64:
      return "W";
      break;
    case 192:
      return "WNW";
      break;
    case 128:
      return "NW";
      break;
    case 129:
      return "NNW";
      break;
    default:
      return "ERR";
      break;
  }

}

