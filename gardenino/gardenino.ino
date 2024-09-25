//---------------------------------------------------------------------------------------------------------------------
//
//   GARDENINO - Garden Wifi with Home Assistant and NodeMCU
//   - Optimized for NodeMCU 1.0 (ESP-12E Module)
//
//   I made this project to make use of an old 8 chanels relay module with a low cost NodeMCU module... :) 
//   The goal is to control the garden watering, some decorative led lights, a couple of light flood, etc 
//   using [Home Assistant](https://home-assistant.io) with MQTT protocol (e.g. mosquitto broker).
//
//   - Features:
//      - works with Home Assistant to control garden watering and more 7 smart switches (MQTT), lights, etc
//      - alternativelly, works autonomus with up to 200 programable timers (using real time clock - optional, use config flags)
//      - soil moisture sensor to (avoids watering when the soil is wet - optional, use config flags)
//      - watering watchdog (prevents watering for long time... - optional, use config flags)   
//      - turn on/off watering with local push button (optional, use config flags)
//
//   This project should communicate with a MQTT broket (e.g., mosquitto broker), ideally using [home assistant](https://home-assistant.io)
//
//    ------------------         ------------------                ----------  
//   |  home assistant  |  <=>  | mosquitto broker |  <=[MQTT]=>  |  NodeMCU | 
//    ------------------         ------------------                ----------  
//                                   MQTT broker         wifi       gardenino   
//
//   Source code:
//   - https://github.com/fortalbrz/gardenino
//
//   Materials:
//   - NodeMCU (ESP 8266-12e) [25 BRL](https://produto.mercadolivre.com.br/MLB-1211973212-modulo-wifi-esp8266-nodemcu-esp-12e-_JM)
//   - relay module 5v 8-ch (optional: less that 8 channels can be used) [35 BRL](https://produto.mercadolivre.com.br/MLB-1758954385-modulo-rele-rele-5v-8-canais-para-arduino-pic-raspberry-pi-_JM)
//   - solenoid valve 3/4" 110 v (normaly closed) [25 BRL](https://produto.mercadolivre.com.br/MLB-1511610317-valvula-solenoide-simples-entrada-agua-com-suporte-127v-220v-_JM)
//   - soil moisture sensor (optional: avoids watering when the soil is wet) [9 BRL](https://www.a2robotics.com.br/sensor-de-umidade-do-solo-modulo-sonda-higrometro)
//   - power supply 5vdc (1A) [14 BRL](https://produto.mercadolivre.com.br/MLB-3445635491-fonte-alimentaco-5v-1a-bivolt-roteador-wireles-modem-d-link-_JM)
//
//   Others:
//   - kit 4 LED garden spike light (7w) [69 BRL](https://produto.mercadolivre.com.br/MLB-1987365811-kit-4-luminaria-espeto-jardim-lmpada-cob-led-7w-luz-verde-_JM)
//   - led flood light 50w [19 BRL](https://produto.mercadolivre.com.br/MLB-1703139744-refletor-holofote-led-50w-branco-frio-bivolt-6000k-_JM)
//
//   Circuit Wiring Instruction:
//      - NodeMCU (GND) --> power supply 5vdc (negative/Gnd)
//      - NodeMCU (Vin) --> power supply 5vdc (positive/Vcc)
//      - Relay 8 ch (VCC) --> power supply 5vdc (negative/Gnd)
//      - Relay 8 ch (GND) --> power supply 5vdc (positive/Vcc)
//      - Relay 8 ch (In 1) --> Arduino Nano (D1)
//      - Relay 8 ch (In 2) --> Arduino Nano (D2)
//      - Relay 8 ch (In 3) --> Arduino Nano (D5)
//      - Relay 8 ch (In 4) --> Arduino Nano (D6)
//      - Relay 8 ch (In 5) --> Arduino Nano (D7)
//      - Relay 8 ch (In 6) --> Arduino Nano (D3)
//      - Relay 8 ch (In 7) --> Arduino Nano (D0)
//      - Relay 8 ch (In 8) --> Arduino Nano (D4)
//      - Soil moisture sensor VIN (right) --> NodeMCU (3.3v)
//      - Soil moisture sensor GND (center) --> power supply 5vdc (negative/Gnd)
//      - Soil moisture sensor SIG/A0 (left) --> NodeMCU (A0)
//
//   Flashing the code:
//
//   Drivers (CH340g) for NodeMCU:
//    - CH340g USB/Serial driver (windows 11 compatible driver): https://bit.ly/44WdzVF 
//    - driver install instructions (pt): https://bit.ly/3ZqIqc0
//   
//   The ESP-01 module should be programed with the sketch with the [Arduino IDE](https://www.arduino.cc/en/software) 
//    - go to File > Preferences
//    - on "Additonal boards manager", set the value "http://arduino.esp8266.com/stable/package_esp8266com_index.json"
//    - go to Tools > Board > Board Manager
//    - search for “ESP8266”
//    - install the ESP8266 Community package ("esp8266" by ESP8266 Community)//   
//    - select board "NodeMCU 1.0 (ESP-12E Module)" and coonected COM port (checks the Windows "device manager")
//
//   MQTT topics:
//    - gardenino/available: sensors availability ["online"/"offline"]
//    - gardenino/cmd: pushes commands to NodeMCU [home assistant -> gardenino]:
//         "watering": watering for 5 min (turn on/off sonenoid valve, i.e. relay #1)
//         "light on/off": turns on/off the garden decorative led lights (i.e. relay #2)
//         "light on/off": turns on/off the garden decorative led lights (i.e. relay #2)
//         "light flood 1 on/off": turn on/of the 1st light flood (i.e. relay #3)
//         "light flood 2 on/off": turn on/of the 2nd light flood (i.e. relay #4)
//         "relays on/off": turn on/off all relays [debug only]
//         "watchdog on/off": enables/disables the watering watchdog (5 minutes) [debug only]
//         "sensor on/off": enables/disables the soil humidity sensor to block the watering if the soil is too wet [debug only]
//         "refresh": update MQTT state [debug only]
//    - gardenino/state: retrieves NodeMCU states as json [gardenino -> home assistant]
//         {
//            "relay_1": "off",    // relay 1 state: [on/off]
//            "relay_2": "on",     // relay 2 state: [on/off]
//            "relay_3": "off",    // relay 3 state: [on/off]
//            "relay_4": "off",    // relay 4 state: [on/off] 
//            "relay_5": "off",    // relay 5 state: [on/off]
//            "relay_6": "off",    // relay 6 state: [on/off]
//            "relay_7": "off",    // relay 7 state: [on/off]
//            "relay_8": "off",    // relay 8 state: [on/off]
//            "sensor": "off",     // soil mosture blocks watering enabled: [on/off]
//            "watchdog": "on",    // watering watchdog enabled: [on/off]
//            "moisture": 91,      // soil moisture, as percentage [0-100]
//            "cond": 84,          // soil conductivity [0-1024]
//            "soil": "wet"}       // soil state: [dry/ok/wet]
//         } 
//
// Configuration flags:
//   - WIFI_SSID: Wi-fi SSID
//   - WIFI_PASSWORD: Wi-fi password
//   - MQTT_BROKER_ADDRESS MQTT: broker server ip address
//   - MQTT_BROKER_PORT: MQTT broker port (default: 1883)
//   - MQTT_USERNAME: mqtt broker username
//   - MQTT_PASSWORD: mqtt broker password
//   - MQTT_DEVICE_ID: MQTT session identifier (changes for more then one gardeino on the same MQTT broker)
//
// Option flags:
//   - USE_WATERING_WATCHDOG: enables a watering time limit (watering watchdog), false otherwise (default: true)
//   - USE_MOISTURE_SENSOR: enables the moisture sensor, false otherwise - set false if the moisture sensor is not needed (default: true)
//   - USE_BUILDIN_BLINKING_LED: set true to use disable Relay #8 and use NodeMCU D4 as blinking led, false otherwise (default: false)
//
// Debug flags:
//   - DEBUG_MODE: enables/disables serial monitor debugging messages
//   - WIRING_TEST_MODE: enables/disables a wiring test mode
//
//
//   Jorge Albuquerque (2024) - https://linkedin.com/in/jorgealbuquerque
//
//------------------------------------------------------------------------------------------------------------------
#define DEBUG_MODE false                  // enables/disables serial debugging messages
#define WIRING_TEST_MODE false            // enables/disables testing mode
//------------------------------------------------------------------------------------------------------------------
#define USE_WATERING_WATCHDOG true        // true to set a watering time limit, false otherwise (default: true)
#define USE_MOISTURE_SENSOR true          // true to use moisture sensor, false otherwise - dont need the moisture sensor (default: true)
#define USE_BUILDIN_BLINKING_LED false    // true to use disable Relay #8 and use NodeMCU D4 as blinking led, false otherwise
//------------------------------------------------------------------------------------------------------------------
//
// Configuration flags (enables or disables features in order to "skip" unwanted hardware)
//
//------------------------------------------------------------------------------------------------------------------
// Wi-fi setup
#define WIFI_SSID "wifi ssid"               // Wi-fi SSID
#define WIFI_PASSWORD "wifi password"       // Wi-fi password
// MQTT setup
#define MQTT_BROKER_ADDRESS "192.168.68.10"  // MQTT broker server ip
#define MQTT_BROKER_PORT 1883                // MQTT broker port
#define MQTT_USERNAME "mqtt-user"            // can be omitted if not needed
#define MQTT_PASSWORD "mqtt-password"        // can be omitted if not needed
// MQTT topics
#define MQTT_COMMAND_TOPIC "gardenino/cmd"             // MQTT topic for send door commands (e.g., open from door)
#define MQTT_STATUS_TOPIC "gardenino/state"           // MQTT topic for doorbell status
#define MQTT_AVAILABILITY_TOPIC "gardenino/available"  // MQTT topic for availability notification (home assistant "unavailable" state)
#define MQTT_DEVICE_ID "gardenino_12fmo43iowerwe2"     // MQTT session identifier
// others
#define SOIL_WET 450                                   // defines max soil conductivity value we consider soil 'wet' (valid: 0 to 1023, default: 450)
#define SOIL_DRY 750                                   // defines min soil conductivity value we consider soil 'dry' (valid: 0 to 1023, default: 750)
#define WATERING_WATCHDOG_INTERVAL 300000              // watering time limit (milisseconds) (default: 300000 = 5 min) 
#define MOISTURE_SENSOR_INTERVAL 30000                 // watering time limit (milisseconds) (default: 300000 = 5 min) 
#define MQTT_STATUS_UPDATE_TIME 120000                 // time for send and status update (default: 2 min)
#define MQTT_AVAILABILITY_TIME 60000                   // elapsed time to send MQTT availability, in miliseconds (default: 1 min)
#define SERIAL_BAUDRATE 9600                           // serial monitor baud rate (only for debuging)
#define EEPROM_ADDRESS 0
#define RELAY_SIZE 8
//
// pins definitions (NodeMCU)
//
#define MOISTURE_SENSOR_PIN A0              // A0 - moisture sensor analog input
#define RELAY_01_PIN D1                     // D1: pull-up (high) - Relay #1: solenoid valve 110v
#define RELAY_02_PIN D2                     // D2: pull-up (high) - Relay #2: garden lights 
#define RELAY_03_PIN D5                     // D5: pull-up (high) - relay #3: light flood garden
#define RELAY_04_PIN D6                     // D6: pull-up (high) - relay #4: light flood neighbor
#define RELAY_05_PIN D7                     // D7: pull-up (high) - relay #5: TBD
#define RELAY_06_PIN D3                     // D3 pull-up (high) (remark: boot fails on low)  - Relay #6: TBD
#define RELAY_07_PIN D0                     // D0: pull-down (low) - relay #7: TBD
#define RELAY_08_PIN D4                     // D4: pull-up (high) - [connected to build-in LED] - relay #8: TBD
#define WATERING_RELAY_INDEX 0
//
// protocol commands
//
#define MQTT_COMMAND_RELAY_01_ON "relay 1 on"
#define MQTT_COMMAND_RELAY_01_OFF "relay 1 off"
#define MQTT_COMMAND_RELAY_02_ON "relay 2 on"
#define MQTT_COMMAND_RELAY_02_OFF "relay 2 off"
#define MQTT_COMMAND_RELAY_03_ON "relay 3 on"
#define MQTT_COMMAND_RELAY_03_OFF "relay 3 off"
#define MQTT_COMMAND_RELAY_04_ON "relay 4 on"
#define MQTT_COMMAND_RELAY_04_OFF "relay 4 off"
#define MQTT_COMMAND_RELAY_05_ON "relay 5 on"
#define MQTT_COMMAND_RELAY_05_OFF "relay 5 off"
#define MQTT_COMMAND_RELAY_06_ON "relay 6 on"
#define MQTT_COMMAND_RELAY_06_OFF "relay 6 off"
#define MQTT_COMMAND_RELAY_07_ON "relay 7 on"
#define MQTT_COMMAND_RELAY_07_OFF "relay 7 off"
#define MQTT_COMMAND_RELAY_08_ON "relay 8 on"
#define MQTT_COMMAND_RELAY_08_OFF "relay 8 off"
#define MQTT_COMMAND_RELAYS_ON "relays on"
#define MQTT_COMMAND_RELAYS_OFF "relays off"
#define MQTT_COMMAND_ENABLE_MOISTURE_SENSOR "sensor on"
#define MQTT_COMMAND_DISABLE_MOISTURE_SENSOR "sensor off"
#define MQTT_COMMAND_ENABLE_WATERING_WATCHDOG "watchdog on"
#define MQTT_COMMAND_DISABLE_WATERING_WATCHDOG "watchdog off"
// command "alias"
#define MQTT_COMMAND_WATERING "watering"
#define MQTT_COMMAND_GARDEN_LIGHT_ON "light on" 
#define MQTT_COMMAND_GARDEN_LIGHT_OFF "light off"
#define MQTT_COMMAND_LIGHT_FLOOD_ON "light flood 1 on"
#define MQTT_COMMAND_LIGHT_FLOOD_OFF "light flood 1 off"
#define MQTT_COMMAND_LIGHT_FLOOD_NEIGHBOR_ON "light flood 2 on"
#define MQTT_COMMAND_LIGHT_FLOOD_NEIGHBOR_OFF "light flood 2 off"
#define MQTT_COMMAND_DEBUG_REFRESH "refresh"
//
// librarys (see doc above)
//
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>

