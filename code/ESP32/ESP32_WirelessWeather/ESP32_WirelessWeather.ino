/* Solar powered WIFI weather station using ESP32.

   Wind speed and rain gets detected by ESP32's built-in ULP co-processor while main ESP32 sleeps.
   Every few minutes, ESP32 wakes up, collect data from other devices by I2C,
   send everything to your MQTT broker and sleeps again.

   Please take note of settings.h
   Here you find wfi related settings, connection settings to MQTT, sleep settings and others

   Because I didn't want to learn a new IDE, I used Ardiono IDE to program everything (also ULP part), instead of esp-idf.
   To upload ULP code with Arduino IDE, you need ulptool provided by duff2013 (https://github.com/duff2013/ulptool),
   Arduino IDE itself and official packages for Arduino IDE provided by Espressif

   Copyright (c) 2019 by Patrick Weber

   Private-use only! (you need to ask for a commercial-use)

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Private-use only! (you need to ask for a commercial-use)
*/


#include <stdio.h>
#include "esp_sleep.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"
#include "soc/rtc_periph.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "settings.h"
#include "driver/adc.h"
#include <Adafruit_BME280.h>
#include "WindVane.h"
#include <WiFi.h>
#include <PubSubClient.h>
//#include <Wire.h>

#include "esp32/ulp.h"// Must have this!!!

// include ulp header you will create
#include "ulp_main.h"// Must have this!!!

// Custom binary loader
#include "ulptool.h"// Must have this!!!

// Unlike the esp-idf always use these binary blob names
extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");

WiFiClient espClient;
PubSubClient client(espClient);
WindVane windvane;
Adafruit_BME280 bme;

long lastMsg = 0;
char msg[50];
int value = 0;
char temp[10];
char pressure[10];
char humidity[10];
char windvalue[10];
char rainvalue[10];
char windpos[4];
char batvalue[10];
char solarvalue[10];

static void init_ulp_program(void);
static void update_pulse_count(void);
static void pin_setup(gpio_num_t gpio_num);
static void measureVoltage(void);
static void sleep();

/*--------------------------------------------------
   setup gets called on each boot and on each wakeup
  --------------------------------------------------*/
void setup(void)
{
  btStop();

  pinMode(i2cVccPin, OUTPUT);
  digitalWrite(i2cVccPin, HIGH);

  Serial.begin(115200);
  windvane.begin();
  Serial.println(bme.begin(0x76));


  setup_wifi();
  client.setServer(MQTT_BROKER, MQTT_PORT);

  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  if (cause != ESP_SLEEP_WAKEUP_TIMER) {
    printf("No TIMER wakeup, initializing ULP\n");
    init_ulp_program();
  } else {
    printf("TIMER wakeup, saving pulse count\n");
    client.setServer(MQTT_BROKER, MQTT_PORT);
    update_pulse_count();
  }
}

/*--------------------------------------------------
   loop
  --------------------------------------------------*/

void loop() {
  float lv_temp;
  float lv_pressure;
  float lv_humidity;
  unsigned int lv_wind;
  unsigned int lv_rain;
  unsigned int lv_battery;

  // prepare deep sleep
  ESP_ERROR_CHECK( esp_sleep_enable_timer_wakeup(SLEEP_MIN * 60 * 1000000) );

  // connecting to MQTT broker
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  snprintf (msg, 50, "Alive since %ld milliseconds", millis());
  Serial.print("Publish message: ");
  Serial.println(msg);

  // Read data from BME280
  lv_temp = bme.readTemperature();
  lv_pressure = bme.readPressure();
  lv_humidity = bme.readHumidity();
  delay(20);
  lv_temp = lv_temp +  bme.readTemperature();
  lv_pressure = lv_pressure + bme.readPressure();
  lv_humidity = lv_humidity + bme.readHumidity();
  delay(20);
  lv_temp = lv_temp + bme.readTemperature();
  lv_pressure = lv_pressure + bme.readPressure();
  lv_humidity = lv_humidity + bme.readHumidity();

  lv_temp = lv_temp / 3.0f;
  lv_pressure = lv_pressure / 3.0f;
  lv_humidity = lv_humidity / 3.0f;

  // convert results to char
  dtostrf(lv_temp, 6, 2, temp);
  dtostrf(lv_pressure, 6, 2, pressure);
  dtostrf(lv_humidity, 6, 2, humidity);


  delay(20);
  windvane.getCharPosition(windpos);

  // read battery and solar voltage
  measureVoltage();

  //send by MQTT

  client.publish("/WirelessWeather/Solar", solarvalue);
  client.publish("/WirelessWeather/Temperature", temp );
  client.publish("/WirelessWeather/Pressure", pressure );
  client.publish("/WirelessWeather/Humidity", humidity );
  client.publish("/WirelessWeather/Wind", windvalue );
  client.publish("/WirelessWeather/Rain", rainvalue );
  client.publish("/WirelessWeather/WindDir", windpos );
  client.publish("/WirelessWeather/Battery", batvalue);

  printf("Entering deep sleep\n\n");
  sleep();
}

/*--------------------------------------------------
   This function loads ULP programm to ULP-co processor
  --------------------------------------------------*/
