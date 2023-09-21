#include <EspMQTTClient.h>
//
//   GARDENINO - ESP8266 Module (MQTT)
// 
//   This sketch should be writen on a ESP8266 module (using board "Generic ESP8266 Module"), 
//   using the USB serial adapter CH340g. The ESP-01 should communicate with Home Assistant 
//   using MQTT and act as bridge (like "man-in-the-middle") to Arduino Nano using plain serial 
//   communication. (a NodeMCU could be used instead, but I already got this plain ESP-01...) :)
//
//    ------------------         ------------------                ----------                   ---------------- 
//   |  home assistant  |  <=>  | mosquitto broker |  <=[MQTT]=>  |  ESP-01  |   <=[serial]=>  |  Arduino Nano  |
//    ------------------         ------------------                ----------                   ---------------- 
//                                   MQTT broker         wifi         bridge      LLC 3.3v-5v 
//
//   Source code:
//   - https://github.com/fortalbrz/gardenino
//     (adds library "EspMQTTClient" by Patrick Lapointe at library manager)
//
//   Drivers:
//    - CH340g USB/Serial (windows 11 compatible driver): https://bit.ly/44WdzVF 
//         - driver install instructions (pt-BR): https://bit.ly/3ZqIqc0
//         - flashing ESP-01 tutorial (pt-BR): https://bit.ly/3LRlZqT 
//
//   Materials:
//   - Wifi Module ESP8266-01 (ESP-01)
//   - ESP8266 USB serial adapter CH340g (https://produto.mercadolivre.com.br/MLB-2052186432-esp-01-wifi-esp8266-adaptador-usb-serial-ch340g-arduino-_JM)
//
//   Circuit Wiring Instruction (step by step):
//   -  https://www.circuito.io/app?components=514,11022,13678,821989
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
//
//   MQTT topics:
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
//               mode: single//
//
//   Esp-Arduino "internal serial protocol": 
//
//    - MQTT commands [ESP-01 -> Arduino Nano]
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
//      0xC2 - add new timer (ADD_TIMER / mqtt: "add timer: [relay] [hh] [mm] [hh] [mm]")
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
//   Jorge Albuquerque (2022) - jorgealbuquerque@gmail.com
//
//---------------------------------------------------------------------------------------------------------------------
#define DEBUG_MODE false // true to debug on serial monitor (debug), false to communicate with Arduino (prod)
// configuration
#define RELAY_SIZE 8 // number of relays (valid: 1 to 8, defalut: 8)
#define MQTT_AVALIABILITY_INTERVAL 60000 // publishes on MQTT availability topic once per minute
//
// MQTT setup 
//
#define WIFI_SSID "wifi ssid" // wifi SSID
#define WIFI_PASSWORD "wifi password" // wifi password
#define MQTT_BROKER_ADDRESS "192.168.68.93" // MQTT broker server ip
#define MQTT_USERNAME  "mqtt username" // can be omitted if not needed
#define MQTT_PASSWORD  "mqtt password" // can be omitted if not needed
// MQTT topics 
const char* MQTT_AVAILABILITY_TOPIC  = "gardenino/available"; // topic to check gardenino availability state
const char* MQTT_COMMAND_TOPIC = "gardenino/cmd"; // topic to control gardenino
const char* MQTT_DATETIME_SYNC_TOPIC = "ha/datetime"; // optional create an automation to sync datetime with home assistant
const char* MQTT_ONLINE_MSG = "online";
const char* MQTT_OFFLINE_MSG = "offline";
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
#define RELAY_ON 0xA0
#define RELAY_OFF 0xB0
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

#define BAUD_RATE 9600 // serial communication speed with ESP-01 (adjust both sketches)
#define MESSAGE_DATA_LENGHT 4 // Arduino nano states message lenght (4 bytes!)

// command parameters
struct parameters{
   bool empty;
   unsigned int count;
   byte data[5];
};

// json message structure
const char json_structure[] = "{\"moisture\": [moisture], \"timers_on\": [timers_on]}, \"relays\": {\"water\": [0], \"relay_02\": [1], \"relay_03\": [2], \"relay_04\": [3], \"relay_05\": [4], \"relay_06\": [5], \"relay_07\": [6], \"relay_08\": [7]}, \"timers\": { [timers] }}";

bool _blink = false;
unsigned int _alive = 0;
unsigned long _lastAlive = millis();


EspMQTTClient client(
  WIFI_SSID,
  WIFI_PASSWORD,
  MQTT_BROKER_ADDRESS,
  MQTT_USERNAME, 
  MQTT_PASSWORD,
  "gardenino",
  1883
);