WiFiClient espClient;
PubSubClient MQTT(espClient);

//
// internal states (globals)
//
bool _blink = false;
bool _useMoistureSensor = true;
bool _useWateringWatchDog = true;
byte _soilMoistureLevel = 0;
unsigned int _soilConductivity = 1023;
unsigned long _wateringStartTime = 0;
unsigned long _lastMoistureUpdateTime = 0;
unsigned long _lastAvailabilityTime = 0;
unsigned long _lastStatusUpdateTime = 0;

byte RELAY_PINS[] = {RELAY_01_PIN, RELAY_02_PIN, RELAY_03_PIN, RELAY_04_PIN, 
  RELAY_05_PIN, RELAY_06_PIN, RELAY_07_PIN, RELAY_08_PIN};
bool RELAY_STATES[RELAY_SIZE];

//------------------------------------------------------------------------------------------------------------------
//
// prototypes
//
//------------------------------------------------------------------------------------------------------------------
void connectWiFi();
void connectMQTT();
void onMessage(char* topic, byte* payload, unsigned int length);
void updateStates();
unsigned int readMoistureSensor(const bool& force = false);
void setRelay(const unsigned int& relayPin, bool state);
void loadConfig();
void saveConfig();
void wiringTest();
String toRelayStr(const unsigned int& index);
String toStr(const char* label, const bool& state);


