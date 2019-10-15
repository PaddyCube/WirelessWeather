## Description
Build your own Wireless Weather station. This station includes:
- Temperature
- Humidity
- Pressure
- Rain Level
- Wind Speed
- Wind direction
- Battery monitor

There are two versions available. One uses ESP8622 module and some ATTINYs, one uses ESP32 and only one ATTINY for wind direction. Code and schematics for ESP32 can be found in corresponding folders named as "ESP32".

## Operation mode
# ESP8266
Wind speed and rain gets detected by pin change interrupt of ATTINYs. 
There is a power hungry device called WEMOS D1 mini pro, which deep sleeps most time. 
Every few minutes, it wakes up, collect data from other devices by I2C, send everything to your MQTT broker and sleeps again.

# ESP32
Wind speed and rain gets count by ULP co-processor of ESP32 module. Every few minutes, ESP32 wakes from deep sleep, read count values from ULP, power up I2C devices and sends everything to your MQTT broker. Then it sleeps again.

## Project site
You find all 3D printed parts at Thingiverse 
https://www.thingiverse.com/thing:3718078

A video is here https://youtu.be/xa0Dt5vs0kM

## BOM
This package includes software for the following components
# ESP8266
- 2x ATTINY85 (wind speed, rain and battery level)
- 1x ATTINY24 (wind direction)
- Wemos D1 Mini pro (or other ESP8266 device)
- BME280
- Please find a complete BOM in BOM.txt file

Moreover you need some device to program ATTINYs. This can be done with any Arduino device. There are plenty of tutorials out there.

# ESP32
- 1x ATTINY24 (wind direction, see folder Wireless Weather)
- ESP32 code (DOIT DevKit V1 with 30 GPIOs)

For further information, please see readme file in ESP32 folder

## Required Software
For ATTINYs, you need this package
https://github.com/SpenceKonde/ATTinyCore
https://github.com/rambo/TinyWire

For Wemos, I used this one
https://arduino.esp8266.com/stable/package_esp8266com_index.json

## License
<br>Copyright (c) 2019 by Patrick Weber  

Private-use only! (you need to ask for a commercial-use)
 

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Private-use only! (you need to ask for a commercial-use)

