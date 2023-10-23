# fiat-600-smart

## Components:

- Arduino Mega 2560
- [ESP32](https://a.aliexpress.com/_EukP5mF), for Wi-Fi and Bluetooth connectivity 
- [MCP2515 CAN Module](https://a.aliexpress.com/_EvRh7Bd), in order to read and send in the CAN bus of the car 
- A bunch of [LM2596](https://a.aliexpress.com/_EQ7uZWT) DC-DC Regulator, for lower voltage power supply (5V and 3.3V) 
- [OBD2](https://a.aliexpress.com/_EyTDDjd) breakout cable
- 2 channels [relay module](https://a.aliexpress.com/_EJ4ra7p)
- A lot of Dupont prototyping cables

#### Buying soon:

- GSM Module (https://a.aliexpress.com/_EwLYeAP)
- GPS Module (https://a.aliexpress.com/_ExqmfET)
- Batteries for the GPS and GSM modules

#### Buying lately:

- Rolling code compatible remote (https://a.aliexpress.com/_EGZrL0X)
- If it's possible to use it in some ways, an NFC reader
- https://www.ricambi-smc.it/ricambi-auto/prodotti/batteria-auto-fiamm-7905140-12v-44ah-390a/420237?gclid=CjwKCAjw8ZKmBhArEiwAspcJ7lWkkx5FRfzFPQXfDzFnIXq4pmC18679WCiFq-hGUBT20aIF_oPuIxoCKEkQAvD_BwE

----

## Programming

First of all, adapt the [MCP2515 library](https://github.com/autowp/arduino-mcp2515) to the Mega. I've used the one provided by [adamtheone](https://github.com/adamtheone/canDrive), so in the MCP2515.h file we have to change `MCP2515_DEFAULT_CS_PIN` from 10 to 53

<img width="1173" alt="Screenshot 2023-08-10 at 11 47 44" src="https://github.com/sh4tteredd/fiat-600-smart/assets/55893559/e1595817-e60f-4ea5-93a7-51470de354b7">

so the new connections are:

| MEGA | CAN module |
| ---- | ---------- |
| 2    | INT        |
| 52   | SCK        |
| 51   | SI         |
| 50   | SO         |
| 53   | CS         |

N.B. if you want to use an Arduino Uno instead of the Mega, you don't need to modify the library and you have to use [the classic wiring](https://europe1.discourse-cdn.com/arduino/original/4X/c/a/3/ca3f21ea49f6ca4242c1168779d26aa42d888ba0.png)

--------

### Acquisto novembre

- GPS

- GSM

- radio

- schermo CARPLAY

- schermino contagiri

- cavetto schermino contagiri

- caricatore USB
