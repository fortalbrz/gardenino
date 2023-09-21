#include <EEPROM.h>
#include <RTClib.h>
//---------------------------------------------------------------------------------------------------------------------
//
//   GARDENINO - Garden Wifi with Home Assistant and a single Arduino Nano
//   - Optimized for Arduino Nano R3 (ATmega328P) 
//
//   I made this project to make use of an old 8 chanels relay board with a single Arduino Nano 
//   and a really low cost ESP-02 module... :) The goal is to control irrigation, lights, etc
//   at garden using home assistant with MQTT protocol (e.g. mosquitto broker) or autonomus 
//   with some "standalone" inteligence on Arduino Nano.
//
//   - Features:
//      - works with home assistant to control garden watering and more 7 switches (MQTT), lights, etc
//      - alternativelly, works autonomus with up to 200 programable timers (using real time clock - optional, use config flags)
//      - soil moisture sensor to (avoids watering when the soil is wet - optional, use config flags)
//      - watering watchdog (prevents watering for long time... - optional, use config flags)   
//      - turn on/off watering with local push button (optional, use config flags)
//
//   However, Arduino Nano is not easy integrated with Wifi (I could have used a NodeMCU instead, 
//   but I already got this Nano and plain ESP-01 module... And after all, what's the fun? kkk). 
//   To solve this issue, I made a sketch apart to be flashed on the ESP8266-01 module (esp8266-01.ino)
//   (using board "Generic ESP8266 Module"), using the USB serial adapter CH340g. The ESP-01 should 
//   communicate with Home Assistant using MQTT and act as bridge (like "man-in-the-middle") to Arduino 
//   Nano using plain serial communication. 
//
//    ------------------         ------------------                ----------                   ---------------- 
//   |  home assistant  |  <=>  | mosquitto broker |  <=[MQTT]=>  |  ESP-01  |   <=[serial]=>  |  Arduino Nano  |
//    ------------------         ------------------                ----------                   ---------------- 
//                                   MQTT broker         wifi        bridge       LLC 3.3v-5v 
//
//   Therefore, to use serial communication on this sketch to debug with "Serial Monitor" set the macro
//   "DEBUG_MODE true", otherwise the serial communication is intended to ESP-01 module itself. 
//
//   NOTICE: that this sketch should be pushed into an Arduino board (Board "Arduino Nano") using the 
//   ATmega328P (Processor: "ATmega328P (Old Bootloader)").
//
//   Source code:
//   - https://github.com/fortalbrz/gardenino
//
//   Drivers (CH340g) for both Arduino Nano and ESP-01:
//    - CH340g USB/Serial driver (windows 11 compatible driver): https://bit.ly/44WdzVF 
//    - driver install instructions (pt-BR): https://bit.ly/3ZqIqc0
//    - flashing ESP-01 tutorial (pt-BR): https://bit.ly/3LRlZqT 
//
//   Materials:
//   - Arduino Nano R3 (ATmega328P)
//   - Wifi Module ESP8266-01 (ESP-01)
//   - ESP8266 USB serial adapter CH340g (https://produto.mercadolivre.com.br/MLB-2052186432-esp-01-wifi-esp8266-adaptador-usb-serial-ch340g-arduino-_JM)
//   - Logic Level Converter (LLC) 5v-3.3v (Bi-Directional)
//   - Real Time Clock (RTC) DS3231 (optional: standalone timers support)
//   - solenoid valve 3/4" 110 v (normaly closed)
//   - relay module 5v 8-ch (optional: less that 8 channels can be used)
//   - soil moisture sensor (optional: avoids watering when the soil is wet)
//   - push button and 10 k olhms resistor (optional: turn on/off watering)
//   - power supply 5vdc (1A)
//
//   Circuit Wiring Instruction (step by step):
//   -  https://www.circuito.io/app?components=514,11022,13322,13678,821989
//      - Arduino Nano pin29 (GND) --> power supply 5vdc (negative/Gnd)
//      - Arduino Nano pin27 (Vin) --> power supply 5vdc (positive/Vcc)
//      - Arduino Nano pin27 (Vin) --> power supply 5vdc (positive/Vcc)
//      - RTC DS3231 pin1 (GND) --> Arduino Nano pin29 (GND)
//      - RTC DS3231 pin4 (SCL) --> Arduino Nano pin (A5)
//      - RTC DS3231 pin3 (SDA) --> Arduino Nano pin (A4)
//      - RTC DS3231 pin2 (VCC) --> Arduino Nano pin17 (3.3v)
//      - ESP8266-01 pin8 (RXD) --> LLC pin1 (LV1)
//      - ESP8266-01 pin1 (TXD) --> LLC pin2 (LV2)
//      - ESP8266-01 pin7 (VCC) --> Arduino Nano pin17 (3.3v)
//      - ESP8266-01 pin2 (GND) --> Arduino Nano pin29 (GND)
//      - ESP8266-01 pin3 (CH_PD) --> Arduino Nano pin17 (3.3v)
//      - LLC pin3 (LV) --> Arduino Nano pin17 (3.3v)
//      - LLC pin3 (GND) --> Arduino Nano pin29 (GND)
//      - LLC pin11 (HV2) --> Arduino Nano pin13 (D10)
//      - LLC pin12 (HV1) --> Arduino Nano pin14 (D11)
//      - LLC pin12 (HV) --> Arduino Nano pin27 (5v)
//      - Relay 8 ch (VCC) --> Arduino Nano pin27 (5v)
//      - Relay 8 ch (GND) --> Arduino Nano pin29 (GND)
//      - Relay 8 ch (D0) --> Arduino Nano pin7 (D4)
//      - Relay 8 ch (D1) --> Arduino Nano pin8 (D5)
//      - Relay 8 ch (D2) --> Arduino Nano pin9 (D6)
//      - Relay 8 ch (D3) --> Arduino Nano pin10 (D7)
//      - Relay 8 ch (D4) --> Arduino Nano pin11 (D8)
//      - Relay 8 ch (D5) --> Arduino Nano pin12 (D9)
//      - Relay 8 ch (D6) --> Arduino Nano pin15 (D12)
//      - Relay 8 ch (D7) --> Arduino Nano pin16 (D13)
//      - Soil moisture sensor VIN (right) --> Arduino Nano pin6 (D3)
//      - Soil moisture sensor GND (center) --> Arduino Nano pin29 (GND)
//      - Soil moisture sensor SIG/A0 (left) --> Arduino Nano pin19 (A0)
//      - push button (term 1) --> Arduino Nano pin16 (D2 - interruption) 
//      - push button (term 1) --> Arduino Nano pin27 (5v)
//      - push button (term 2) --> 10k olhms resistor (term 1)
//      - 10k olhms resistor (term 2)  --> Arduino Nano pin29 (GND)
//
//   (connected pins: D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, A0, A4, A5)
//
//   MQTT topics (handled by ESP-01):
//    - gardenino/available: sensors availability ["online"/"offline"]
//    - gardenino/cmd: pushes commands to Arduino Nano [home assistant -> gardenino]:
//         "water on/off": turn on/off watering valve (i.e. relay 1)
//         "relay 4 on/of": turn on/of relay by number [1-8]
//         "reset": reset internal states
//         "relays on/off": turn on/off all relays
//         "enable/disable timers": enables/disables relay timers routine (avoid execution with out delete/update)
//         "enable/disable timer: 04": disable timer by "id" (see states for gets "timer id")
//         "add timer: relay hh mm hh mm": adds new relay timer (relay number, turn on hour, turn on minute, turn off hour, turn off minute)
//         "update timer: id hh mm hh mm": updates relay timer by "id" (id, turn on hour, turn on minute, turn off hour, turn off minute)
//         "delete timer: 04": delete timer by "id"
//         "delete all timers": deletes all relay timers (warning: all timers will be erased)
//         "default timers": sets default relay timers (i.e. timers "factory reset", warning: all timers will be erased)
//    - gardenino/state: retrieves states as json [gardenino -> home assistant]
//         {
//            "moisture": 50,
//            "timers_on": "on",
//            "relays": {
//                 "water": "on",
//                 "relay 2": "off",       
//                  ... 
//            },
//            "timers": {
//                 "timer_01": {
//                      "id": 4,
//                      "enabled": "on", 
//                      "relay": 1,
//                      "on": "12:20:00", 
//                      "off": "15:30:00"}
//                  ...           
//            },   
//         } 
//    - ha/datetime: [optional] creates a automation on home assistant in order to syncronize data & time
//               alias: MQTT date time sync
//               description: "syncronize data & time on client MQTT devices"
//               trigger:
//                 - platform: time_pattern
//                   minutes: /1
//               condition: []
//               action:
//                 - service: mqtt.publish
//                   data:
//                     qos: "0"
//                     retain: false
//                     topic: ha/datetime
//                     payload_template: "{{ now().timestamp() | timestamp_custom('%d %m %Y %H %M %S') }}"
//               mode: single
//
//
//   Esp-Arduino "internal serial protocol": 
//    - protocol commands [ESP-01 -> Arduino Nano]
//      0x00 - nop (NOP)
//      0x01 - inform that mqtt is connected (CONNECTED)
//      0x02 - inform that mqtt is disconnected (DISCONNECTED)
//      0x03 - datetime syncronization (uses home assistant as "time server" - SYNC) 
//      0x04 - clear states (CLEAR / mqtt: "clear")
//      0x05 - turn on all relays (ALL_RELAYS_ON / mqtt: "relays on")
//      0x06 - turn off all relays (ALL_RELAYS_OFF / mqtt: "relays off")
//      0xA0 - turn on relay 1 (i.e., watering relay)
//      0xA1 - turn on relay 2
//      0xA2 - turn on relay 3
//      0xA3 - turn on relay 4
//      ...
//      0xB0 - turn off relay 1 (i.e., watering relay)
//      0xB1 - turn off relay 2
//      0xB2 - turn off relay 3
//      0xB3 - turn off relay 4
//      ...
//      0xC0 - enables timers routine (ENABLE_TIMERS / mqtt: "enable timers")
//      0xC1 - enables timers routine (DISABLE_TIMERS / mqtt: "disable timers")
//      0xC2 - add new timer (ADD_TIMER / mqtt: "add timer: [hh] [mm] [hh] [mm]")
//      0xC3 - enables timer by id (ENABLE_TIMER / mqtt: "enable timer: [id]")
//      0xC4 - disable timer by id (DISABLE_TIMER / mqtt: "disable timer: [id]")
//      0xC5 - delete timer by id (DELETE_TIMER / mqtt: "delete timer: [id]") 
//      0xC6 - delete all timers (DELETE_ALL_TIMERS / mqtt: "delete timers")
//      0xC7 - update timer by if (UPDATE_TIMER / mqtt: "update timer: [id] [hh] [mm] [hh] [mm]")
//      0xC8 - set default timers, i.e. "factory reset" (SET_DEFAULT_TIMERS /  mqtt: "default timers")
//      0xFE - token for message start (START)
//      0xFF - token for message end (END)
//
//   - Arduino States [Arduino Nano -> ESP-01]
//      - 1st byte: START token (START - 0xFE)
//      - 2nd byte: gardeino states
//          bit 0 - timers enabled
//      - 3th byte: relay states:
//          bit 0 - relay 1 state
//          bit 1 - relay 2 state
//          bit 2 - relay 3 state
//          bit 3 - relay 4 state
//          ...
//      - 4th byte: soil moisture in range [0-100] %
//      - 5th byte: number of timers (EEPROM)
//      - 6th byte: END token (END - 0xFF)
//      - next bytes (optional, for each active timer)
//         - timer id (bit 7 = enable/disabled) [00-49]
//         - relay number [01-08]
//         - start hour in range [00-23]
//         - start minute in range [00-59]
//         - end hour in range [00-23]
//         - end minute in range [00-59]
//
// REMARK: debug routines (DEBUG_MODE true) commands:
//     - "mem"
//     - "reset"
//     - "sync"
//     - "relay on/off"
//     - "relays on/off"
//     - "enable/disable timers"
//     - "enable/disable timer""
//     - "add timer"
//     - "update timer"
//     - "delete timer"
//     - "delete all timers"
//     - "default timers"
//
//   Jorge Albuquerque (2022) - jorgealbuquerque@gmail.com
//
//---------------------------------------------------------------------------------------------------------------------
#define DEBUG_MODE false // true to debug on serial monitor (debug), false to communicate with ESP-01 (prod)
//
// pins definitions (Arduino Nano)
//
#define RELAY_PIN_01 7  // D4 - solenoid valve 110v
#define RELAY_PIN_02 8  // D5 - light
#define RELAY_PIN_03 9  // D6 - light
#define RELAY_PIN_04 10  // D7 - etc
#define RELAY_PIN_05 11  // D8 - etc
#define RELAY_PIN_06 12  // D9 - etc
#define RELAY_PIN_07 15 // D12 - etc
#define RELAY_PIN_08 16 // D13 - etc
#define WIFI_PIN_TX 14  // D11 - ESP-01 serial
#define WIFI_PIN_RX 13  // D10 - ESP-01 serial
#define MOISTURE_SENSOR_PIN_SIG 19 // A0 - moisture sensor analog input
#define MOISTURE_SENSOR_PIN_PWR 6 // D3 - moisture sensor power
#define WATERING_BUTTON_PIN 5  // D2 - turn on/off watering push button
//
// serial communcation to Arduino Nano/ESP-01
//
#if (DEBUG_MODE != true)
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial _serial(WIFI_PIN_RX, WIFI_PIN_TX); 
#endif
#endif