static void init_ulp_program(void)
{
  esp_err_t err = ulp_load_binary(0, ulp_main_bin_start,
                                  (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));
  ESP_ERROR_CHECK(err);

  /* GPIO used for pulse counting. */
  assert(rtc_gpio_desc[GPIO_WIND].reg && "GPIO used for pulse counting must be an RTC IO");
  assert(rtc_gpio_desc[GPIO_RAIN].reg && "GPIO used for pulse counting must be an RTC IO");

  /* Initialize some variables used by ULP program.
     Each 'ulp_xyz' variable corresponds to 'xyz' variable in the ULP program.
     These variables are declared in an auto generated header file,
     'ulp_main.h', name of this file is defined in component.mk as ULP_APP_NAME.
     These variables are located in RTC_SLOW_MEM and can be accessed both by the
     ULP and the main CPUs.

     Note that the ULP reads only the lower 16 bits of these variables.
  */
  ulp_debounce_counter_wind = 2;
  ulp_debounce_counter_rain = 2;
  ulp_debounce_max_count = 3;
  ulp_next_edge_wind = 0;
  ulp_next_edge_rain = 0;
  ulp_io_number_wind = rtc_gpio_desc[GPIO_WIND].rtc_num; /* map from GPIO# to RTC_IO# */
  ulp_io_number_rain = rtc_gpio_desc[GPIO_RAIN].rtc_num; /* map from GPIO# to RTC_IO# */

  /* Initialize selected GPIO as RTC IO, enable input, disable pullup and pulldown */
  pin_setup(GPIO_WIND);
  pin_setup(GPIO_RAIN);

  /* Disconnect GPIO12 and GPIO15 to remove current drain through
     pullup/pulldown resistors.
     GPIO12 may be pulled high to select flash voltage.
  */
  rtc_gpio_isolate(GPIO_NUM_12);
  rtc_gpio_isolate(GPIO_NUM_15);
  esp_deep_sleep_disable_rom_logging(); // suppress boot messages

  /* Set ULP wake up period to T = 20ms.
     Minimum pulse width has to be T * (ulp_debounce_counter + 1) = 80ms.
  */
  ulp_set_wakeup_period(0, ULP_SLEEP_MS * 1000);

  /* Start the program */
  err = ulp_run(&ulp_entry - RTC_SLOW_MEM);
  ESP_ERROR_CHECK(err);
}

/*--------------------------------------------------
   This function shows last pulse counts
  --------------------------------------------------*/
static void update_pulse_count(void)
{


  /* ULP program counts signal edges, convert that to the number of pulses */
  uint32_t pulse_count_from_ulp_wind = (ulp_edge_count_wind & UINT16_MAX) / 2;
  /* In case of an odd number of edges, keep one until next time */
  ulp_edge_count_wind = ulp_edge_count_wind % 2;
  printf("Pulse count from wind sensor: %5d\n", pulse_count_from_ulp_wind);

  /* ULP program counts signal edges, convert that to the number of pulses */
  uint32_t pulse_count_from_ulp_rain = (ulp_edge_count_rain & UINT16_MAX) / 2;
  /* In case of an odd number of edges, keep one until next time */
  ulp_edge_count_rain = ulp_edge_count_rain % 2;
  printf("Pulse count from rain sensor: %5d\n", pulse_count_from_ulp_rain);

  itoa(pulse_count_from_ulp_wind, windvalue, 10);
  itoa(pulse_count_from_ulp_rain, rainvalue, 10);
}

/*--------------------------------------------------
   This function prepares RTC_GPIO for ULP program
  --------------------------------------------------*/
static void pin_setup(gpio_num_t gpio_num)
{
  rtc_gpio_init(gpio_num);
  rtc_gpio_set_direction(gpio_num, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pulldown_dis(gpio_num);
  rtc_gpio_pullup_dis(gpio_num);
  rtc_gpio_hold_en(gpio_num);
}


/*--------------------------------------------------
   This function connects ESP32 to WIFI
  --------------------------------------------------*/
void setup_wifi() {
  WiFi.setHostname("ESP32WirelessWeather");
  delay(10);
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.print(SSID);

  WiFi.begin(SSID, PSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
/*--------------------------------------------------
   This function reconnects to MQTT broker
  --------------------------------------------------*/
void reconnect() {

  int mqtt_timeout = 0;
  while (!client.connected() && mqtt_timeout < 5) {
    Serial.println("Reconnecting to MQTT broker...");
    if (!client.connect("ESP32Client")) {
      Serial.println("failed, rc=");
      Serial.println(client.state());
      mqtt_timeout++;

      if (mqtt_timeout >= 5)
      {
        Serial.print("Error connecting to MQTT broker, sleep ");
        delay(100);
        sleep();
      } else {
        Serial.println(" retrying in 5 seconds");
        delay(5000);
      }
    }
  }
}

/*--------------------------------------------------
   This function measures voltage
  --------------------------------------------------*/
static void measureVoltage() {

  // Voltage divider R1 = 220k+100k+220k =540k and R2=100k

  // battery
  int raw = analogRead(adc_battery);
  // 1MOhm / 560kOhm * ref_voltage / 12bit ADC
  float volt = (1000.0f / 560.0f) * 3.30f * float(raw) / 4096.0f * calib_factor_battery;
  dtostrf(volt, 6, 2, batvalue);
  Serial.print( "Voltage Battery = ");
  Serial.print(batvalue);
  Serial.println (" V");

  // solar
  raw = analogRead(adc_solar);
  // 1MOhm / 560kOhm * ref_voltage / 12bit ADC
  volt = (1000.0f / 560.0f) * 3.30f * float(raw) / 4096.0f * calib_factor_solar;
  dtostrf(volt, 6, 2, solarvalue);
  Serial.print( "Voltage Solar = ");
  Serial.print(solarvalue);
  Serial.println (" V");
}

/*--------------------------------------------------
   This function prepares everything for deep sleep
  --------------------------------------------------*/
static void sleep()
{
  digitalWrite(i2cVccPin, LOW);
  delay(1000); // Time to complete all WiFi transmissions
  adc_power_off();
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  delay(100);

  esp_deep_sleep_start();
}