void setup() {
  //
  // initialization
  //
  pinMode(LED_BUILTIN, OUTPUT);  
  
  #if (DEBUG_MODE == true)
  // Serial debug on serial monitor
  Serial.begin(9600); // default
  Serial.println(F(" "));
  Serial.println(F(" "));
  Serial.print(F("initializing: "));
  Serial.println(F(" "));
  #else
  //Serial communication with Arduino Nano
  Serial.begin(BAUD_RATE);
  Serial.setTimeout(800);
  #endif
  
  delay(150);
  
  client.enableDebuggingMessages(DEBUG_MODE);
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
  client.enableOTA(); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  client.enableLastWillMessage(MQTT_AVAILABILITY_TOPIC, MQTT_OFFLINE_MSG);  // You can activate the retain flag by setting the third parameter to true
  
  client.enableMQTTPersistence();
  // client.enableDrasticResetOnConnectionFailures();

  if (DEBUG_MODE)
    Serial.println(F("ok..."));
}

void loop() {
  //
  // main loop
  //

  // build-in led blinking
  digitalWrite(LED_BUILTIN, (_blink ? HIGH: LOW));
  _blink = !_blink;  
  
  // keeps MQTT connection alive
  client.loop();

  if (!client.isConnected()){
    #if (DEBUG_MODE == true)
    Serial.println(F("disconnected"));
    #else 
    // send "mqtt disconnected" signal to Arduino Nano
    Serial.write(START);
    Serial.write(DISCONNECTED);
    Serial.write(END);      
    #endif
    Serial.flush();
    delay(5000);
    return;
  }

  #if (DEBUG_MODE == true)
  // alive ping!
  _alive = (++_alive) % 10;
  if (_alive == 0)
    Serial.println(F("[alive]"));
  #endif
  
  //
  // reads message data from Arduino Nano 
  // (in order to push it back to Home Assistant)
  //
  int counter = -1;
  byte data[MESSAGE_DATA_LENGHT];
  if (Serial.available() > 0) {    
    byte token = Serial.read();
    // "serial protocol" definition
    //   - 1st byte: START token
    //   - 2nd byte: gardeino states
    //   - 3th byte: relay states 
    //   - 4th byte: soil moisture in range [0-100] %
    //   - 5th byte: number of timers (EEPROM) [0-50]
    //   - 6th byte: END token
    //   - next bytes (optional, for each active timer)
    //      - timer id (bit 7 = enable/disabled)
    //      - start hour in range [0-24]
    //      - start minute in range [0-60]
    //      - end hour in range [0-24]
    //      - end minute in range [0-60]
    if (token == START) {
      // starts message (START token found!)
      counter = 0;
    } else if (counter >= 0) {
      if (token == END) {
        // end of message (END TOKEN found!)
        if (counter == MESSAGE_DATA_LENGHT){
          // valid message: expected message lenght checked!
          // reads timers data in sequence (if required)
          unsigned int n = 6 * (unsigned int)data[4];
          byte timers_data[n];      
          for (unsigned int i = 0; i < n; i++)
            timers_data[i] = 0x00;
          for (unsigned int i = 0; i < n && Serial.available() > 0; i++){
            timers_data[i] = Serial.read();
            delay(10);
          }          
          // publishes data to home assistant (as json topic)
          publishStates(data, timers_data);
        }
        // resets counter
        counter = -1;
      }
      else if (counter < MESSAGE_DATA_LENGHT)
        // reads message data byte
        data[counter++] = token;
    }     
  }

  // waits
  delay(1000);
  
  if((millis() - _lastAlive) > MQTT_AVALIABILITY_INTERVAL){
    // sends a MQTT "availability" messages to home assistant
    client.publish(MQTT_AVAILABILITY_TOPIC, MQTT_ONLINE_MSG);    
    #if (DEBUG_MODE == true)
    Serial.println(F("MQTT available send "));
    #endif
    _lastAlive = millis();
  }
}


void onConnectionEstablished(){
  //
  // This function is called once everything is connected (Wifi and MQTT)
  //
  #if (DEBUG_MODE == true)
  Serial.println(F("MQTT connected"));
  #endif

  // subscribe to home assistant commands topic
  client.subscribe(MQTT_COMMAND_TOPIC, onCommandReceived);
  delay(150);

  client.subscribe(MQTT_DATETIME_SYNC_TOPIC, onCommandReceived);
  delay(150);
  
  // notifies home assistant of the connection status
  client.publish(MQTT_AVAILABILITY_TOPIC, MQTT_ONLINE_MSG);
  delay(150);
  client.publish(MQTT_AVAILABILITY_TOPIC, MQTT_ONLINE_MSG);
  delay(150);

  #if (DEBUG_MODE == true)
  Serial.println(F("connected: ok"));
  #else
  // sends "mqtt connected notification to Arduino Nano"
  Serial.write(START);
  Serial.write(CONNECTED);
  Serial.write(END);
  #endif
  Serial.flush();
  delay(500);
}