//
// configuration
//
#define RELAY_SIZE 8 // number of relays (valid: 1 to 8, defalut: 8)
#define USE_RELAY_TIMERS true // true to use timers, false otherwise - dont need the RTC (default: true)
#define MAX_NUMBER_OF_TIMERS 40 // maximum number of standalone timers (valid: 3 to 200, default: 40)
#define USE_MOISTURE_SENSOR true // true to use moisture sensor, false otherwise - dont need the moisture sensor (default: true)
#define SOIL_WET 450 // defines max soil conductivity value we consider soil 'wet' (valid: 0 to 1023, default: 450)
#define SOIL_DRY 750 // defines min soil conductivity value we consider soil 'dry' (valid: 0 to 1023, default: 750)
#define USE_WATERING_BUTTON true // true to use push button, false otherwise - dont need the push button (default: true)
#define USE_WATERING_WATCHDOG true // true to set a watering time limit, false otherwise (default: true)
#define WATERING_WATCHDOG_INTERVAL 300000 // watering time limit (milisseconds) (default: 300000 = 5 min) 
#define WATERING_RELAY 0 // watering valve relay index (default: 0)
#define BAUD_RATE 9600 // serial communication speed with ESP-01 (adjust both sketches)
//#define BAUD_RATE 31250 // serial communication speed (adjust both sketches)

