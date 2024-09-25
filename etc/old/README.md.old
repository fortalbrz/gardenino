# GARDEN.INO 
## :house_with_garden: Garden Wifi with Home Assistant and a single Arduino Nano 

**Optimized for Arduino Nano R3 (ATmega328P)** 

I made this project to make use of an old 8 channels relay board with a single Arduino Nano and 
a really low cost *ESP-01* module... :grin: The goal is to control my garden irrigation, lights, etc, 
using *home assistant* with *MQTT* protocol (e.g., *mosquitto broker*) or autonomous with some 
"standalone" intelligence on Arduino Nano. Sure you can use a plain Arduino relay module instead :smiley:
(with any number of channels, with maximum of 8)...

![project resources](https://github.com/fortalbrz/gardenino/blob/main/nano%2Besp01/project_001.jpg?raw=true)

## Features:
 - works with home assistant to control garden watering and more 7 switches (MQTT), lights, *etc*
 - alternatively, works autonomous with up to 50 programmable timers (using real time clock - *optional, see [config flags](https://github.com/fortalbrz/gardenino/blob/main/README.md#configuration-flags)*)
 - soil moisture sensor to (avoids watering when the soil is wet - *optional, see [config flags](https://github.com/fortalbrz/gardenino/blob/main/README.md#configuration-flags)*)
 - watering watchdog (prevents watering for long time... :potable_water: - *optional, see [config flags](https://github.com/fortalbrz/gardenino/blob/main/README.md#configuration-flags)*)   
 - turn on/off watering with local push button (*optional, see [config flags](https://github.com/fortalbrz/gardenino/blob/main/README.md#configuration-flags)*)


### Plain Arduino Nano and WiFi?

However, Arduino Nano is not easy integrated with Wifi (I could have used a NodeMCU instead, 
but I already got this Nano and plain ESP-01 module... And after all, what's the fun?  :zany_face:  kkk). 

To solve this issue, I made a sketch apart to be flashed on the *ESP8266-01* module ([esp8266-01.ino](https://github.com/fortalbrz/gardenino/blob/main/nano%2Besp01/esp8266-01/esp8266-01.ino))
(using board "*Generic ESP8266 Module*"). The ESP-01 should communicate with Home Assistant using MQTT and act as bridge (like "*man-in-the-middle*") to Arduino 
Nano using plain serial communication. 


![communication diagram](https://github.com/fortalbrz/gardenino/blob/main/nano%2Besp01/general_schema_001.png?raw=true)

Therefore, to use serial communication on this sketch to debug with "Serial Monitor" set the flag macro
"*[DEBUG_MODE true](https://github.com/fortalbrz/gardenino/blob/main/README.md#configuration-flags)*", otherwise the serial communication is intended to ESP-01 module itself. 


## Source code:
- https://github.com/fortalbrz/gardenino

**Drivers (CH340g)** for both Arduino Nano and ESP-01:
- [CH340g USB/Serial driver](https://bit.ly/44WdzVF) (windows 11 compatible driver)  
- driver install instructions ([pt-BR](https://bit.ly/3ZqIqc0))
- flashing ESP-01 tutorial ([pt-BR](https://bit.ly/3LRlZqT)) 

*REMARK*: this sketch should be pushed into an Arduino Nano board using the *ATmega328P* processor at Arduino IDE:
 - **Board**: "*Arduino Nano*"
 - **Processor**: "*ATmega328P (Old Bootloader)*"


### Configuration flags
  
Use these configurations flags to set *gardeino* behaviour (as you wish). You can avoid dispensable hardware 
and features to fit your needs...

| macro                      | default | description                                                                       |
|----------------------------|---------|-----------------------------------------------------------------------------------|
| RELAY_SIZE                 | 8       | number of relays (valid: 1 to 8)                                                  |
| USE_RELAY_TIMERS           | true    | true to use timers, false otherwise - dont need the RTC                           | 
| MAX_NUMBER_OF_TIMERS       | 50      | maximum number of standalone timers (valid: 3 to 200)                             |
| USE_MOISTURE_SENSOR        | true    | true to use moisture sensor, false otherwise - dont need the moisture sensor      | 
| SOIL_WET                   | 450     | defines max soil conductivity value we consider soil "wet" (valid: 0 to 1023)     |
| SOIL_DRY                   | 750     | defines min soil conductivity value we consider soil "dry" (valid: 0 to 1023)     |
| USE_WATERING_BUTTON        | true    | true to use push button, false otherwise - dont need the push button              |
| USE_WATERING_WATCHDOG      | true    | true to set a watering time limit, false otherwise                                |
| WATERING_WATCHDOG_INTERVAL | 300000  | watering time limit (milisseconds)                                                | 
| BAUD_RATE                  | 9600    | serial communication speed with ESP-01 (adjust both sketches)                     |
| DEBUG_MODE                 | false   | true to debug on serial monitor (debug), false to communicate with ESP-01 (prod)  |


## Materials:
- Arduino Nano R3 (ATmega328P)
- Wifi Module ESP8266-01 (ESP-01)
- [ESP8266 USB serial adapter CH340g](https://produto.mercadolivre.com.br/MLB-2052186432-esp-01-wifi-esp8266-adaptador-usb-serial-ch340g-arduino-_JM) (see [next session](https://github.com/fortalbrz/gardenino/edit/main/README.md#programming-the-esp8266-01-esp-01))
- Logic Level Converter (LLC) 5v-3.3v (bi-directional) (*see [note](https://github.com/fortalbrz/gardenino/edit/main/README.md#logic-level-converter-llc)*)
- Real Time Clock (RTC) DS3231 (*optional: uses for standalone timers support, see [config flags](https://github.com/fortalbrz/gardenino/blob/main/README.md#configuration-flags)*)
- solenoid valve 3/4" 110 v (normally closed)
- relay module 5v 8-ch (*optional: less than 8 channels can be used, see [config flags](https://github.com/fortalbrz/gardenino/blob/main/README.md#configuration-flags)*)
- soil moisture sensor (*optional: avoids watering when the soil is wet, see [config flags](https://github.com/fortalbrz/gardenino/blob/main/README.md#configuration-flags)*)
- push button and 10 k ohms resistor (*optional: turn on/off watering, see [config flags](https://github.com/fortalbrz/gardenino/blob/main/README.md#configuration-flags)*)
- power supply 5vdc (1A)

### Programming the ESP8266-01 (ESP-01)

The ESP-01 module should be programed with the sketch ([esp8266-01.ino](https://github.com/fortalbrz/gardenino/blob/main/nano%2Besp01/esp8266-01/esp8266-01.ino)), 
using [ESP8266 USB serial adapter CH340g](https://produto.mercadolivre.com.br/MLB-2052186432-esp-01-wifi-esp8266-adaptador-usb-serial-ch340g-arduino-_JM) as show bellow:

![ESP8266 USB serial adapter](https://github.com/fortalbrz/gardenino/blob/main/nano%2Besp01/project_002.jpg?raw=true)

One "*detail*" is that this USB/serial adapter can connect ESP-01 to USB (Arduino IDE), but **CAN'T** flash it :dizzy_face:. 
In order to program the ESP-01, a small modification is needed: add a push button over GPIO and GND pins :sunglasses:.

![ESP8266 USB serial adapter modification](https://github.com/fortalbrz/gardenino/blob/main/nano%2Besp01/project_003.jpg?raw=true)


![ESP8266 USB serial adapter modification](https://github.com/fortalbrz/gardenino/blob/main/nano%2Besp01/project_004.jpg?raw=true)
 
In order to write the code, press the button when connecting the USB Adapter to your computer's USB port :nerd_face:.

See more details in this flashing ESP-01 tutorial ([pt-BR](https://bit.ly/3LRlZqT)) 

## Circuit Wiring Instruction (step by step):

![wiring diagram](https://github.com/fortalbrz/gardenino/blob/main/nano%2Besp01/wiring_schema_001.png?raw=true)
 
- [circuito.io (step by step)](https://www.circuito.io/app?components=514,11022,13322,13678,821989)
   - Arduino Nano pin29 (GND) --> power supply 5vdc (negative/Gnd)
   - Arduino Nano pin27 (Vin) --> power supply 5vdc (positive/Vcc)
   - Arduino Nano pin27 (Vin) --> power supply 5vdc (positive/Vcc)
   - RTC DS3231 pin1 (GND) --> Arduino Nano pin29 (GND)
   - RTC DS3231 pin4 (SCL) --> Arduino Nano pin (A5)
   - RTC DS3231 pin3 (SDA) --> Arduino Nano pin (A4)
   - RTC DS3231 pin2 (VCC) --> Arduino Nano pin17 (3.3v)
   - ESP8266-01 pin8 (RXD) --> LLC pin1 (LV1)
   - ESP8266-01 pin1 (TXD) --> LLC pin2 (LV2)
   - ESP8266-01 pin7 (VCC) --> Arduino Nano pin17 (3.3v)
   - ESP8266-01 pin2 (GND) --> Arduino Nano pin29 (GND)
   - ESP8266-01 pin3 (CH_PD) --> Arduino Nano pin17 (3.3v)
   - LLC pin3 (LV) --> Arduino Nano pin17 (3.3v)
   - LLC pin3 (GND) --> Arduino Nano pin29 (GND)
   - LLC pin11 (HV2) --> Arduino Nano pin13 (D10)
   - LLC pin12 (HV1) --> Arduino Nano pin14 (D11)
   - LLC pin12 (HV) --> Arduino Nano pin27 (5v)
   - Relay 8 ch (VCC) --> Arduino Nano pin27 (5v)
   - Relay 8 ch (GND) --> Arduino Nano pin29 (GND)
   - Relay 8 ch (D0) --> Arduino Nano pin7 (D4)
   - Relay 8 ch (D1) --> Arduino Nano pin8 (D5)
   - Relay 8 ch (D2) --> Arduino Nano pin9 (D6)
   - Relay 8 ch (D3) --> Arduino Nano pin10 (D7)
   - Relay 8 ch (D4) --> Arduino Nano pin11 (D8)
   - Relay 8 ch (D5) --> Arduino Nano pin12 (D9)
   - Relay 8 ch (D6) --> Arduino Nano pin15 (D12)
   - Relay 8 ch (D7) --> Arduino Nano pin16 (D13)
   - Soil moisture sensor VIN (right) --> Arduino Nano pin6 (D3)
   - Soil moisture sensor GND (center) --> Arduino Nano pin29 (GND)
   - Soil moisture sensor SIG/A0 (left) --> Arduino Nano pin19 (A0)
   - push button (term 1) --> Arduino Nano pin16 (D2 - interruption) 
   - push button (term 1) --> Arduino Nano pin27 (5v)
   - push button (term 2) --> 10k olhms resistor (term 1)
   - 10k olhms resistor (term 2)  --> Arduino Nano pin29 (GND)

(*connected pins: D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12, D13, A0, A4, A5*)

## MQTT topics (handled by ESP-01):
 
- **gardenino/available**: sensors availability (*"online"/"offline"*)
- **gardenino/cmd**: pushes commands to Arduino Nano (*home assistant -> gardenino*):
  - "*water on/off*": turn on/off watering valve (i.e., *relay 1*)
  - "*relay 4 on/of*": turn on/of relay by number (1-8)
  - "*reset*": reset internal states
  - "*relays on/off*": turn on/off all relays
  - "*enable/disable timers*": enables/disables relay timers routine (avoid execution with out delete/update)
  - "*enable/disable timer: 04*": disable timer by "*id*" (checks MQTT state topic for gets to "*timer id*")
  - "*add timer: relay hh mm hh mm*": adds new relay timer (parameters: *relay number, turn on hour, turn on minute, turn off hour, turn off minute*)
  - "*update timer: id hh mm hh mm*": updates relay timer by "*id*" (parameters: *timer id, turn on hour, turn on minute, turn off hour, turn off minute*)
  - "*delete timer: 04*": delete timer by "*id*"
  - "*delete all timers*": deletes all relay timers (**warning**: *all timers will be erased!*)
  - "*default timers*": sets default relay timers (i.e. timers "*factory reset*", **warning**: *all timers will be erased!*)
  
- **gardenino/state**: retrieves states as json (*gardenino -> home assistant*)
  ```   
      {
          "moisture": 50,
          "timers_on": "on",
          "relays": {
               "water": "on",
               "relay 2": "off",
               "relay 3": "off",       
                ... 
          },
          "timers": {
               "timer_01": {
                    "id": 4,
                    "enabled": "on", 
                    "relay": 1,
                    "on": "12:20:00", 
                    "off": "15:30:00"}
                ...           
          },   
       }
  ```
- **ha/datetime**: (*optional*) creates a automation on home assistant in order to synchronize data & time :nerd_face:
  ```   
           alias: MQTT date time sync
           description: "syncronize data & time on client MQTT devices"
           trigger:
             - platform: time_pattern
               minutes: /1
           condition: []
           action:
             - service: mqtt.publish
               data:
                 qos: "0"
                 retain: false
                 topic: ha/datetime
                 payload_template: "{{ now().timestamp() | timestamp_custom('%d %m %Y %H %M %S') }}"
           mode: single
  ```

### Esp-Arduino "internal serial protocol": 

This is a code detail just for documentation purposes

#### protocol commands (ESP-01 -> Arduino Nano)

 These are the codes send from ESP-01 to Arduino Nano (as single bytes) (mostly as response to MQTT topic **gardenino/cmd**):

| code | minemonic          | MQTT topic cmd | description                                                                                   |
|------|--------------------|----------------|-----------------------------------------------------------------------------------------------|
| 0x00 | NOP                |                | not operation (NOP)                                                                           |
| 0x01 | CONNECTED          |                | notifies Arduino that MQTT is connected (CONNECTED)                                           |
| 0x02 | DISCONNECTED       |                | notifies that mqtt is disconnected (DISCONNECTED)                                             |
| 0x03 | SYNC               |                | datetime synchronization (uses home assistant as "time server" - SYNC)                        | 
| 0x04 | CLEAR              | clear          | clears states (CLEAR / mqtt: "clear")                                                         |
| 0x05 | ALL_RELAYS_ON      | relays on      | turns on all relays (ALL_RELAYS_ON / mqtt: "relays on")                                       |
| 0x06 | ALL_RELAYS_OFF     | relays off     | turns off all relays (ALL_RELAYS_OFF / mqtt: "relays off")                                    |
| 0xA0 |                    |                | turn on relay 1 (i.e., watering relay)                                                        |
| 0xA1 |                    |                | turn on relay 2                                                                               |
| 0xA2 |                    |                | turn on relay 3                                                                               |
| 0xA3 |                    |                | turn on relay 4                                                                               |
| 0xA4 |                    |                | turn on relay 5                                                                               |
| 0xA5 |                    |                | turn on relay 6                                                                               |
| 0xA6 |                    |                | turn on relay 7                                                                               |
| 0xA7 |                    |                | turn on relay 8                                                                               |
| 0xB0 |                    |                | turn off relay 1 (i.e., watering relay)                                                       |
| 0xB1 |                    |                | turn off relay 2                                                                              |
| 0xB2 |                    |                | turn off relay 3                                                                              |
| 0xB3 |                    |                | turn off relay 4                                                                              |
| 0xB4 |                    |                | turn off relay 5                                                                              |
| 0xB5 |                    |                | turn off relay 6                                                                              |
| 0xB6 |                    |                | turn off relay 7                                                                              |
| 0xB7 |                    |                | turn off relay 8                                                                              |
| 0xC0 | ENABLE_TIMERS      | enable timers  | enables timers routine (ENABLE_TIMERS / mqtt: "enable timers")                                |
| 0xC1 | DISABLE_TIMERS     | disable timers | enables timers routine (DISABLE_TIMERS / mqtt: "disable timers")                              |
| 0xC2 | ADD_TIMER          | add timer      | add new timer (ADD_TIMER / mqtt: "add timer: [*relay*] [*hh*] [*mm*] [*hh*] [*mm*]")          |
| 0xC3 | ENABLE_TIMER       | enable timer   | enables timer by id (ENABLE_TIMER / mqtt: "enable timer: [*id*]")                             |
| 0xC4 | DISABLE_TIMER      | disable timer  | disables timer by id (DISABLE_TIMER / mqtt: "disable timer: [*id*]")                          |
| 0xC5 | DELETE_TIMER       | delete timer   | deletes timer by id (DELETE_TIMER / mqtt: "delete timer: [*id*]")                             | 
| 0xC6 | DELETE_ALL_TIMERS  | delete timers  | deletes all timers (DELETE_ALL_TIMERS / mqtt: "delete timers")                                |
| 0xC7 | UPDATE_TIMER       | update timer   | updates timer by if (UPDATE_TIMER / mqtt: "update timer: [*id*] [*hh*] [*mm*] [*hh*] [*mm*]") |
| 0xC8 | SET_DEFAULT_TIMERS | default timers | sets default timers, i.e. "factory reset" (SET_DEFAULT_TIMERS /  mqtt: "default timers")      |
| 0xFE | START              |                | token for message start (START)                                                               |
| 0xFF | END                |                | token for message end (END)                                                                   |


#### Arduino States (Arduino Nano -> ESP-01)

These are the payloads send back from Arduino Nano to ESP-01 (mostly as response to MQTT topic **gardenino/state**):

- 1st byte: START token (START - 0xFE)
- 2nd byte: gardeino states
  - bit 0 - timers enabled
- 3th byte: relay states:
  - bit 0 - relay 1 state
  - bit 1 - relay 2 state
  - bit 2 - relay 3 state
  - bit 3 - relay 4 state
  - ...
- 4th byte: soil moisture in range [0-100] %
- 5th byte: number of timers (EEPROM)
- 6th byte: END token (END - 0xFF)
- next bytes (optional, for each active timer)
  - timer id (bit 7 = enable/disabled) [00-49]
  - relay number [01-08]
  - start hour in range [00-23]
  - start minute in range [00-59]
  - end hour in range [00-23]
  - end minute in range [00-59]

##### REMARK: debugging routines (DEBUG_MODE true) commands:
  - "mem"
  - "reset"
  - "sync"
  - "relay on/off"
  - "relays on/off"
  - "enable/disable timers"
  - "enable/disable timer""
  - "add timer"
  - "update timer"
  - "delete timer"
  - "delete all timers"
  - "default timers"


## Final notes

### Logic Level Converter (LLC)

Logic Level Converter (LLC) 5v-3.3v is not required if you make voltage divider (5v to 3.3v) for the RX serial communication.

In order to do that, all you will need is a 1k ohms and a 2.2k ohms resistors, and make these circuit changes:   

- 2.2k ohms resistor (terminal 1) --> Arduino Nano pin29 (GND)
- 2.2k ohms resistor (terminal 2) --> ESP8266-01 pin8 (RXD)
- 2.2k ohms resistor (terminal 2) --> 1k ohms resistor (terminal 1)
- 1k ohms resistor (terminal 1) --> Arduino Nano pin14 (D11) 



*[Jorge Albuquerque](mailto:jorgealbuquerque@gmail.com) (2022)*