//------------------------------------------------------------------------------------------------------------------
//
// functions
//
//------------------------------------------------------------------------------------------------------------------

void setup() {
  //
  // intialization
  // 
  #if (DEBUG_MODE == true)
    //Serial.begin(SERIAL_BAUDRATE);
    Serial.begin(115200);
    Serial.println(F("-- Debug Mode --"));
  #endif

  // initializes relays
  delay(60);
  for(unsigned int i = 0; i < RELAY_SIZE; i++) {    
    pinMode(RELAY_PINS[i], OUTPUT);
    // relays off (security)
    digitalWrite(RELAY_PINS[i], HIGH);
  }
  
  #if (DEBUG_MODE == true)
    Serial.print(F("Connecting to wifi network: "));
    Serial.println(WIFI_SSID);
  #endif
  delay(50);
  
  // Wi-fi connection
  connectWiFi();

  // MQTT broker setup
  MQTT.setServer(MQTT_BROKER_ADDRESS, MQTT_BROKER_PORT);  
  #if (WIRING_TEST_MODE == false)
    
    MQTT.setCallback(onMessage);
    
    loadConfig();
    delay(250);
    updateStates();
  #endif  
}


void loop() {
  //
  // main loop
  //  
  #if (USE_BUILDIN_BLINKING_LED == true)
    digitalWrite(BUILTIN_LED, (_blink ? LOW : HIGH));
    _blink = !_blink;
  #endif

  #if (WIRING_TEST_MODE == true)
    // wiring test routine
    wiringTest();
    delay(2000);
    return;
  #endif
  
  //
  // keeps MQTT connection alive
  //
  if (!MQTT.connected())
    connectMQTT();
  
  // sends MQTT "availability" message
  if ((millis() - _lastAvailabilityTime) > MQTT_AVAILABILITY_TIME) {
    MQTT.publish(MQTT_AVAILABILITY_TOPIC, "online");
    _lastAvailabilityTime = millis();
  }

  //
  // watering watchdog
  //
  if (USE_WATERING_WATCHDOG && _useWateringWatchDog    
    && RELAY_STATES[WATERING_RELAY_INDEX] 
    && ((millis() - _wateringStartTime) > WATERING_WATCHDOG_INTERVAL)){
      setRelay(WATERING_RELAY_INDEX, false);    
  }  

  // status update (with max timespan for 5 minutes)
  if ((millis() - _lastStatusUpdateTime) > MQTT_STATUS_UPDATE_TIME) {        
    updateStates();    
  }

  // MQTT loop
  MQTT.loop();

  // waits
  delay(1000);    
}