//
// serial communication codes
//
#define NOP 0x00
#define CONNECTED 0x01
#define DISCONNECTED 0x02
#define SYNC 0x03
#define RESET 0x04
#define ALL_RELAYS_ON 0x05
#define ALL_RELAYS_OFF 0x06
#define ENABLE_TIMERS 0xC0
#define DISABLE_TIMERS 0xC1
#define ADD_TIMER 0xC2
#define ENABLE_TIMER 0xC3
#define DISABLE_TIMER 0xC4
#define DELETE_TIMER 0xC5
#define DELETE_ALL_TIMERS 0xC6
#define UPDATE_TIMER 0xC7
#define SET_DEFAULT_TIMERS 0xC8
#define START 0xFE
#define END 0xFF
// debug codes
#define STATES 0xD0
#define LIST_RELAYS 0xD1
#define LIST_TIMERS 0xD2
#define LOAD_TIMERS 0xD3
#define SAVE_TIMERS 0xD4
#define UPDATE 0xD5

#define EEPROM_ADDRESS 0

// real time clock (RTC)
RTC_DS3231 rtc;

// timer state
struct timer{
   bool enabled; 
   bool active; 
   byte relay;
   byte turnOnHour;
   byte turnOnMinute;
   byte turnOffHour;
   byte turnOffMinute;
};

#define is_valid_timer(t) ((t.turnOnHour != t.turnOffHour || t.turnOnMinute != t.turnOffMinute) && t.relay < RELAY_SIZE)

bool _isConnected = false;
bool _isReceivingMessage = false;
bool _isTimersEnabled = true;
bool _blink = true;
byte _soilMoistureLevel = 0;
unsigned long _WateringOn = 0;

#if (DEBUG_MODE == true) 
// debug only variables: emulates serial communcation with ESP-01
unsigned int _debugCacheIndex;
unsigned int _debugCacheLenght;
byte _debugCache[10];
#endif

// caches
byte RELAY_PINS[] = {RELAY_PIN_01, RELAY_PIN_02, RELAY_PIN_03, RELAY_PIN_04, 
  RELAY_PIN_05, RELAY_PIN_06, RELAY_PIN_07, RELAY_PIN_08};
bool RELAY_STATES[RELAY_SIZE];
timer RELAY_TIMERS[MAX_NUMBER_OF_TIMERS];


void setup() {
  //
  // intialization
  // 
  pinMode(LED_BUILTIN, OUTPUT);  

  #if (USE_MOISTURE_SENSOR == true)
  // soil moisture sensor off
  pinMode(MOISTURE_SENSOR_PIN_PWR, OUTPUT);
  digitalWrite(MOISTURE_SENSOR_PIN_PWR, LOW);
  #endif

  // initializes relays off (security)
  for(unsigned int i = 0; i < RELAY_SIZE; i++) {    
    pinMode(RELAY_PINS[i], OUTPUT);
    setRelay(i, false);
  }

  #if (USE_WATERING_BUTTON == true)
  pinMode(WATERING_BUTTON_PIN, INPUT);
  digitalWrite(WATERING_BUTTON_PIN, HIGH);
  attachInterrupt(digitalPinToInterrupt(WATERING_BUTTON_PIN), onWateringButton, FALLING);
  #endif
  
  if(!DEBUG_MODE && rtc.begin() && rtc.lostPower()) {
    // sets real time clock with sketch compiling time (if required)
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    delay(150);
  }

  _isConnected = false;    
  #if (DEBUG_MODE == true) 
  // initializes serial for Debug (serial monitor)
  // Serial.setTimeout(10000);
  Serial.begin(9600);  // default
  Serial.println(F("starting..."));    
  
  // load default timers (debug mode)
  _isTimersEnabled = true;    
  setDefaultTimers();
  saveTimers(false);
  #else  
  // load timers and internal states
  loadTimers(true);

  // initializes serial for ESP-01 module communication
  // _serial.setTimeout(800);
  _serial.begin(BAUD_RATE);
  #endif
  
  // waits
  delay(1500);
}


void loop() {
  //
  // main loop
  //
  
  // build-in led blinking
  digitalWrite(LED_BUILTIN, (_blink ? HIGH: LOW));
  _blink = !_blink;
  
  // handles serial commands from ESP-01 (MQTT bridge)
  #if (DEBUG_MODE == true) 
  dummySerialInput();     
  if (dummySerialAvailable()){
    if (onCommand(dummySerialRead())){
      // states updated!
      //Serial.println(F(" >> ok"));               
      updateStates();
    }
  }

  // runs timers (if required)
  runTimers();
  wateringWatchdog();
  
  #else
  
  if (_serial.available() > 0) {
    if (onCommand(_serial.read())){
      // states updated!
      updateStates();
    }
  }

  // runs timers (if required)
  runTimers();

  // prevents water valve from be opened long time...
  wateringWatchdog();

  // waits
  delay(1000);    
  #endif
}

