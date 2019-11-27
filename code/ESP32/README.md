## Description
This folder contains alternative code for ESP32 boards. By using this, less components are needed.

## Operation mode
Wind speed and rain gets detected by ESP32's built-in ULP co-processor while main ESP32 sleeps. 
Every few minutes, ESP32 wakes up, collect data from other devices by I2C, 
send everything to your MQTT broker and sleeps again.

## Project site
You find all 3D printed parts at Thingiverse 
https://www.thingiverse.com/thing:3718078


## BOM
This package includes software for the following components
- ESP32 board (I use ESP32 DEVKIT V1 with 30 GPIOs)

Because I didn't want to learn a new IDE, I used Arduino IDE to program everything (also ULP part), instead of esp-idf.
To upload ULP code with Arduino IDE, you need ulptool provided by duff2013 (https://github.com/duff2013/ulptool),
Arduino IDE itself and official packages for Arduino IDE provided by Espressif


## License
<br>Copyright (c) 2019 by Patrick Weber  

Private-use only! (you need to ask for a commercial-use)
 

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Private-use only! (you need to ask for a commercial-use)

