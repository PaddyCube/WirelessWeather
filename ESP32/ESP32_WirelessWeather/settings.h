
// WIFI -settings
const char* SSID = "YOUR_SSID";
const char* PSK = "YOUR_PSK";

// MQTT Settings
const char* MQTT_BROKER = "IP_ADDRESS OF MQTT BROKER";
const int MQTT_PORT = 1883;

// Sleep settings for ESP32
const int SLEEP_MIN = 5;

// Sleep settings for ULP co-processor in milli seconds
const int ULP_SLEEP_MS = 10;

// GPIO number for anemometer, please use GPIO, not RTC_GPIO number
const gpio_num_t GPIO_WIND = GPIO_NUM_35;

// GPIO number for tipping bucket (rain sensor), please use GPIO, not RTC_GPIO number
const gpio_num_t GPIO_RAIN = GPIO_NUM_26;

// ADC GPIO number for battery
const int adc_battery = A0;

// ADC GPIO number for solar
const int adc_solar = A3;

// change this value to calibrate the battery voltage
float calib_factor_battery = 1.863; 

// change this value to calibrate the solar voltage
float calib_factor_solar = 1.863;

// Define Pin for I2C VCC
const gpio_num_t i2cVccPin = GPIO_NUM_23;