bool onCommand(byte cmd) {
  //
  // Handles ESP-01 commands
  //
  switch(cmd){

    case START:
      _isReceivingMessage = true;
      #if (DEBUG_MODE == true) 
      Serial.println(F(" - token: START"));
      #endif
      return false;

    case END:
      _isReceivingMessage = false;
      #if (DEBUG_MODE == true) 
      Serial.println(F(" - token END"));
      #endif
      return false;
  }

  #if (DEBUG_MODE == true) 
  Serial.print(F(" - token cmd: 0x"));
  Serial.println(String(cmd, HEX));
  #endif

  if (!_isReceivingMessage){
    #if (DEBUG_MODE == true) 
    Serial.println(F("WARNING: incoming data without START token!"));
    #endif
    return false;
  }
    
  byte id = 0;
  byte relay = 0;
  byte hourOn = 0;
  byte minuteOn = 0;
  byte hourOff = 0;
  byte minuteOff = 0;
  unsigned int i = 0;
  // handles commands
  switch(cmd){

    case NOP:
      // nothing to do!
      #if (DEBUG_MODE == true) 
      Serial.println(F(" >> CMD: nop"));
      #endif
      return false;

    case CONNECTED:
      // MQTT connected
      // sets connected flag (ESP-01 has connected to MQTT broker)
      _isConnected = true;
      #if (DEBUG_MODE == true) 
      Serial.println(F(" >> CMD: connected"));
      #endif
      return false;

    case DISCONNECTED:
      // MQTT disconnected
      // clears connected flag (ESP-01 has disconnected from MQTT broker)
      _isConnected = false;
      #if (DEBUG_MODE == true) 
      Serial.println(F(" >> CMD: disconnected"));
      #endif
      return false;

    case ENABLE_TIMERS:
      // enables timers routines
      #if (DEBUG_MODE == true) 
      Serial.println(F(" >> CMD: timers enabled"));
      #endif
      _isTimersEnabled = true;
      // saves configuration on EEPROM
      saveConfig();
      return false;

    case DISABLE_TIMERS:
      // disables timers routines
      #if (DEBUG_MODE == true) 
      Serial.println(F(" >> CMD: timers disabled"));
      #endif
      _isTimersEnabled = false;
      // saves configuration on EEPROM
      saveConfig();
      return false;

    case SYNC:
      // syncronizes data and time with home assistant (if required)
      syncDateTime();
      return false;

    case 0XA0:
      // turn on water valve (relay 1 by default)!
      startWatering();
      return true;

    case 0XA1:
      // turn on relay 2
      setRelay(1, true);
      return true;

    case 0XA2:
      // turn on relay 3
      setRelay(2, true);
      return true;

    case 0XA3:
      // turn on relay 4
      setRelay(3, true);
      return true;

    case 0XA4:
      // turn on relay 5
      setRelay(4, true);
      return true;

    case 0XA5:
      // turn on relay 6
      setRelay(5, true);
      return true;

    case 0XA6:
      // turn on relay 7
      setRelay(6, true);
      return true;

    case 0XA7:
      // turn on relay 8
      setRelay(7, true);
      return true;

    case 0XB0:
      // turn off water valve (relay 1 by default)!
      setRelay(0, false);
      return true;

    case 0XB1:
      // turn off relay 2
      setRelay(0x01, false);
      return true;

    case 0XB2:
      // turn off relay 3
      setRelay(0x02, false);
      return true;

    case 0XB3:
      // turn off relay 4
      setRelay(0x03, false);
      return true;

    case 0XB4:
      // turn off relay 5
      setRelay(0x04, false);
      return true;

    case 0XB5:
      // turn off relay 6
      setRelay(0x05, false);
      return true;

    case 0XB6:
      // turn off relay 7
      setRelay(0x06, false);
      return true;

    case 0XB7:
      // turn off relay 8
      setRelay(0x07, false);
      return true;

    case ALL_RELAYS_ON:
      // turn on all relays
      for(i = 0; i < RELAY_SIZE; i++) 
        setRelay(i, true);
      return true;

    case ALL_RELAYS_OFF:
      // turn off all relays
      for(i = 0; i < RELAY_SIZE; i++) 
        setRelay(i, false);
      return true;

    case DELETE_ALL_TIMERS:
      // delete all timers
      cleanTimers();
      saveTimers(false);
      return false;

    case SET_DEFAULT_TIMERS:
      // set default timers (a.k.a., "factory reset")
      setDefaultTimers();
      _isTimersEnabled = true;
      saveTimers(true);
      return false;

    case ENABLE_TIMER:
      //
      // enables timer by "id" [0-49]
      //
      #if (DEBUG_MODE == true) 
      id = dummySerialRead();
      #else
      id = _serial.read();
      #endif
      enableTimer(id);
      return false;

    case DISABLE_TIMER:
      //
      // disables timer by "id" [0-49]
      //
      #if (DEBUG_MODE == true) 
      id = dummySerialRead();
      #else
      id = _serial.read();
      #endif
      disableTimer(id);
      return false;

    case DELETE_TIMER:
      //
      // deletes timer by "id" [0-49]
      //
      #if (DEBUG_MODE == true) 
      id = dummySerialRead();
      #else
      id = _serial.read();
      #endif
      deleteTimer(id);
      return false;

    case ADD_TIMER:
      //
      // creates new timer 
      //
      #if (DEBUG_MODE == true) 
      relay = dummySerialRead() - 1; // [1-8]
      hourOn = dummySerialRead(); // [0-23]
      minuteOn = dummySerialRead(); // [0-59]
      hourOff = dummySerialRead(); // [0-23]
      minuteOff = dummySerialRead(); // [0-59]
      #else
      relay = _serial.read() - 1; // [1-8]
      hourOn = _serial.read(); // [0-23]
      minuteOn = _serial.read(); // [0-59]
      hourOff =  _serial.read(); // [0-23]
      minuteOff = _serial.read(); // [0-59]
      #endif      
      addTimer(relay, hourOn, minuteOn, hourOff, minuteOff);
      return false;

    case UPDATE_TIMER:
      //
      // updates timer by "id"
      //
      #if (DEBUG_MODE == true) 
      id = dummySerialRead(); // timer id [0-49]
      hourOn = dummySerialRead(); // hh [0-23]
      minuteOn = dummySerialRead(); // mm [0-59]
      hourOff = dummySerialRead(); // hh [0-23]
      minuteOff = dummySerialRead(); // mm [0-59]
      #else 
      id = _serial.read(); // timer id [0-49]
      hourOn = _serial.read(); // hh [0-23]
      minuteOn = _serial.read(); // mm [0-59]
      hourOff =  _serial.read(); // hh [0-23]
      minuteOff = _serial.read(); // mm [0-59]
      #endif      
      updateTimer(id, hourOn, minuteOn, hourOff, minuteOff);
      return false;

    case STATES:
      //
      // list  internal states (DEBUG)
      //
      #if (DEBUG_MODE == true) 
      Serial.print(F("   - connected: "));
      Serial.println(_isConnected ? F("on") :  F("off"));
      Serial.print(F("   - timers enabled: "));
      Serial.println(_isTimersEnabled ? F("on") :  F("off"));
      Serial.print(F("   - timers: "));
      Serial.println(getTimersCount());
      #else
      id = 0X00;
      if (_isTimersEnabled)
        bitSet(id, 0); // bit 0 = timers enabled on/off
      if (_isConnected)
        bitSet(id, 1); // bit 1 = connected on/off
      _serial.write(START);
      _serial.write(STATES);
      _serial.write(id);
      _serial.write(END);
      _serial.flush();
      #endif
      return false;

    case LIST_RELAYS:
      // list relays states
      #if (DEBUG_MODE == true) 
      for(i = 0; i < RELAY_SIZE; i++) {
        Serial.print(F(" - relay: "));
        Serial.print(i);
        Serial.println(RELAY_STATES[i] ? F(": on") :  F(": off"));
      }
      #else
      id = 0X00;
      for(i = 0; i < RELAY_SIZE; i++) 
        if (RELAY_STATES[i])
          bitSet(id, i); // each bit = relay state on/off
      _serial.write(START);
      _serial.write(STATES);
      _serial.write(id);
      _serial.write(END);
      _serial.flush();
      #endif
      return false;

    case LIST_TIMERS:
      // list timers
      #if (DEBUG_MODE == true) 
      printTimers();
      #else
      unsigned int n = getTimersCount();
      _serial.write(START);
      _serial.write(LIST_TIMERS);
      _serial.write((byte)n);
      if (n > 0) 
        for (unsigned int i = 0; i < MAX_NUMBER_OF_TIMERS; i++) 
          if (RELAY_TIMERS[i].active && is_valid_timer(RELAY_TIMERS[i])) {
            byte id = (byte)i; // timer id
            if (RELAY_TIMERS[i].enabled)
              bitSet(id, 7); // bit 7 = enabled on/off
            _serial.write(id);
            _serial.write(RELAY_TIMERS[i].relay  + 1); // relay [1-8]
            _serial.write(RELAY_TIMERS[i].turnOnHour); // hh [0-23]
            _serial.write(RELAY_TIMERS[i].turnOnMinute); // mm [0-59]
            _serial.write(RELAY_TIMERS[i].turnOffHour); // hh [0-23]
            _serial.write(RELAY_TIMERS[i].turnOffMinute); // mm [0-59]
          }
      _serial.write(END);
      _serial.flush();
      #endif
      return false;

    case LOAD_TIMERS:
      // loads timers from EEPROM
      loadTimers(false);
      return false;

    case SAVE_TIMERS:
      // saves timers to EEPROM
      saveTimers(false);
      return false;

    case UPDATE:
      // gets a full states update
      return true;

    default:
      return false;  
  }
}


void runTimers() {
  //
  // runs timers
  //
  if (!USE_RELAY_TIMERS || !_isTimersEnabled)
    // nothing to do!
    return;

  #if (DEBUG_MODE == true) 
  Serial.println(F(" - checking timers - "));
  DateTime now = DateTime(2023, 9, 1, 6, 1, 9);  
  #else
  if (rtc.lostPower())
    // could not check timers without time!
    return;

  DateTime now = rtc.now();
  #endif
  
  for (unsigned int i = 0; i < MAX_NUMBER_OF_TIMERS; i++) {    
    // for each timer, check if its enabled (active and valid)
    if (RELAY_TIMERS[i].enabled 
      && RELAY_TIMERS[i].active 
      && is_valid_timer(RELAY_TIMERS[i])) {

      bool shouldBeOn = checkInRange(now, RELAY_TIMERS[i]);
      unsigned int relay_idx = RELAY_TIMERS[i].relay; 
      
      // #if (DEBUG_MODE == true) 
      // Serial.print("should be on: ");
      // Serial.print(shouldBeOn);
      // printTimer(RELAY_TIMERS[i], i);
      // #endif
      
      if (!RELAY_STATES[relay_idx] && shouldBeOn){
        // turns on relay
        if (relay_idx == WATERING_RELAY) 
          startWatering();        
        else 
          setRelay(relay_idx, true);
        updateStates();

      } else if (RELAY_STATES[relay_idx] && !shouldBeOn){
        // turns off relay
        setRelay(relay_idx, false);
        updateStates();
      }
    }
  }
}


