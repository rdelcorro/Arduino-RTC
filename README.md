# Arduino clock   

This project its a clock based on the esp8266 module. It connects to a wifi network, 
syncs with an NTP server and displays the time with millisecond accuracy on a 8 digit 7 segments display

## Parts

- ESP-12E NodeMcu
- MAX7219 based display
- Jump wires

## Usage:

- Download the code and configure your wifi network SSID and password.
- Download the https://github.com/rdelcorro/Arduino-RtNTPClient library.
- Download the https://github.com/rdelcorro/Arduino-LedControl library.
- Burn with Arduino IDE