//--------------------------------------------------------------------------------------------------
//
// MQTT
//
//--------------------------------------------------------------------------------------------------
void onMessage(char* topic, byte* payload, unsigned int length) {
  //
  // On MQTT message
  //
  char msg[length + 1];
  memcpy(msg, payload, length);
  msg[length] = '\0';  // NULL;

  #if (DEBUG_MODE == true)
    Serial.print(F(" - mqtt command: ["));
    Serial.print(msg);
    Serial.println(F("]"));
  #endif

  // decode Home Assistant command (MQTT)
  if (strcmp(msg, MQTT_COMMAND_RELAY_01_ON) == 0 || strcmp(msg, MQTT_COMMAND_WATERING) == 0) {
    // relay #1 on: watering valve open
    setRelay(0, true);
  } else if (strcmp(msg, MQTT_COMMAND_RELAY_01_OFF) == 0) {
    // relay #1 off: watering valve closed
    setRelay(0, false);
  } else if (strcmp(msg, MQTT_COMMAND_RELAY_02_ON) == 0 || strcmp(msg, MQTT_COMMAND_GARDEN_LIGHT_ON) == 0) {
    // relay #2 on: garden lights on
    setRelay(1, true);
  } else if (strcmp(msg, MQTT_COMMAND_RELAY_02_OFF) == 0 || strcmp(msg, MQTT_COMMAND_GARDEN_LIGHT_OFF) == 0) {
    // relay #2 off: garden lights off
    setRelay(1, false);
  } else if (strcmp(msg, MQTT_COMMAND_RELAY_03_ON) == 0 || strcmp(msg, MQTT_COMMAND_LIGHT_FLOOD_ON) == 0) {
    // relay #3 on: light flood 1 on
    setRelay(2, true);
  } else if (strcmp(msg, MQTT_COMMAND_RELAY_03_OFF) == 0 || strcmp(msg, MQTT_COMMAND_LIGHT_FLOOD_OFF) == 0) {
    // relay #3 off: light flood 1 off
    setRelay(2, false);
  } else if (strcmp(msg, MQTT_COMMAND_RELAY_04_ON) == 0 || strcmp(msg, MQTT_COMMAND_LIGHT_FLOOD_NEIGHBOR_ON) == 0) {
    // relay #4 on: light flood 2 (neighbor) on
    setRelay(3, true);
  } else if (strcmp(msg, MQTT_COMMAND_RELAY_04_OFF) == 0 || strcmp(msg, MQTT_COMMAND_LIGHT_FLOOD_NEIGHBOR_OFF) == 0) {
    // relay #4 off: light flood 2 (neighbor) off
    setRelay(3, false);
  } else if (strcmp(msg, MQTT_COMMAND_RELAY_05_ON) == 0) {
    // relay #5 on
    setRelay(4, true);
  } else if (strcmp(msg, MQTT_COMMAND_RELAY_05_OFF) == 0) {
    // relay #5 off
    setRelay(4, false);
  } else if (strcmp(msg, MQTT_COMMAND_RELAY_06_ON) == 0) {
    // relay #6 on
    setRelay(5, true);
  } else if (strcmp(msg, MQTT_COMMAND_RELAY_06_OFF) == 0) {
    // relay #6 off
    setRelay(5, false);  
  } else if (strcmp(msg, MQTT_COMMAND_RELAY_07_ON) == 0) {
    // relay #7 on
    setRelay(6, true);
  } else if (strcmp(msg, MQTT_COMMAND_RELAY_07_OFF) == 0) {
    // relay #7 off
    setRelay(6, false);
  } else if (strcmp(msg, MQTT_COMMAND_RELAY_08_ON) == 0) {
    // relay #8 on
    setRelay(7, true);
  } else if (strcmp(msg, MQTT_COMMAND_RELAY_08_OFF) == 0) {
    // relay #8 off
    setRelay(7, false);
  } else if (strcmp(msg, MQTT_COMMAND_DEBUG_REFRESH) == 0) {
    // refresh moisture sensor data and update states
    readMoistureSensor(true);
    updateStates();
  } else if (strcmp(msg, MQTT_COMMAND_ENABLE_WATERING_WATCHDOG) == 0) {
    // enables watering watchdog
    _useWateringWatchDog = true;
    saveConfig();
    updateStates();
  } else if (strcmp(msg, MQTT_COMMAND_DISABLE_WATERING_WATCHDOG) == 0) {
    // disables watering watchdog
    _useWateringWatchDog = false;
    saveConfig();
    updateStates();
  } else if (strcmp(msg, MQTT_COMMAND_ENABLE_MOISTURE_SENSOR) == 0) {
    // enables moisture sensor to avoid watering (too wet)
    _useMoistureSensor = true;
    saveConfig();
    readMoistureSensor(true);
    updateStates();
  } else if (strcmp(msg, MQTT_COMMAND_DISABLE_MOISTURE_SENSOR) == 0) {
    // disables moisture sensor to avoid watering (too wet)
    _useMoistureSensor = false;
    saveConfig();
    updateStates();
  } else if (strcmp(msg, MQTT_COMMAND_RELAYS_ON) == 0) {
    // turn on all relays
    for(unsigned int i = 0; i < RELAY_SIZE; i++)     
      setRelay(i, true);  
    updateStates();
  } else if (strcmp(msg, MQTT_COMMAND_RELAYS_OFF) == 0) {
    // turn off all relays
    for(unsigned int i = 0; i < RELAY_SIZE; i++)     
      setRelay(i, false);  
    updateStates();
  }  
}