void startWatering() {
    //
    // starts watering
    //
    #if (DEBUG_MODE == true) 
    Serial.print("start watering");
    #endif

    #if (USE_MOISTURE_SENSOR == true)
    if (readMoistureSensor() < SOIL_WET)
      // soil is already wet: nothing to do!
      return;
    #endif

    // open water valve
    setRelay(WATERING_RELAY, true);

    // set initial time to watering watchdog
    _WateringOn = millis();
}


void wateringWatchdog() {
  //
  // prevents water valve from be opened long time...
  //
  if (USE_WATERING_WATCHDOG 
    && RELAY_STATES[WATERING_RELAY] 
    && ((millis() - _WateringOn) > WATERING_WATCHDOG_INTERVAL)){
    setRelay(WATERING_RELAY, false);
    updateStates();
  }  
}

#if (USE_WATERING_BUTTON == true)
void onWateringButton(){
  //
  // watering button interruption handler
  //
  if (!USE_WATERING_BUTTON)
    return;

  if (RELAY_STATES[WATERING_RELAY]) 
    setRelay(WATERING_RELAY, false);
  else
    startWatering();
  
  updateStates();
}
#endif

void syncDateTime() {
  //
  // syncronizes date & time with home assistant ("time server")
  // (no delays at RTC!!!)
  //
  #if (DEBUG_MODE == true) 
  unsigned int day = (unsigned int)dummySerialRead();
  unsigned int month = (unsigned int)dummySerialRead();
  unsigned int year = 2000 + (unsigned int)dummySerialRead();
  unsigned int hour = (unsigned int)dummySerialRead();
  unsigned int minute = (unsigned int)dummySerialRead();
  unsigned int second = (unsigned int)dummySerialRead();
  #else
  unsigned int day = (unsigned int)_serial.read();
  unsigned int month = (unsigned int)_serial.read();
  unsigned int year = 2000 + (unsigned int)_serial.read();
  unsigned int hour = (unsigned int)_serial.read();
  unsigned int minute = (unsigned int)_serial.read();
  unsigned int second = (unsigned int)_serial.read();  
  #endif
  if (day == 0 || month == 0 || year == 2000)
    // something is wrong!
    return;

  #if (DEBUG_MODE == true) 
  Serial.print(F("sync: "));
  Serial.print(day);
  Serial.print(F("/"));
  Serial.print(month);
  Serial.print(F("/"));
  Serial.print(year);
  Serial.print(F(" "));
  Serial.print(hour);
  Serial.print(F(":"));
  Serial.print(minute);
  Serial.print(F(":"));
  Serial.println(second);
  #else
  DateTime now = rtc.now();  
  if (rtc.lostPower() || now.day() != day || now.month() != month || now.year() != year 
    || now.hour() != hour || now.minute() != minute) {      
    rtc.adjust(DateTime(year, month, day, hour, minute, second));
    delay(100);
  }
  #endif
}


void setRelay(unsigned int index, const boolean& state) {
  //
  // Set "true" to activates the relay with specified index ([0-7]), 
  // "false" to deactivates it.
  //
  #if (DEBUG_MODE == true) 
  Serial.print(F(" - relay "));
  Serial.print(index + 1);
  Serial.println(state ? F(": on") : F(": off"));    
  #endif
  RELAY_STATES[index] = state;
  digitalWrite(RELAY_PINS[index], (state ? HIGH : LOW));
}


void updateStates() {    
    //
    // pushes current states to ESP-01 using serial 
    // (bridge to publish states to MQTT/home assistant)
    //
    if (!_isConnected)
      // not connected: nothing to do!
      return;

    // gardeino states
    byte states = 0x00;
    if (_isTimersEnabled)
      bitSet(states, 0);

    // create message code (one byte with relay states)
    byte relay_states = 0x00;
    for(unsigned int i = 0; i < RELAY_SIZE; i++) 
      if (RELAY_STATES[i])
        bitSet(relay_states, i);

    unsigned int n_timers = getTimersCount();
    
    #if (DEBUG_MODE == true) 
    // sends data to serial monitor
    Serial.println(F("START"));   
    Serial.print(F(" - states: ")); 
    Serial.println(String(states, BIN));
    Serial.print(F(" - relays: ")); 
    Serial.println(String(relay_states, BIN));
    Serial.print(F(" - moisture: ")); 
    Serial.println(String(_soilMoistureLevel, DEC));
    Serial.print(F(" - timers: ")); 
    Serial.println(String(n_timers, DEC));
    Serial.println(F("END"));
    if (n_timers > 0) 
      printTimers(); 
    #else 
    // sends byte data to ESP-01 using serial
    #if (USE_MOISTURE_SENSOR == true)
    readMoistureSensor();      
    #endif
    _serial.write(START);
    _serial.write(states);
    _serial.write(relay_states);
    _serial.write(_soilMoistureLevel);
    _serial.write((byte)n_timers);
    _serial.write(END);
    if (n_timers > 0) 
      for (unsigned int i = 0; i < MAX_NUMBER_OF_TIMERS; i++) 
        if (RELAY_TIMERS[i].active && is_valid_timer(RELAY_TIMERS[i])) {
          byte id = (byte)i;
          if (RELAY_TIMERS[i].enabled)
            bitSet(id, 7);
          _serial.write(id);
          _serial.write(RELAY_TIMERS[i].relay  + 1);
          _serial.write(RELAY_TIMERS[i].turnOnHour);
          _serial.write(RELAY_TIMERS[i].turnOnMinute);
          _serial.write(RELAY_TIMERS[i].turnOffHour);
          _serial.write(RELAY_TIMERS[i].turnOffMinute);
        }      
    _serial.flush();
    #endif

    delay(100);
}


bool checkInRange(const DateTime& now, const timer& t){
  //
  // Returns True if the specified time is inside timer "turn on" window
  //
  
  // checks if specified is inside timer "hour" range
  int hour = now.hour(); 
  if (t.turnOnHour <= t.turnOffHour) {
    // "same day" timer, eg:
    //   - turn on:  07:00 am
    //   - current:  08:00 am (inside range)
    //   - turn off: 10:00 am
    if (hour < t.turnOnHour || hour > t.turnOffHour)
      // out of "turn on" hours ("same day" timer)
      return false;
  } else {
    // "next day" timer, eg:
    //   - turn on:  20:00 pm
    //   - current:  23:00 pm (inside range)
    //   - turn off: 05:00 am (next day)
    if (hour < t.turnOnHour || hour > t.turnOffHour)
      // out of "turn on" hours ("next day" timer)
      return false;
  }

  // checks if before initial minute
  int minute = now.minute();
  if (hour == t.turnOnHour && minute < t.turnOnMinute)
    return false;

  // checks if after final minute
  if (hour == t.turnOffHour && minute >= t.turnOffMinute)
    return false;

  return true;
}


