# ThermoLight - The IoT Thermometer & Display

## Project description
ThermoLight is a thermometer look-alike display that can show up to four values using NeoPixels & the ESP8266.

A more detailed description can be found on [hackster.io](https://www.hackster.io/har-bra/thermolight-the-iot-thermometer-display-c543e6).

## Folder contents
* Schematic and PCB for the underlying electronics, created in KiCAD
  * Some custom components are included in a custom KiCAD library in /mylibrary

## Licenses
* Schematic, PCB and enclosure design are licensed with [CC-BY-SA-4.0](https://creativecommons.org/licenses/by-sa/4.0/)

## Hints
* It seems inserting the diffusor slightly warps/bends the main enclosure part. This can lead to a slight bulging of the back lid. I removed some material towards the top end of the lid where the M2 screw goes to reduce the warping of the back lid.

## BOM
| Ref | Qnty | Value | Part | Footprint | Comment |
| --- | ---- | ----- | ---- | --------- | ------- |
| C1, C2, C3, C5, C44 | 5 | 10u | device:C | Capacitors_SMD:C_1206_HandSoldering | Unpolarized capacitor |
| C4, C45 | 2 | 470u | device:CP | myfootprints:CP_Radial_D6.3mm_P2.50mm_lying | Polarised capacitor |
| C6, C7, C8, C9, C10, C11, C12, C13, C14, C15, C16, C17, C18, C19, C20, C21, C22, C23, C24, C25, C26, C27, C28, C29, C30, C31, C32, C33, C34, C35, C36, C37, C38, C39, C40, C41, C42, C43 | 38 | 0.1u | device:C | Capacitors_SMD:C_0603_HandSoldering | Unpolarized capacitor |
| J1 | 1 | 952-1951-ND | conn:Conn_01x03 | Pin_Headers:Pin_Header_Straight_1x03_Pitch2.54mm_SMD_Pin1Right | Generic connector, single row, 01x03 |
| J2 | 1 | USB_OTG | conn:USB_OTG | myfootprints:1051640001 | USB mini/micro connector |
| LED1, LED2, LED3, LED4, LED5, LED6, LED7, LED8, LED9, LED10, LED11, LED12, LED13, LED14, LED15, LED16, LED17, LED18, LED19, LED20, LED21, LED22, LED23, LED24, LED25, LED26, LED27, LED28, LED29, LED30, LED31, LED32, LED33, LED34, LED35, LED36, LED37, LED38 | 38 | WS2812B | Worldsemi:WS2812B | LEDs:LED_WS2812B-PLCC4 | RGB LED with integrated controller |
| PROG1, RST1 | 2 | TL3315NF160Q | switches:SW_Push | myfootprints:TL3315 | Push button switch, generic, two pins |
| R1, R2, R3, R4, R5, R6 | 6 | 10k | device:R | Resistors_SMD:R_0603_HandSoldering | Resistor |
| R7 | 1 | 470 | device:R | Resistors_SMD:R_0603_HandSoldering | Resistor |
| R8, R9, R10 | 3 | 4k7 | device:R | Resistors_SMD:R_0603_HandSoldering | Resistor |
| U1 | 1 | AP2112 | mylibrary:AP2112 | TO_SOT_Packages_SMD:SOT-23-5_HandSoldering | |
| U2 | 1 | ESP-12 | ESP8266:ESP-12 | myfootprints:ESP-12S_SMD | ESP8266 ESP-12 module, 16 pins, 2mm, PCB antenna |
| U3 | 1 | 74AHCT1G125 | mylibrary:74AHCT1G125 | TO_SOT_Packages_SMD:SOT-23-5_HandSoldering | |
| U4 | 1 | iAQ-Core | mylibrary:iAQ-Core | myfootprints:iAQ-Core | |
| U5 | 1 | SHT31-D | mylibrary:SHT31-D | myfootprints:DFN-8-2.5x2.5-0.5mm-handsolder | |
| U6 | 1 | CCS811 | mylibrary:CCS811 | myfootprints:LGA_CCS811 | |

_Note:_ CCS811 and iAQ-Core can detect the same information and are connected to the same I2C bus. So only one of them needs to be equipped.