void updateStates() {
  //
  // MQTT publish states update
  //
  if (MQTT.connected()) {
    
    readMoistureSensor();

    String json = String(F("{"));
    // relay states   
    for(unsigned int i = 0; i < RELAY_SIZE; i++) {    
      json += toRelayStr(i);
    }

    // moisute sensor enabled: on/off
    json += toStr("sensor" , _useMoistureSensor);
    
    // watering watchdog enabled: on/off
    json += toStr("watchdog" , _useWateringWatchDog);
    
    // soil moisture level (as percentage): 0% dry - 100% wet
    json += String(F("\"moisture\": "));
    json += String(_soilMoistureLevel, DEC);
    
    // soil conductivity: [0, 1023]
    json += String(F(", \"cond\": "));
    json += String(_soilConductivity);
    
    // soil state: [wet, ok, dry]
    json += String(F(", \"soil\": "));
    if (_soilConductivity < SOIL_WET)
       json += String(F("\"wet\"}"));
    else if (_soilConductivity > SOIL_DRY)
       json += String(F("\"dry\"}"));
    else
       json += String(F("\"ok\"}"));
        
    unsigned int n = json.length() + 1;
    char message[n];
    json.toCharArray(message, n);

    #if (DEBUG_MODE == true)
      Serial.print(F("mqtt update: "));
      Serial.println(message);
    #endif
    
    MQTT.publish(MQTT_STATUS_TOPIC, message);
    _lastStatusUpdateTime = millis();    
  }
}