void enableTimer(unsigned int index) {
  //
  // enables timer new timer
  //
  if (!USE_RELAY_TIMERS || index >= MAX_NUMBER_OF_TIMERS || 
    (RELAY_TIMERS[index].enabled && RELAY_TIMERS[index].active))
    // invalid or already enabled    
    return;

  #if (DEBUG_MODE == true) 
  Serial.print(F(" - Enabling timer: "));
  Serial.println(index);    
  #endif

  RELAY_TIMERS[index].enabled = true;
  RELAY_TIMERS[index].active = true;
  saveTimers(true);
}


void disableTimer(unsigned int index) {
  //
  // enables timer new timer
  //
  if (!USE_RELAY_TIMERS || index >= MAX_NUMBER_OF_TIMERS || !RELAY_TIMERS[index].enabled)
    // invalid or already disabled
    return;

  #if (DEBUG_MODE == true) 
  Serial.print(F(" - Disabling timer: "));
  Serial.println(index);
  #endif

  RELAY_TIMERS[index].enabled = false;
  RELAY_TIMERS[index].active = true;
  saveTimers(true);
}


void deleteTimer(unsigned int index) {
  //
  // enables timer new timer
  //
  if (!USE_RELAY_TIMERS || index >= MAX_NUMBER_OF_TIMERS || !RELAY_TIMERS[index].active)
    return;

  #if (DEBUG_MODE == true) 
  Serial.print(F(" - Deleting timer: "));
  Serial.println(index);    
  #endif

  RELAY_TIMERS[index].active = false;
  saveTimers(false);
}


void addTimer(byte relay, const byte& hourOn, const byte& minuteOn, const byte& hourOff, const byte& minuteOff) {
  //
  // adds new timer
  //
  if (!USE_RELAY_TIMERS)
    return;

  if ((hourOn == hourOff && minuteOn == minuteOff) || relay >= RELAY_SIZE){
    #if (DEBUG_MODE == true) 
    Serial.print(F("ERROR creating timer: invalid parameters!"));
    #endif
    return;
  }

  for (unsigned int i = 0; i < MAX_NUMBER_OF_TIMERS; i++) {
    if (!RELAY_TIMERS[i].active){
      // adds on first free position
      RELAY_TIMERS[i].enabled = true;
      RELAY_TIMERS[i].relay = relay;
      RELAY_TIMERS[i].turnOnHour = hourOn;
      RELAY_TIMERS[i].turnOnMinute = minuteOn;
      RELAY_TIMERS[i].turnOffHour = hourOff;
      RELAY_TIMERS[i].turnOffMinute = minuteOff;
      RELAY_TIMERS[i].active = true;
      saveTimers(false);

      #if (DEBUG_MODE == true) 
      // debug only
      Serial.print(F("new timer created "));
      printTimer(RELAY_TIMERS[i], i);
      #endif
      return;
    }
  }  

  #if (DEBUG_MODE == true) 
  Serial.print(F("could not save timer: no free space!"));
  #endif
}


void updateTimer(const byte& id, const byte& hourOn, const byte& minuteOn, const byte& hourOff, const byte& minuteOff) {
  //
  // updates the specified timer
  //
  if (!USE_RELAY_TIMERS)
    return;

  unsigned int index = (unsigned int)id;
  if (index >= MAX_NUMBER_OF_TIMERS || (hourOn == hourOff && minuteOn == minuteOff)){
    #if (DEBUG_MODE == true) 
    Serial.print(F("ERROR updating timer: invalid parameters!"));
    #endif
    return;
  }

  #if (DEBUG_MODE == true) 
  Serial.print(F("updating timer - id: "));
  Serial.println(index);
  
  Serial.print(F("before "));
  printTimer(RELAY_TIMERS[index], index);
  #endif

  RELAY_TIMERS[index].enabled = true;
  RELAY_TIMERS[index].turnOnHour = hourOn;
  RELAY_TIMERS[index].turnOnMinute = minuteOn;
  RELAY_TIMERS[index].turnOffHour = hourOff;
  RELAY_TIMERS[index].turnOffMinute = minuteOff;
  RELAY_TIMERS[index].active = true;
  saveTimers(false);

  #if (DEBUG_MODE == true) 
  Serial.print(F("after "));
  printTimer(RELAY_TIMERS[index], index);
  #endif
}


bool saveTimers(const bool& saveStates) {
  //
  // saves timers into EEPROM (255 bytes)
  //
  // remark: An EEPROM write takes 3.3 ms to complete. 
  // The EEPROM memory has a specified life of 100,000 
  // write/erase cycles, so you may need to be careful 
  // about how often you write to it.
  //
  if (!USE_RELAY_TIMERS)  
    return false;

  // gets EEPROM size
  unsigned int nsize = EEPROM.length();
  #if (DEBUG_MODE == true) 
  Serial.println(F("saving timers..."));
  Serial.print(F("EEPROM size: "));
  Serial.print(nsize);
  Serial.println(F(" bytes"));
  #endif

  // saves checker flag
  unsigned int address = EEPROM_ADDRESS;  
  // saves EEPROM DATA CHECK (flags that EEPROM is actually written)
  EEPROM.update(address++, START);  
  
  // saves internal states (if required)
  if (saveStates){
    byte states = 0x00;
    if (_isTimersEnabled)
      bitSet(states, 0);
    EEPROM.update(address, states);    
    #if (DEBUG_MODE == true) 
    Serial.print(F("saving states: 0x"));
    Serial.println(String(states, HEX));
    #endif    
  }
  else {    
    address++;
    #if (DEBUG_MODE == true) 
    Serial.println(F("skipping states..."));
    #endif
  }
  
  // saves number of timers
  unsigned int n = getTimersCount();
  if (n == 0) {
    EEPROM.update(address++, 0x00);
    #if (DEBUG_MODE == true) 
    Serial.println(F("no timers!"));
    #endif    
    return false;
  }

  #if (DEBUG_MODE == true) 
  Serial.print(n);
  Serial.println(F(" timers"));
  #endif

  unsigned int nrequied = address + 5 * n; // 5 bytes per timer
  if (nrequied > nsize) {
      #if (DEBUG_MODE == true) 
      Serial.println(F("the timers data exceeds EEPROM size")); 
      #endif
      // fit
      while(address + 5 * n > nsize){
        n--;
      }

      #if (DEBUG_MODE == true) 
      Serial.println(F("only "));
      Serial.print(n);
      Serial.println(F(" timers will be saved!"));
      #endif
  }
  EEPROM.update(address++, (byte)n);

  // saves active timers (deactived timers are not persisted)
  unsigned int counter = 0;
  for (unsigned int i = 0; i < MAX_NUMBER_OF_TIMERS; i++) 
    if (RELAY_TIMERS[i].active && is_valid_timer(RELAY_TIMERS[i])) {
      if (counter < n) {
        // handles enabled/relay number (same byte)
        byte relay = RELAY_TIMERS[i].relay;
        if (RELAY_TIMERS[i].enabled)
          bitSet(relay, 7);
        EEPROM.update(address++, relay);
        // turn on/off times of day
        EEPROM.update(address++, RELAY_TIMERS[i].turnOnHour);
        EEPROM.update(address++, RELAY_TIMERS[i].turnOnMinute);
        EEPROM.update(address++, RELAY_TIMERS[i].turnOffHour);
        EEPROM.update(address++, RELAY_TIMERS[i].turnOffMinute);
        #if (DEBUG_MODE == true)       
        printTimer(RELAY_TIMERS[i], counter);      
        #endif
      }
      else {
        #if (DEBUG_MODE == true)   
        // list not save timers    
        Serial.print(F("NOT SAVED: "));
        printTimer(RELAY_TIMERS[i], counter);      
        #endif
      }
      counter++;
    }

  // return true if all saved
  return counter == n;
}


