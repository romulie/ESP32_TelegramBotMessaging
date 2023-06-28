/*
    An ESP32-based device which shows messages and dates of the messages sent to your Telegram bot on the local LCD screen
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
 The IP-address is then shown on the LCD-screen.
 IP address and MAC address of your WiFi Shield (once connected) are sent to the Serial monitor also.
 
 From there, you can open Telegram and send messages through a bot with token defined below which are then displayed on the LCD screen.
 Buzzer plays short melody once a minute if incoming message is not read.
 Using push-button you can reset the received message, disable melody and turn-off the backlight of the LCD screen

    TelegramBot is Written by Brian Lough
    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
*/

#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <LiquidCrystal_I2C.h>
#include <ezBuzzer.h>
#include <ctime>
#include "src\Button\button.h"   
#include "src\Timer\timer.h"    
#include "src\Clock\clock.h"

#define BTN_PIN 12
#define BUZ_PIN 19
#define LCD_WIDTH 16
#define MSG_MAX_LEN 38                  // max length of the message to fit into lcd scrolling screen (appr.40 for LCD 2*16 ??)
#define BOT_TOKEN "XXXXXXXXXX:YYGm8XBuz_a0JwCdkG1pBJuEimVaylV5Bzc"  // Telegram BOT Token (Get it from Telegram Botfather)
const unsigned long BOT_MTBS = 5000;    // mean time between scan messages
const char* ssid_AP = "ESP32_Config";   // Configuration Access Point NAME
const char* password_AP = "12345678";   // Configuration Access Point PASSWORD ( >=8 chars !!!)

WiFiManager wm;                             // WiFiManager object for creating Access Point and getting WiWi Network Credentials (ssid and pswd) via browser at 192.168.4.1
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;                 // last time messages' scan has been done
Timer timer_1sec(1000);                     // is used to drive the clock
Timer timer_scroll(500);                    // is used to scroll and update the LCD screen
Clock myClock;                              // is used to measures time after the message is received
Button myButton(BTN_PIN);                   // push-button to confirm the receit of the message and turn-off the screen
LiquidCrystal_I2C lcd(0x27, LCD_WIDTH, 2);  // Check for proper I2C address (most common is 0x27)
ezBuzzer myBuzzer(BUZ_PIN);                 // Buzzer to generate alarm signal for the incoming message in the background mode

int melody[] = { NOTE_C5, NOTE_G4, NOTE_C4 }; // notes in the melody for incoming message
int noteDurations[] = { 4, 4, 4 };            // note durations: 4 = quarter note, 8 = eighth note, etc, also called tempo
int melodyLength = sizeof(noteDurations) / sizeof(int);

String msg = "";                              // here we hold the message received
String date = "";                             // here we hold the date of the message received

void show();                                  // Shows the info on LCD, see below
void blinkLCD(size_t, size_t);                // Flashes LCD screen number of times with period specified, see below
void handleNewMessages(int numNewMessages);   // Basically sands incoming message back
String unixTimestampToLocalTime(const String &date);  // converts unix timestamp to local time string

void setup(){
  pinMode(BTN_PIN, INPUT_PULLUP);             // actually not nessesary since it is configured in Button class

  WiFi.mode(WIFI_STA);                        // explicitly set WiFi-mode as stationary Access Point to configure 
  Serial.begin(115200);
  Serial.println();
  delay(10);
    
  //wm.resetSettings(); // ONLY for development PHASE to check full configuration procedure through AP 
    
  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name (ssid_AP),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result
  bool res = wm.autoConnect(ssid_AP, password_AP);          // connect to the Access Point using NAME and PASSWORD for configuration
  if(!res)  { Serial.println("Failed to connect"); }        // ESP.restart(); } 
  else      { Serial.println("connected!!! :)"); }          // if you get here you have connected to the WiFi
  
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);      // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  
  Serial.print("\nWiFi connected. IP address: "); Serial.println(WiFi.localIP());
  Serial.print("\nMAC address: "); Serial.println(WiFi.macAddress());

  // display local IP-address on the LCD-screen after connection to the WiFi in order to notify the user about local IP-address of the ESP32-device
  msg = WiFi.localIP().toString();

  Serial.print("\nRetrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600){
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  String s(now);
  date = unixTimestampToLocalTime(s);
  Serial.print("date: "); Serial.println(date);
}

void loop(){
  
  myBuzzer.loop(); // MUST call the buzzer.loop() function in loop()
  // Play melody once a minute if the incoming message is not read
  if (msg.length() && myClock.getSeconds() == 0) { myBuzzer.playMelody(melody, noteDurations, melodyLength); blinkLCD(5, 100); };
  // On button press: clear message, turn-off LCD backlight and reset the clock
  if (myButton.click())                                      { msg = ""; lcd.noBacklight(); date = ""; myClock.tick();}
  // Increment clock only if message is not empty
  if (msg.length() && timer_1sec.ready())                    { myClock.tick(); show(); }
  // scroll the screen if message does not fit into screen
  if (timer_scroll.ready() && (msg.length() > LCD_WIDTH - 2 || date.length() > LCD_WIDTH - 2))  { lcd.scrollDisplayLeft(); }
  
  if (millis() - bot_lasttime > BOT_MTBS){
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages){
      Serial.print("Got message: "); Serial.println(bot.messages[0].text);
      Serial.print("From: "); Serial.println(bot.messages[0].from_name);
      date = unixTimestampToLocalTime(bot.messages[0].date);
      Serial.print("Date: "); Serial.println(date);
      
      msg = bot.messages[0].from_name + ": " + bot.messages[0].text;
      if (msg.length() > MSG_MAX_LEN) msg = msg.substring(0, MSG_MAX_LEN); // cut the message if it is too long
      
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}
//========================================================
void show(){ 
  lcd.setCursor(0, 0); lcd.print("                ");
  lcd.setCursor(2, 0); lcd.print(date);
  //byte h = myClock.getHours(), m = myClock.getMinutes(), s = myClock.getSeconds();
  //lcd.setCursor(2, 0); //lcd.print("Time:  ");
  //if (h < 10) lcd.print(0);
  //lcd.print(h); lcd.print(":");
  //if (m < 10) lcd.print(0);
  //lcd.print(m);lcd.print(":");
  //if (s < 10) lcd.print(0);
  //lcd.print(s);
  lcd.setCursor(0, 1); lcd.print("                ");
  lcd.setCursor(2, 1); lcd.print(msg);
}
//=======================================
void blinkLCD(size_t cnt = 3, size_t tm = 100){ // blinks LCD-backlight cnt-times, with tm half-period.
  for (size_t i = 0; i < cnt; ++i){
    lcd.noBacklight();
    delay(tm);
    lcd.backlight();
    delay(tm);
  }
  lcd.init(); lcd.setCursor(2, 0);
}
//=======================================
void handleNewMessages(int numNewMessages){
  for (int i = 0; i < numNewMessages; i++){
    bot.sendMessage(bot.messages[i].chat_id, bot.messages[i].text, "");
  }
}
//=======================================
String unixTimestampToLocalTime(const String &date){ // converts unix timestamp to local time string
  long int date_secs = strtoul(date.c_str(), NULL, 10);
  String out(std::asctime(std::localtime(&date_secs)));
  out = out.substring(0, out.length()-1);// and cut off terminating '\n'
  
  return out;                 
}