void connectMQTT() {
  //
  // Connects to MQTT broker
  //
  while (!MQTT.connected()) {
  #if (DEBUG_MODE == true)
    Serial.print(F("connecting to MQTT broker: "));
    Serial.println(MQTT_BROKER_ADDRESS);
  #endif

    // retry until connection
    connectWiFi();

    if (MQTT.connect(MQTT_DEVICE_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
      #if (DEBUG_MODE == true)
        Serial.println(F("MQTT broker connected!"));
      #endif
      // subscribe to command topic on connection
      MQTT.subscribe(MQTT_COMMAND_TOPIC);
      delay(50);
      MQTT.publish(MQTT_AVAILABILITY_TOPIC, "online");
      delay(50);
      updateStates();
      delay(50);
    } else {
      #if (DEBUG_MODE == true)
        Serial.println(F("Fail connecting to MQTT broker (retry in 2 secs)."));
      #endif
      delay(2000);
    }
  }
}

void connectWiFi() {
  //
  // connects to WiFi
  //
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // retry until connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #if (DEBUG_MODE == true)
      Serial.print(F("."));
    #endif
  }

  #if (DEBUG_MODE == true)
    Serial.println();
    Serial.print(F("WiFi connected: "));
    Serial.println(WIFI_SSID);
    Serial.print(F(" - IP address: "));
    Serial.println(WiFi.localIP());
  #endif
}

