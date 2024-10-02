# GARDEN.INO 
## :house_with_garden: Garden Wifi with Home Assistant and NodeMCU

**Optimized for NodeMCU 1.0 (ESP-12E Module)** 

<center>

![logo](https://github.com/fortalbrz/gardenino/blob/main/img/logo.png?raw=true)

</center>

I made this project to make use of an old 8 channels relay module with a low cost NodeMCU module... :smiley: 
The goal is to control the garden watering, some decorative led lights, a couple of light flood, etc 
using [Home Assistant](https://home-assistant.io) with *MQTT* protocol (e.g., *mosquitto broker*).


 - **Video**: **[YouTube](https://youtu.be/8qxTY6HyU3Y)** [pt]

<center>

![figure1](https://github.com/fortalbrz/gardenino/blob/main/img/figure_01.png?raw=true)

![figure2](https://github.com/fortalbrz/gardenino/blob/main/img/figure_02.png?raw=true)

![figure3](https://github.com/fortalbrz/gardenino/blob/main/img/figure_03.png?raw=true)

![figure4](https://github.com/fortalbrz/gardenino/blob/main/img/figure_04.png?raw=true)

</center>


## Features:
 - works with Home Assistant to control garden watering and more 7 smart switches (MQTT), lights, *etc*
 - soil moisture sensor (avoids watering when the soil is wet - *optional, use [config flags](https://github.com/fortalbrz/gardenino/blob/main/README.md#configuration-flags)*)
 - watering watchdog (prevents watering for long time... :potable_water: - *optional, see [config flags](https://github.com/fortalbrz/gardenino/blob/main/README.md#configuration-flags)*)   


## MQTT broker

This project should communicate with a MQTT broker (e.g., *mosquitto broker*), ideally using **[home assistant](https://home-assistant.io)**

![mqtt diagram](https://github.com/fortalbrz/gardenino/blob/main/img/schema.png?raw=true)


## Source code:
 - **https://github.com/fortalbrz/gardenino**


## Materials:
 - 1 x NodeMCU (ESP 8266-12e) - [[25 BRL](https://produto.mercadolivre.com.br/MLB-1211973212-modulo-wifi-esp8266-nodemcu-esp-12e-_JM)]
 - 1 x relay module 5v 8-ch (optional: less that 8 channels can be used) - [[35 BRL](https://produto.mercadolivre.com.br/LB-1758954385-modulo-rele-rele-5v-8-canais-para-arduino-pic-raspberry-pi-_JM)]
 - 1 x solenoid valve 3/4" 110 v (normally closed) - [[25 BRL](https://produto.mercadolivre.com.br/MLB-1511610317-valvula-solenoide-simples-entrada-agua-com-suporte-127v-220v-_JM)]
 - 1 x soil moisture sensor (optional: avoids watering when the soil is wet) - [[9 BRL](https://www.a2robotics.com.br/sensor-de-umidade-do-solo-modulo-sonda-higrometro)]
 - 1 x power supply 5vdc (1A) - [[14 BRL](https://produto.mercadolivre.com.br/MLB-3445635491-fonte-alimentaco-5v-1a-bivolt-roteador-wireles-modem-d-link-_JM)]
 - 1 x led and resistor 10k ohms (optional, indicates "power on")
 - 1 x electrolytic capacitor 100 uF (optional)
 - flexible cab (22 agw)

*Others:*
 - 4 LED garden spike light (7w) - [[69 BRL](https://produto.mercadolivre.com.br/MLB-1987365811-kit-4-luminaria-espeto-jardim-lmpada-cob-led-7w-luz-verde-_JM)]
 - led flood light 50w - [[19 BRL](https://produto.mercadolivre.com.br/MLB-1703139744-refletor-holofote-led-50w-branco-frio-bivolt-6000k-_JM)]


## Circuit Wiring Instruction (step by step):


![project resources](https://github.com/fortalbrz/gardenino/blob/main/img/wiring_diagram.png?raw=true)

[[wiring diagram](https://www.circuito.io/app?components=513,13322,360216,442979)]:
  - NodeMCU (GND) -> power supply 5vdc (negative/Gnd)
  - NodeMCU (Vin) -> power supply 5vdc (positive/Vcc)
  - Relay 8 ch (VCC) -> power supply 5vdc (negative/Gnd)
  - Relay 8 ch (GND) -> power supply 5vdc (positive/Vcc)
  - Relay 8 ch (In 1) -> Arduino Nano (D1)
  - Relay 8 ch (In 2) -> Arduino Nano (D2)
  - Relay 8 ch (In 3) -> Arduino Nano (D5)
  - Relay 8 ch (In 4) -> Arduino Nano (D6)
  - Relay 8 ch (In 5) -> Arduino Nano (D7)
  - Relay 8 ch (In 6) -> Arduino Nano (D3)
  - Relay 8 ch (In 7) -> Arduino Nano (D0)
  - Relay 8 ch (In 8) -> Arduino Nano (D4)
  - Soil moisture sensor VIN (right) -> NodeMCU (3.3v)
  - Soil moisture sensor GND (center) -> power supply 5vdc (negative/Gnd)
  - Soil moisture sensor SIG/A0 (left) -> NodeMCU (A0)
  - Led terminal 1 (positive) -> +5 V power source (VCC) (optional, "power on led")
  - Led terminal 2 (negative/bevel) -> resistor 10k olhms "D" terminal 1 (optional, "power on led")
  - resistor 10k olhms "D" terminal 2 -> -5 V power source (GND) (optional, "power on led")
  - capacitor 100uF (positive) -> +5 V power source (VCC) (optional)
  - capacitor 100uF (negative/"minus sign") -> resistor 10k olhms "D" terminal 2 (optional)


## Flashing the code

**Drivers (CH340g)** for NodeMCU:
  - [CH340g USB/Serial driver](https://bit.ly/44WdzVF) (windows 11 compatible driver)  
  - driver install instructions ([pt](https://bit.ly/3ZqIqc0))

The ESP-01 module should be programed with the sketch with the [Arduino IDE](https://www.arduino.cc/en/software):
  - go to File > Preferences
  - on "Additional boards manager", set the value "http://arduino.esp8266.com/stable/package_esp8266com_index.json"
  - go to Tools > Board > Board Manager
  - search for "**ESP8266**"
  - install the ESP8266 Community package ("**esp8266**" by *ESP8266 Community*)//   
  - select board "**NodeMCU 1.0 (ESP-12E Module)**" and connected COM port (checks at Windows "device manager")
  - select Sketch > Upload

## Home Assistant Configuration

Adds the line on *configuration.yaml*: 


     mqtt: !include mqtt.yaml


And creates a file "*[mqtt.yaml](https://github.com/fortalbrz/gardenino/blob/main/mqtt.yaml)*" as follow:


    - sensor:
      #
      # Soil moisture (front garden)
      #
      name: "Garden Soil moisture"
      unique_id: front_garden_soil_moisture
      device_class: "moisture"
      unit_of_measurement: "%"
      suggested_display_precision: 0
      state_topic: "gardenino/state"
      value_template: "{{ value_json.moisture }}"
      json_attributes_topic: "gardenino/state"
      json_attributes_template: "{{ value_json | tojson }}"
      icon: mdi:water
      availability_mode: latest
      expire_after: 0
      qos: 0
      availability:
        - topic: "gardenino/available"
          payload_available: "online"
          payload_not_available: "offline"
    - sensor:
      #
      # Soil condition (front garden)
      #
      name: "Garden Soil Condition"
      unique_id: front_garden_soil_condition
      device_class: "enum"
      state_topic: "gardenino/state"
      value_template: "{{ value_json.soil }}"
      icon: mdi:water
      availability_mode: latest
      expire_after: 0
      qos: 0
      availability:
        - topic: "gardenino/available"
          payload_available: "online"
          payload_not_available: "offline"
    - switch:
      #
      # Front garden watering: on/off
      #
      name: "Garden Watering"
      unique_id: front_garden_watering
      state_topic: "gardenino/state"
      value_template: "{{ value_json.relay_1 }}"
      state_on: "on"
      state_off: "off"
      command_topic: "gardenino/cmd"
      payload_on: "relay 1 on"
      payload_off: "relay 1 off"
      availability:
        - topic: "gardenino/available"
          payload_available: "online"
          payload_not_available: "offline"
      availability_mode: latest
      enabled_by_default: true
      optimistic: false
      qos: 0
      retain: true
      icon: mdi:sprinkler
      device_class: "outlet"
    - switch:
      #
      # Front garden lights: on/off
      #
      name: "Garden Decorative Lights"
      unique_id: front_garden_small_lights
      state_topic: "gardenino/state"
      value_template: "{{ value_json.relay_2 }}"
      state_on: "on"
      state_off: "off"
      command_topic: "gardenino/cmd"
      payload_on: "relay 2 on"
      payload_off: "relay 2 off"
      availability:
        - topic: "gardenino/available"
          payload_available: "online"
          payload_not_available: "offline"
      availability_mode: latest
      enabled_by_default: true
      optimistic: false
      qos: 0
      retain: true
      icon: mdi:post-lamp
      device_class: "outlet"
    - switch:
      #
      # Front garden light flood: on/off
      #
      name: "Garden Light Flood 1"
      unique_id: front_garden_light_flood
      state_topic: "gardenino/state"
      value_template: "{{ value_json.relay_3 }}"
      state_on: "on"
      state_off: "off"
      command_topic: "gardenino/cmd"
      payload_on: "relay 3 on"
      payload_off: "relay 3 off"
      availability:
        - topic: "gardenino/available"
          payload_available: "online"
          payload_not_available: "offline"
      availability_mode: latest
      enabled_by_default: true
      optimistic: false
      qos: 0
      retain: true
      icon: mdi:light-flood-down
      device_class: "outlet"
    - switch:
      #
      # Front garden light flood (neighbor): on/off
      #
      name: "Garden Light Flood 2"
      unique_id: front_garden_light_flood_neighbor
      state_topic: "gardenino/state"
      value_template: "{{ value_json.relay_4 }}"
      state_on: "on"
      state_off: "off"
      command_topic: "gardenino/cmd"
      payload_on: "relay 4 on"
      payload_off: "relay 4 off"
      availability:
        - topic: "gardenino/available"
          payload_available: "online"
          payload_not_available: "offline"
      availability_mode: latest
      enabled_by_default: true
      optimistic: false
      qos: 0
      retain: true
      icon: mdi:light-flood-down
      device_class: "outlet"
    - switch:
      #
      # Enables the moisuture sensor to lock watering on front garden (wet soil)
      #
      name: "Enables moisture Sensor to Lock Watering"
      unique_id: front_garden_enables_moisture_sensor
      state_topic: "gardenino/state"
      value_template: "{{ value_json.sensor }}"
      state_on: "on"
      state_off: "off"
      command_topic: "gardenino/cmd"
      payload_on: "sensor on"
      payload_off: "sensor off"
      availability:
        - topic: "gardenino/available"
          payload_available: "online"
          payload_not_available: "offline"
      availability_mode: latest
      enabled_by_default: true
      optimistic: false
      qos: 0
      retain: true
      icon: mdi:sprinkler
      device_class: "outlet"
    - switch:
      #
      # Enables watering watchdog on front garden (5 min)
      #
      name: "Enables Watering Watchdog"
      unique_id: front_garden_enables_watering_watchdog
      state_topic: "gardenino/state"
      value_template: "{{ value_json.watchdog }}"
      state_on: "on"
      state_off: "off"
      command_topic: "gardenino/cmd"
      payload_on: "watchdog on"
      payload_off: "watchdog off"
      availability:
        - topic: "gardenino/available"
          payload_available: "online"
          payload_not_available: "offline"
      availability_mode: latest
      enabled_by_default: true
      optimistic: false
      qos: 0
      retain: true
      icon: mdi:sprinkler
      device_class: "outlet"




## MQTT topics:

   - **gardenino/available**: sensors availability [*"online"/"offline"*]

   - **gardenino/cmd**: pushes commands to NodeMCU [*home assistant -> gardenino*]:
     - "*watering*": watering for 5 min (turn on/off sonenoid valve, i.e. relay #1)
     - "*light on/off*": turns on/off the garden decorative led lights (i.e. relay #2)
     - "*light flood 1 on/off*": turn on/of the 1st light flood (i.e. relay #3)
     - "*light flood 2 on/off*": turn on/of the 2nd light flood (i.e. relay #4)
     - "*relays on/off*": turn on/off all relays [debug only]
     - "*watchdog on/off*": enables/disables the watering watchdog (5 minutes) [debug only]
     - "*sensor on/off*": enables/disables the soil humidity sensor to block the watering if the soil is too wet [debug only]
     - "*refresh*": update MQTT state [debug only]

   - **gardenino/state**: retrieves NodeMCU states as json [*gardenino -> home assistant*]


         {
            "relay_1": "off",    // relay 1 state: [on/off]
            "relay_2": "on",     // relay 2 state: [on/off]
            "relay_3": "off",    // relay 3 state: [on/off]
            "relay_4": "off",    // relay 4 state: [on/off] 
            "relay_5": "off",    // relay 5 state: [on/off]
            "relay_6": "off",    // relay 6 state: [on/off]
            "relay_7": "off",    // relay 7 state: [on/off]
            "relay_8": "off",    // relay 8 state: [on/off]
            "sensor": "off",     // soil mosture blocks watering enabled: [on/off]
            "watchdog": "on",    // watering watchdog enabled: [on/off]
            "moisture": 91,      // soil moisture, as percentage [0-100]
            "cond": 84,          // soil conductivity [0-1024]
            "soil": "wet"}       // soil state: [dry/ok/wet]
         } 



### Configuration flags
  

| macro                      | default | description                                                                                   |
|----------------------------|---------|-----------------------------------------------------------------------------------------------|
| WIFI_SSID                  |         | Wi-fi SSID                                                                                    |
| WIFI_PASSWORD              |         | Wi-fi password                                                                                |
| MQTT_BROKER_ADDRESS MQTT   |         | MQTT broker server ip address                                                                 |
| MQTT_BROKER_PORT           | 1883    | MQTT broker port                                                                              |
| MQTT_USERNAME              |         | MQTT broker username                                                                          |
| MQTT_PASSWORD              |         | MQTT broker password                                                                          |
| MQTT_DEVICE_ID             |         | MQTT session identifier (changes for more then one gardeino on the same MQTT broker)          |
| USE_WATERING_WATCHDOG      | true    | enables a watering time limit (watering watchdog), false otherwise                            |
| USE_MOISTURE_SENSOR        | true    | enables the moisture sensor, false otherwise (set false if the moisture sensor is not needed) | 
| DEBUG_MODE                 | false   | true to debug on serial monitor (debug), false otherwise                                      |
| WIRING_TEST_MODE           | false   | enables/disables a wiring test routine                                                        |


<hr>

*[Jorge Albuquerque](https://linkedin.com/in/jorgealbuquerque) (2024)*