bool loadTimers(const bool& loadStates) {
  //
  // loads timers from EEPROM
  //
  if (!USE_RELAY_TIMERS)
    return false;
  
  #if (DEBUG_MODE == true) 
  Serial.println("reading timers from EEPROM");
  #endif

  // clean all
  cleanTimers();

  // checks EEPROM for previously written data
  unsigned int address = EEPROM_ADDRESS;
  if (EEPROM.read(address++) != START) {
    #if (DEBUG_MODE == true) 
    Serial.println("Nothing previously saved on EEPROM!");
    #endif
    return false;
  }

  #if (DEBUG_MODE == true) 
  Serial.println("previous data found on EEPROM");
  #endif

  if (loadStates) {
    byte states = EEPROM.read(address++);
    _isTimersEnabled = (bitRead(states, 0) == 1);
    #if (DEBUG_MODE == true) 
    Serial.print(F("timers enabled: "));
    Serial.println(_isTimersEnabled? F("on") : F("off"));
    #endif
  }
  else {
    address++;
  }
  
  // retrieves number of timers to load
  unsigned int n = EEPROM.read(address++);
  #if (DEBUG_MODE == true)   
  if (n == 0)
    Serial.print(F("no"));
  else 
    Serial.print(n);
  Serial.println(F(" timers"));
  #endif
  
  // retrieves timers from EEPROM
  for (unsigned int i = 0; i < n; i++) {
    // handles enabled/relay number (same byte)
    byte relay = EEPROM.read(address++);
    RELAY_TIMERS[i].enabled = (bitRead(relay, 7));
    bitClear(relay, 7);
    RELAY_TIMERS[i].relay = relay;
    // turn on/off times od day
    RELAY_TIMERS[i].turnOnHour = EEPROM.read(address++);
    RELAY_TIMERS[i].turnOnMinute = EEPROM.read(address++);
    RELAY_TIMERS[i].turnOffHour = EEPROM.read(address++);
    RELAY_TIMERS[i].turnOffMinute = EEPROM.read(address++);
    RELAY_TIMERS[i].active = true; 

    #if (DEBUG_MODE == true) 
    printTimer(RELAY_TIMERS[i], i);
    #endif
  }
}

bool readConfig() {
  //
  // reads configuration (EEPROM)
  //
  _isTimersEnabled = getBoolState(0);
}

bool saveConfig() {
  //
  // saves configuration (EEPROM)
  //
  setBoolState(_isTimersEnabled, 0);
}

bool getBoolState(const unsigned int& id) {
  //
  // gets boolean configuration (EEPROM)
  //
  unsigned int address = EEPROM_ADDRESS;
  if (EEPROM.read(address++) != START) 
    return false;    
  byte states = EEPROM.read(address);
  return bitRead(states, id) == 1;
}

void setBoolState(const bool& value, const unsigned int& id) {
  //
  // sets boolean configuration (EEPROM)
  //
  unsigned int address = EEPROM_ADDRESS;
  if (EEPROM.read(address++) != START) 
    return;
  byte states = EEPROM.read(address);
  if (value)
    bitSet(states, id);
  else
    bitClear(states, id);
  EEPROM.update(address, states);
}

void cleanTimers() {
  //
  // reset all timers 
  //
  if (!USE_RELAY_TIMERS)
    return;

  for (unsigned int i = 0; i < MAX_NUMBER_OF_TIMERS; i++) {
    RELAY_TIMERS[i].enabled = false;
    RELAY_TIMERS[i].relay = 0x00;
    RELAY_TIMERS[i].turnOnHour = 0x00;
    RELAY_TIMERS[i].turnOnMinute = 0x00;
    RELAY_TIMERS[i].turnOffHour = 0x00;
    RELAY_TIMERS[i].turnOffMinute = 0x00;
    RELAY_TIMERS[i].active = false;
  }

  #if (DEBUG_MODE == true) 
  Serial.println(F("timers cleaned"));
  printTimers();
  #endif
}


unsigned int getTimersCount() {
  //
  // returns the number of active timers
  //
  if (!USE_RELAY_TIMERS)
    return 0;

  unsigned int count = 0;
  for (unsigned int i = 0; i < MAX_NUMBER_OF_TIMERS; i++) {     
    // for each timer, check if its active (and valid)
    if (RELAY_TIMERS[i].active && is_valid_timer(RELAY_TIMERS[i]))
      count++;
  }

  return count;
}

#if (USE_MOISTURE_SENSOR == true)
int readMoistureSensor() {
  //
  // reads the soil moisture sensor 
  //
  // remark: this sensor evaluates the soil conductivity 
  // (in range [0-1024]), therefore:
  // - lower value = WET soil
  // - higher value  = DRY soil
  //
  if (!USE_MOISTURE_SENSOR){
    _soilMoistureLevel = 0x00;
    return SOIL_DRY;
  }

  // turns the sensor on
	digitalWrite(MOISTURE_SENSOR_PIN_PWR, HIGH);	
  // allows power to settle
	delay(10);
  
  // read value [0-1024]
	int value = analogRead(MOISTURE_SENSOR_PIN_SIG);	
  // cap & floor
  if (value < 0) value = 0;
  if (value > 1024) value = 1024;  
  #if (DEBUG_MODE == true) 
  Serial.print(F(" - moisture sensor: "));
  Serial.print(value);
  #endif
  
  // byte: [0 - 100]
  _soilMoistureLevel = (byte)(100 - ceil((float)value/10.24));
  
  #if (DEBUG_MODE == true) 
  Serial.print(F(" [ "));
  Serial.print((int)_soilMoistureLevel);
  Serial.println(F("% ]"));    
  #endif

  // turns the sensor off
	digitalWrite(MOISTURE_SENSOR_PIN_PWR, LOW);
  delay(10);		  
	return value;
}
#endif

void setDefaultTimers() {
  //
  // reset all timers
  //
  if (!USE_RELAY_TIMERS)
    return;

  cleanTimers();

  // watering: 06:00 - 06:03 (relay 1)
  RELAY_TIMERS[WATERING_RELAY].enabled = true;
  RELAY_TIMERS[WATERING_RELAY].relay = 0;
  RELAY_TIMERS[WATERING_RELAY].turnOnHour = 6;
  RELAY_TIMERS[WATERING_RELAY].turnOnMinute = 0;
  RELAY_TIMERS[WATERING_RELAY].turnOffHour = 6;
  RELAY_TIMERS[WATERING_RELAY].turnOffMinute = 3;
  RELAY_TIMERS[WATERING_RELAY].active = true;

  // lights: 19:00 - 23:00 (relay 2)
  RELAY_TIMERS[1].enabled = true;  
  RELAY_TIMERS[1].relay = 1;
  RELAY_TIMERS[1].turnOnHour = 19;
  RELAY_TIMERS[1].turnOnMinute = 0;
  RELAY_TIMERS[1].turnOffHour = 23;
  RELAY_TIMERS[1].turnOffMinute = 0;  
  RELAY_TIMERS[1].active = true;

  // lights: 19:30 - 20:30 (relay 3)
  RELAY_TIMERS[2].enabled = true;
  RELAY_TIMERS[2].relay = 2;
  RELAY_TIMERS[2].turnOnHour = 19;
  RELAY_TIMERS[2].turnOnMinute = 30;
  RELAY_TIMERS[2].turnOffHour = 23;
  RELAY_TIMERS[2].turnOffMinute = 30;
  RELAY_TIMERS[2].active = true;

  #if (DEBUG_MODE == true) 
  Serial.println(F("default timers loaded!"));
  printTimers();
  #endif
}