//--------------------------------------------------------------------------------------------------

void setRelay(const unsigned int& index, bool state) {
  //
  // sets the relay states (on/off)
  //
  // parameters:
  //     index          relay index: [0, 7]
  //     state          true for turn on, false for turn off
  //
  if (RELAY_STATES[index] == state)
    return;

  #if (USE_BUILDIN_BLINKING_LED == true)
    if (index == RELAY_SIZE - 1)
      // do not use last relay (if allocated to buildin led - D4)
      return;
  #endif
  
  delay(50);  

  if (index == WATERING_RELAY_INDEX && state) {
    // before turning on watering relay
    #if (USE_MOISTURE_SENSOR == true)
      if (_useMoistureSensor && readMoistureSensor(true) < SOIL_WET) 
        // soil is too wet: do nothing! (turn relay off)
        state = false;
      else
        // keeps going on: update last watering start time 
        // (required for the watering watchdog)
        _wateringStartTime = millis();
    #else
      // keeps going on: update last watering start time 
        // (required for the watering watchdog)
      _wateringStartTime = millis();
    #endif    
  }

  // relays are active LOW (on = LOW / off = HIGH)
  digitalWrite(RELAY_PINS[index], (state ? LOW : HIGH));

  // keep states on cache
  RELAY_STATES[index] = state;

  // update states (MQTT)
  updateStates();
}


unsigned int readMoistureSensor(const bool& force) {
  //
  // reads the soil conductivity sensor as:
  //
  //  [0-450]: dry, [450-750]: ok, [750-1024]: wet    
  //
  // remark: this sensor evaluates the soil conductivity (in range [0-1024]), therefore:
  // - lower value = WET soil
  // - higher value  = DRY soil
  //
  #if (USE_MOISTURE_SENSOR == true)
  
    if (!force && (millis() - _lastMoistureUpdateTime) < MOISTURE_SENSOR_INTERVAL) {
      return _soilConductivity;  
    }  

    delay(10);    
    
    // read value: [0-1024]
    int value = analogRead(MOISTURE_SENSOR_PIN);	
    // cap & floor
    if (value < 0) value = 0;
    if (value > 1024) value = 1024;  
    
    // Soil conductivity: 
    //   0    = totally wet    
    //   450  = max soil conductivity value we consider soil 'wet'  (SOIL_WET) 
    //   750  = min soil conductivity value we consider soil 'dry' (SOIL_DRY)
    //   1023 = totally dry
    //
    // thus: [0-450]: dry, [450-750]: ok, [750-1024]: wet    
    _soilConductivity = value;
    //
    // normalize as "misture level percentage", as byte: [0 - 100]
    // thus:
    //   [0-44]: wet
    //   [44-73]: ok
    //   [73-100]: dry    
    _soilMoistureLevel = (byte)(100 - ceil((float)value/10.24));

    delay(10);
    
  #else
    // dummy level
    _soilConductivity = SOIL_DRY;
    _soilMoistureLevel = 0x00;    
  #endif

  _lastMoistureUpdateTime = millis();
  return _soilConductivity;
}