void onCommandReceived(const String& topic, const String& command) {
  //
  // MQTT command received: sends message to Arduino Nano
  //
  #if (DEBUG_MODE == true)
  Serial.print(F("topic: "));
  Serial.println(topic);
  Serial.print(F("command: "));
  Serial.println(command);
  #endif

  if (topic == MQTT_COMMAND_TOPIC){
    parseCommand(command);
    return;
  }

  if (topic == MQTT_DATETIME_SYNC_TOPIC){
    parseTimeSync(command);
    return;
  }
}

void parseCommand(const String& command) {
  //
  // parses MQTT commands (with parameter, if applicable)
  //
  parameters param;
  char code = textToCode(command, param);  
  if (code != NOP) {
    #if (DEBUG_MODE == true)
    // debug only
    Serial.println(F("START")); 
    Serial.print(F("0x"));
    Serial.println(String(code, HEX));
    if (!param.empty){
      for (unsigned int i = 0; i < param.count; i++)
        Serial.println((unsigned int)param.data[i]);
    }
    Serial.println(F("END")); 
    Serial.println(F(" "));
    #else    
    // sends command and parameters to Arduino Nano
    Serial.write(START);
    Serial.write(code);
    if (!param.empty){
      for (unsigned int i = 0; i < param.count; i++)
        Serial.write(param.data[i]);
    }
    Serial.write(END);
    #endif        
    Serial.flush();
    delay(500);
  }
}

void parseTimeSync(const String& message) {
  //
  // Home assistant datetime sync
  // 
  // message format: "dd MM yyyy hh mm ss"
  //                  1234567890123456789  
  if (message.length() != 19) {
    #if (DEBUG_MODE == true)
    Serial.print(F("Time sync: "));
    Serial.println(message);
    Serial.print(F("Invalid message sync size (expected 19): "));
    Serial.println(message.length());
    #endif
    return;
  }

  #if (DEBUG_MODE == true)
  Serial.print(F("Time sync: "));
  Serial.println(message);
  Serial.println(F("START"));
  Serial.println(F("SYNC"));
  Serial.println(message.substring(0,2).toInt());   // dd [01-31]
  Serial.println(message.substring(3,5).toInt());   // MM [01-12]
  Serial.println(message.substring(8,10).toInt());  // yy [00-99]
  Serial.println(message.substring(11,13).toInt()); // hh [00-23]
  Serial.println(message.substring(14,16).toInt()); // mm [00-59]
  Serial.println(message.substring(17,19).toInt()); // ss [00-59]
  Serial.println(F("END"));
  Serial.println(F(" "));
  #else 
  //
  // writes timer sync message:
  //   bytes: [START, SYNC, dd, MM, yy, hh, mm, ss, END]
  //
  Serial.write(START);
  Serial.write(SYNC);
  Serial.write((byte)message.substring(0,2).toInt());   // dd [01-31]
  Serial.write((byte)message.substring(3,5).toInt());   // MM [01-12]
  Serial.write((byte)message.substring(8,10).toInt());  // yy [00-99]
  Serial.write((byte)message.substring(11,13).toInt()); // hh [00-23]
  Serial.write((byte)message.substring(14,16).toInt()); // mm [00-59]
  Serial.write((byte)message.substring(17,19).toInt()); // ss [00-59]
  Serial.write(END);
  #endif 
  Serial.flush();  
  delay(500);  
}