#if (DEBUG_MODE == true) 
//
// Debuging functions
//
void dummySerialInput(){    
  //
  // Gets user input and store a sequence of bytes to 
  // emulate ESP-01 comunication
  //
  if (_debugCacheLenght > 0)
    return;

  _debugCacheIndex = 0;
  _debugCacheLenght = 0;
  
  // waits for debug command
  while (Serial.available() == 0) {
    // input message
    Serial.println(F("[debug] enter command: "));
    _blink = !_blink;
    digitalWrite(LED_BUILTIN, (_blink ? HIGH: LOW));
    
    int counter = 0;
    while (Serial.available() == 0 & counter++ < 100) {
      delay(100);
    }
  }  
  // gets debug command
  delay(500);
  String text =  Serial.readString();
  text.trim();
  Serial.print(F(">> "));
  Serial.println(text);
    
  if (text == F("mem")) {
    _debugCacheIndex = 0;
    _debugCacheLenght = 0;
    Serial.println(availableMemory());
    return;
  }  

  if (text == F("hello") || text == F("h")) {
    _debugCacheIndex = 0;
    _debugCacheLenght = 0;
    Serial.println(F("Hello! how's doing?"));
    return;
  }

  if (text == F("n")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = NOP;
    _debugCacheLenght = 1;
    Serial.println(F("NOP!"));
    return;
  }

  if (text == F("nop")) {
    _debugCacheIndex = 0;
    _debugCache[0] = START;
    _debugCache[1] = NOP; // 0x00
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    Serial.println(F("NOP!"));
    return;
  }

  if (text == F("connected") || text == F("c")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = CONNECTED; // 0x01
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    return;
  }

  if (text == F("disconnected")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = DISCONNECTED; // 0x02
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    return;
  }

  if (text == F("list stats") || text == F("s")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = STATES; // 0xD0
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    return;
  }

  if (text == F("list relays") || text == F("r")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = LIST_RELAYS; // 0xD1
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    return;
  }

  if (text == F("list timers") || text == F("t")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = LIST_TIMERS; // 0xD2
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    return;
  }

  if (text == F("load timers") || text == F("load")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = LOAD_TIMERS; // 0xD3
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    return;
  }

  if (text == F("save timers") || text == F("save")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = SAVE_TIMERS; // 0xD4
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    return;
  }

  if (text == F("update") || text == F("u")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = UPDATE; // 0xD5
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    return;
  }
  
  if (text == F("reset")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = RESET; // 0x04
    _debugCache[2] = END;
    _debugCacheLenght = 3;    
    return;
  }

  if (text == F("relays on")) {
    _debugCacheIndex = 0;
    _debugCache[0] = START;
    _debugCache[1] = ALL_RELAYS_ON; // 0x05 
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    return;
  }

  if (text == F("relays off")) {
    _debugCacheIndex = 0;
    _debugCache[0] = START;
    _debugCache[1] = ALL_RELAYS_OFF; // 0x06 
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    return;
  }

  if (text == F("enable timers")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = ENABLE_TIMERS; // 0xC0
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    return;
  }

  if (text == F("disable timers")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = DISABLE_TIMERS; // 0xC1
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    return;
  }

  if (text == F("default timers")) {
    _debugCacheIndex = 0;
    _debugCache[0] = START;
    _debugCache[1] = SET_DEFAULT_TIMERS; // 0xC8 <<
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    return;
  }

  if (text == F("delete timers")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = DELETE_ALL_TIMERS; // 0xC6
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    return;
  }

  if (text == F("relay on")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = 0xA1; // 0xA1 (relay 2)
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    return;
  }

  if (text == F("relay off")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = 0xB1; // 0xB1 (relay 2)
    _debugCache[2] = END;
    _debugCacheLenght = 3;
    return;
  }

  if (text == F("sync")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = SYNC; // 0x03
    _debugCache[2] = 0x19; // dd
    _debugCache[3] = 0x09; // MM
    _debugCache[4] = 0x23; // yy
    _debugCache[5] = 0x19; // hh
    _debugCache[6] = 0x20; // mm
    _debugCache[7] = 0x50; // ss
    _debugCache[8] = END;
    _debugCacheLenght = 9;
    return;
  }

  if (text == F("enable timer")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = ENABLE_TIMER; // 0xC3
    _debugCache[2] = 0x01; // timer id
    _debugCache[3] = END;
    _debugCacheLenght = 4;
    return;
  }

  if (text == F("disable timer")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = DISABLE_TIMER; // 0xC4
    _debugCache[2] = 0x01; // timer id
    _debugCache[3] = END;
    _debugCacheLenght = 4;
    return;
  }

  if (text == F("delete timer")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = DELETE_TIMER; // 0xC5
    _debugCache[2] = 0x01; // timer id
    _debugCache[3] = END;
    _debugCacheLenght = 4;
    return;
  }

  if (text == F("add timer")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = ADD_TIMER; // 0xC2
    _debugCache[2] = 3; // relay
    _debugCache[3] = 12; // hh
    _debugCache[4] = 30; // mm
    _debugCache[5] = 13; // hh
    _debugCache[6] = 40; // mm
    _debugCache[7] = END;
    _debugCacheLenght = 8;
    return;
  }

  if (text == F("update timer")) {
    _debugCacheIndex = 0;    
    _debugCache[0] = START;
    _debugCache[1] = UPDATE_TIMER; // 0xC7
    _debugCache[2] = 03; // id
    _debugCache[3] = 14; // hh
    _debugCache[4] = 30; // mm
    _debugCache[5] = 15; // hh
    _debugCache[6] = 40; // mm
    _debugCache[7] = END;
    _debugCacheLenght = 8;
    return;
  }

  Serial.println(F("[debug] command not found"));
  Serial.flush();
}

bool dummySerialAvailable(){
  //
  // returns true if there are data at serial debug cache (i.e., "Serial.available() != 0")
  //
  return _debugCacheLenght > 0 && _debugCacheIndex < _debugCacheLenght;
}

byte dummySerialRead(){
  //
  // reads byte value from serial debug cache (i.e., Serial.read())
  //
  if (!dummySerialAvailable())
    return NOP;
  
  byte value = _debugCache[_debugCacheIndex];
  Serial.print(F(" - read: 0x"));
  Serial.print(String(value, HEX));
  Serial.print(F(" ("));
  Serial.print(String(value, DEC));
  Serial.println(F(")"));

  _debugCacheIndex++;
  if (_debugCacheIndex >= _debugCacheLenght)
    _debugCacheLenght = 0;
  return value;
}

void printTimers(){
  for(unsigned int i = 0; i < MAX_NUMBER_OF_TIMERS; i++) {
    if (RELAY_TIMERS[i].active && is_valid_timer(RELAY_TIMERS[i]))
      printTimer(RELAY_TIMERS[i], i);
  }
}

void printTimer(const timer& timer, const unsigned int& idx){
  Serial.print(F(" - id: "));
  Serial.print(idx);
  Serial.print(F(" ["));
  Serial.print(timer.enabled ? F("on") :  F("off"));
  Serial.print(F("], relay: "));
  Serial.print((unsigned int)timer.relay + 1);
  Serial.print(F(", on: "));
  Serial.print((unsigned int)timer.turnOnHour);
  Serial.print(F(":"));
  Serial.print((unsigned int)timer.turnOnMinute);
  Serial.print(F(", off: "));
  Serial.print((unsigned int)timer.turnOffHour);
  Serial.print(F(":"));
  Serial.println((unsigned int)timer.turnOffMinute);
}
#endif

int availableMemory() {
  int size = 8192;
  byte *buf;
  while ((buf = (byte *) malloc(--size)) == NULL);
  free(buf);
  return size;
}