//--------------------------------------------------------------------------------------------------

void loadConfig() {
  //
  // loads configurations from EEPROM
  //
  byte states = EEPROM.read(EEPROM_ADDRESS);
  bool crc = (bitRead(states, 0) == 1);
  if (crc) {
    _useMoistureSensor = (bitRead(states, 1) == 1);
    _useWateringWatchDog = (bitRead(states, 2) == 1);
  } else {
    _useMoistureSensor = true;
    _useWateringWatchDog = true;
    saveConfig();
    delay(50);
  }

  #if (DEBUG_MODE == true)
    Serial.println(F(" - configuration loaded: "));
    Serial.println(toStr("sensor", _useMoistureSensor));
    Serial.println(toStr("watchdog", _useWateringWatchDog));
  #endif
}

void saveConfig() {
  //
  // saves configurations to EEPROM
  //
  byte states = 0x00;
  // "crc"
  bitSet(states, 0);
  // enables moisture sensor
  if (_useMoistureSensor)
    bitSet(states, 1);
  // enables watering watchdog
  if (_useWateringWatchDog)
    bitSet(states, 2);

  EEPROM.write(EEPROM_ADDRESS, states);  

  #if (DEBUG_MODE == true)
    Serial.print(F(" - configuration saved: "));
    Serial.println(states, DEC);
  #endif
}

//--------------------------------------------------------------------------------------------------

void wiringTest() {
  //
  // Testing routine
  //
  #if (TESTING_MODE == true)

    bool testing = true;

    while (testing) {
      Serial.println(F("TESTING MODE"));
      Serial.println(F("1) Water Moisture Sensor"));    
      Serial.println(F("2) Relays"));
      Serial.println(F("3) Exit"));
      Serial.println(F("Select one option [1-3]:"));
      Serial.println();
      while (Serial.available() < 2) {
        delay(300);
      }

      // can be 0 if read error
      int input = Serial.parseInt();

      Serial.print(F("option: "));
      Serial.println(input);

      switch (input) {
        case 1:
          // soil moisture level sensor testing
          while (Serial.available() > 0) {
            Serial.read();
          }        
          
          while (Serial.available() == 0) {
            readMoistureSensor(true);
            Serial.print(F("Soil moisture: "));
            Serial.print(_soilMoistureLevel, DEC);
            Serial.print(F("% ["));
            Serial.print(_soilConductivity, DEC);
            Serial.println(F("]"));
            delay(1000);
          }

          Serial.read();
          break;

        case 2:
          // relays testing
          for(unsigned int i = 0; i < RELAY_SIZE; i++) {    
            setRelay(i, true);
            delay(1500);
            setRelay(i, false);
            delay(1500);
            setRelay(i, true);
            delay(1500);
            setRelay(i, false);
            delay(2500);
          }
            
        case 3:
          testing = false;
          break;

        defaut:
          Serial.print(F("Invalid option: [1-3]"));
          break; 
      }           
    }
  #endif
}

//--------------------------------------------------------------------------------------------------

String toRelayStr(const unsigned int& index) {
  //
  // writes a string line with the format:
  //     "relay_1": "on/off",
  //
  // parameters:
  //     index: relay index [0-7]
  //
  String text = String(F("\"relay_[IDX]\": \"[VAL]\", "));
  text.replace(F("[IDX]"), String(index + 1, DEC));
  text.replace(F("[VAL]"), (RELAY_STATES[index] ? F("on") : F("off")));
  return text;
}

String toStr(const char* label, const bool& state) {
  //
  // writes a string line with the format:
  //     "label": "on/off",
  //
  String text = String(F("\"[LABEL]\": \"[VALUE]\", "));
  text.replace(F("[LABEL]"), label);
  text.replace(F("[VALUE]"), (state ? F("on") : F("off")));
  return text;
}

//--------------------------------------------------------------------------------------------------