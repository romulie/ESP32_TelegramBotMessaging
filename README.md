An ESP32-based device which shows messages and dates of the messages sent to your Telegram bot on the local LCD screen.
The ESP32 sends message back to the receiver.
    
    Parts:
    any ESP32 board
    Push-button attached to pin 12 (should have pull-up);
    I2C LCD 16*2 attached to pins 21(SDA) and 22(SCL) (I2C of ESP32), VCC and GND
    Piezo buzzer is attached to pin 19(+) and GND via 100 Ohm resistor
     
 Uses WiFi Manager to connect to any WiFi network using Name and Password on Access Point configuration page.
 
 Lets you send messages (up to 38 chars for LCD 16*2, English only) using your Telegram bot.
 This version uses WiFi manager in order to be able to connect to any WiFi network if the SSID and password is known:
 ESP32 creates AccessPoint with name "ESP32_Config" (param. ssid_AP, see below). When connected to this AP using
 password "12345678" (param. password_AP, see below) you can automatically visit a page 192.168.4.1 where you can submit credentials
 of the wifi network to connect to. After that the ESP32 connects to this wifi network.
 The IP-address and time of connection is then shown on the LCD-screen.
 IP address and MAC address of your WiFi Shield (once connected) are sent to the Serial monitor also.
 
 From there, you can open Telegram and send messages through the bot with token (got from Telegram Godfather) defined below.
 Incoming messages are then displayed on the LCD screen.
 Buzzer plays short melody once a minute if incoming message is not read.
 Using push-button you can reset the received message, disable melody and turn-off the backlight of the LCD screen.

