// Sleep settings (max sleep in min, only occurs if no wind or rain)
// Wind an rain trigger interrupt which gets counted as wakeup. So when high wind or lot of rain
// occur, update interval is less than 15 min)
const int SLEEP_MIN = 15;

// Pins for wind and rain count
const int WindPin = 2;
const int RainPin = 3;

// pin for data send
const int SendPin = 12;
const int SendPower = 11;

// power for I2C devices
const int i2cPower = 5;