void publishStates(byte data[], byte timers[]){
  //
  // publishes states on MQTT broker
  //
  if (sizeof(data) != MESSAGE_DATA_LENGHT)
    return;

  // "serial protocol" definition (excluding START / END tokens)
  //      - byte 0: gardeino states
  //          bit 0 - timers enabled (on/off)
  //      - byte 1: relay states:
  //          bit 0 - relay 1 state (on/off) - watering relay
  //          bit 1 - relay 2 state (on/off)
  //          bit 2 - relay 3 state (on/off)
  //          ...
  //      - byte 2: soil moisture in range [0-100] %
  //      - byte 3: number of timers (EEPROM)
  byte states = data[0];
  byte relay_states = data[1];
  unsigned int soil_moisture = (unsigned int)data[2];
  unsigned int timers_count = (unsigned int)data[3];
  
  #if (DEBUG_MODE == true)
  Serial.println(F("publish states: "));
  Serial.print(F(" - states: "));
  Serial.println(String(states, BIN));
  Serial.print(F(" - relay states: "));
  Serial.println(String(relay_states, BIN));
  Serial.print(F(" - soil moisture: "));
  Serial.print(String(soil_moisture, DEC));
  Serial.println(F("%"));
  Serial.print(F(" - timers_count: "));
  Serial.println(String(timers_count, DEC));
  #endif
  
  // creates json message body
  String json = String(json_structure);

  json.replace(F("[moisture]"), toStr(soil_moisture));
  json.replace(F("[timers_on]"), toStr(bitRead(states, 0) == 1));
  
  for (unsigned int i = 0; i < RELAY_SIZE; i++)
    json.replace(F("[") + String(i, DEC) + F("]"), toStr(bitRead(relay_states, i) == 1));

  //
  //  5 bytes for each active timer
  //   - timer id (bit 7 = enable/disabled)
  //   - start hour in range [0-24]
  //   - start minute in range [0-60]
  //   - end hour in range [0-24]
  //   - end minute in range [0-60] 
  //   
  String timers_json = String("");
  if (timers_count > 0 && sizeof(timers) == 6 * timers_count)
    // active valid timers found!
    for (unsigned int i = 0; i < timers_count; i++){
      // for each timer (6 bytes per timer)
      unsigned int idx = 6 * i;
      // retrives enabled state (bit 7)
      bool enabled = bitRead(timers[idx], 7) == 1;
      bitClear(timers[idx], 7);
      // retrieves "timer id" [0-49]
      unsigned int id = (unsigned int)timers[idx];
      // retrieves relay number [1-8]
      unsigned int relay = (unsigned int)timers[idx + 1];
      // retrives "turn on" time of day ([0-23]:[0-59])
      unsigned int hourOn = (unsigned int)timers[idx + 2];
      unsigned int minuteOn =  (unsigned int)timers[idx + 3];
      // retrives "turn off" time of day ([0-23]:[0-59]) 
      unsigned int hourOff =  (unsigned int)timers[idx + 4];
      unsigned int minuteOff =  (unsigned int)timers[idx + 5];
      // creates json
      timers_json += F("\"timer_") + toStr(i) + F("\": {\"id\": ") + toStr(id) + F(", \"enabled\": ") + toStr(enabled) + F(",");
      timers_json += F("\"relay\": ") + toStr(relay) + F(", ");
      timers_json += F("\"on\": \"") + toStr(hourOn) +F(":") + toStr(minuteOn)  + F(":00\", ");
      timers_json += F("\"off\": \"") + toStr(hourOff) + F(":") + toStr(minuteOff)  + F(":00\"}, ");
    }
  json.replace(F("[timers]"), timers_json);
  
  #if (DEBUG_MODE == true)
  Serial.print(F("states: "));
  Serial.println(json);
  Serial.println(F(" "));
  #endif

  // publish!
  client.publish(F("gardenino/status"), json);
  delay(150);
}


