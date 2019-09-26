
// WIFI -settings
const char* SSID = "YOUR_SSID";
const char* PSK = "YOUR_PASSWORD";

// MQTT Settings
const char* MQTT_BROKER = "IP of MQTT Broker";
const int MQTT_PORT = 1880;

// Sleep settings for ESP32
const int SLEEP_MIN = 1;  

// Sleep settings for ULP co-processor in milli seconds
const int ULP_SLEEP_MS = 20;

// GPIO number for anemometer, please use GPIO, not RTC_GPIO number
const gpio_num_t GPIO_WIND = GPIO_NUM_0;

// GPIO number for tipping bucket (rain sensor), please use GPIO, not RTC_GPIO number
const gpio_num_t GPIO_RAIN = GPIO_NUM_2;