byte textToCode(const String& command, parameters& param){  
  //
  // Encodes "serial protocol" commands (bytes)
  //
  param.empty = true;
  
  if (command == F("reset"))
    // reset signal
    return RESET;

  if (command == F("relays on"))
    // turn on all relays
    return ALL_RELAYS_ON;

  if (command == F("relays off"))
    // turn off all relays
    return ALL_RELAYS_OFF;
  
  if (command == F("enable timers"))
    // enables timers
    return ENABLE_TIMERS;

  if (command == F("disable timers"))
    // disables timers
    return DISABLE_TIMERS;

  if (command == F("delete all timers"))
    // deletes all timers
    return DELETE_ALL_TIMERS;

  if (command == F("default timers"))
    // set default timers
    return SET_DEFAULT_TIMERS;

  if (command.startsWith(F("water")))
    //
    // message format: "water on/off" (relay #0)
    //
    return command.endsWith(F("on"))? RELAY_ON : RELAY_OFF;    
    
  if (command.startsWith(F("relay"))){
    //
    // turn on/off water valve (relay "1")
    // message format: "relay 5 on/off"
    //
    Serial.println("[" + command.substring(6, 8) + "]"); // relay number: [1-8]
    int index = command.substring(6, 8).toInt() - 1; // relay index: [0-7]
    #if (DEBUG_MODE == true)
    Serial.print(F("relay index: "));
    Serial.println(index);
    #endif
    if (index < 0 || index >= RELAY_SIZE)
      // invalid relay number!
      return NOP;
    
    byte value = command.endsWith(F("on"))? 0xA0 : 0xB0;        
    if (index > 0) 
      value = value | (byte)index;
    
    return value;
  }

  unsigned int len = command.length();

  if (command.startsWith(F("enable timer: ")) && len > 14){
    //
    // enables timer by "id"
    // message format: "enable timer: 01"
    //
    param.empty = false;
    param.count = 1;
    #if (DEBUG_MODE == true)
    Serial.println(F("id: [") + command.substring(14, len) + F("]"));
    #endif
    param.data[0] = (byte)(command.substring(14, len).toInt()); // timer id [0-49]
    return ENABLE_TIMER;
  }

  if (command.startsWith(F("disable timer: ")) && len > 15){
    //
    // disables timer by "id"
    // message format: "disable timer: 01"
    //
    param.empty = false;
    param.count = 1;
    #if (DEBUG_MODE == true)
    Serial.println(F("id: [") + command.substring(15, len) + F("]"));
    #endif
    param.data[0] = (byte)(command.substring(15, len).toInt()); // timer id [0-49]
    return DISABLE_TIMER;
  }

  if (command.startsWith(F("delete timer: ")) && len > 14){
    //
    // deletes timer by "id"
    // message format: "delete timer: 01"
    //
    param.empty = false;
    param.count = 1;
    #if (DEBUG_MODE == true)
    Serial.println(F("id: [") + command.substring(14, len) + F("]"));
    #endif
    param.data[0] = (byte)(command.substring(14, len).toInt()); // timer id [0-49]
    return DELETE_TIMER;
  }

  if (command.startsWith(F("add timer: ")) && len == 25){
    //
    // creates new timer
    // message format: "add timer: relay hh mm hh mm"
    //
    param.empty = false;
    param.count = 5;
    #if (DEBUG_MODE == true)
    Serial.println(F("relay: [") + command.substring(11, 13) + F("]"));
    Serial.println(F("h: [") + command.substring(14, 16) + F("]"));
    Serial.println(F("m: [") + command.substring(17, 19) + F("]"));
    Serial.println(F("h: [") + command.substring(20, 22) + F("]"));
    Serial.println(F("m: [") + command.substring(23, 25) + F("]"));
    #endif
    
    param.data[0] = (byte)(command.substring(11, 13).toInt()); // relay number [1-8]
    param.data[1] = (byte)(command.substring(14, 16).toInt()); // turn on hour [0-23]
    param.data[2] = (byte)(command.substring(17, 19).toInt()); // turn on minute [0-59]
    param.data[3] = (byte)(command.substring(20, 22).toInt()); // turn off hour [0-23]
    param.data[4] = (byte)(command.substring(23, 25).toInt()); // turn off minute [0-59]
    return ADD_TIMER;
  }

  if (command.startsWith(F("update timer: ")) && len == 28){
    //
    // creates new timer
    // message format: "update timer: id hh mm hh mm"
    //
    param.empty = false;
    param.count = 5;
    #if (DEBUG_MODE == true)
    Serial.println(F("id: [") + command.substring(14, 16) + F("]")); 
    Serial.println(F("h: [") + command.substring(17, 19) + F("]"));
    Serial.println(F("m: [") + command.substring(20, 22) + F("]"));
    Serial.println(F("h: [") + command.substring(23, 25) + F("]"));
    Serial.println(F("m: [") + command.substring(26, 28) + F("]"));
    #endif

    param.data[0] = (byte)(command.substring(14, 16).toInt()); // timer id [0-49]
    param.data[1] = (byte)(command.substring(17, 19).toInt()); // turn on hour [0-23]
    param.data[2] = (byte)(command.substring(20, 22).toInt()); // turn on minute [0-59]
    param.data[3] = (byte)(command.substring(23, 25).toInt()); // turn off hour [0-23]
    param.data[4] = (byte)(command.substring(26, 28).toInt()); // turn off minute [0-59]
    return UPDATE_TIMER;
  }

  // not found: nothing to do!
  return NOP;
}


String toStr(unsigned int value){
  //
  // cast int to String (2 digits): e.g., "04"
  //
  if (value < 10)
    return F("0") + String(value, DEC);
  else
    return String(value, DEC);
}


String toStr(bool value){
  //
  // cast boolean to String: "on" / "off"
  //
  return value? String(F("\"on\"")) : String(F("\"off\"")) ;
}


