EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:switches
LIBS:relays
LIBS:motors
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:ESP8266
LIBS:mylibrary
LIBS:74xgxx
LIBS:ac-dc
LIBS:actel
LIBS:allegro
LIBS:Altera
LIBS:analog_devices
LIBS:battery_management
LIBS:bbd
LIBS:bosch
LIBS:brooktre
LIBS:cmos_ieee
LIBS:dc-dc
LIBS:diode
LIBS:elec-unifil
LIBS:ESD_Protection
LIBS:ftdi
LIBS:gennum
LIBS:graphic_symbols
LIBS:hc11
LIBS:infineon
LIBS:intersil
LIBS:ir
LIBS:Lattice
LIBS:leds
LIBS:LEM
LIBS:logic_programmable
LIBS:maxim
LIBS:mechanical
LIBS:microchip_dspic33dsc
LIBS:microchip_pic10mcu
LIBS:microchip_pic12mcu
LIBS:microchip_pic16mcu
LIBS:microchip_pic18mcu
LIBS:microchip_pic24mcu
LIBS:microchip_pic32mcu
LIBS:modules
LIBS:motor_drivers
LIBS:msp430
LIBS:nordicsemi
LIBS:nxp
LIBS:nxp_armmcu
LIBS:onsemi
LIBS:Oscillators
LIBS:powerint
LIBS:Power_Management
LIBS:pspice
LIBS:references
LIBS:rfcom
LIBS:RFSolutions
LIBS:sensors
LIBS:silabs
LIBS:stm8
LIBS:stm32
LIBS:supertex
LIBS:transf
LIBS:triac_thyristor
LIBS:ttl_ieee
LIBS:video
LIBS:wiznet
LIBS:Worldsemi
LIBS:Xicor
LIBS:zetex
LIBS:Zilog
LIBS:thermolight-cache
EELAYER 25 0
EELAYER END
$Descr User 5906 4331
encoding utf-8
Sheet 3 41
Title "AP2112"
Date ""
Rev "1"
Comp ""
Comment1 "5V to 3V3 voltage regulator"
Comment2 "600mA max."
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L +5V #PWR028
U 1 1 59C31E9A
P 1650 1150
F 0 "#PWR028" H 1650 1000 50  0001 C CNN
F 1 "+5V" H 1650 1290 50  0000 C CNN
F 2 "" H 1650 1150 50  0001 C CNN
F 3 "" H 1650 1150 50  0001 C CNN
	1    1650 1150
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR029
U 1 1 59C320F7
P 1950 1850
F 0 "#PWR029" H 1950 1600 50  0001 C CNN
F 1 "GND" H 1950 1700 50  0000 C CNN
F 2 "" H 1950 1850 50  0001 C CNN
F 3 "" H 1950 1850 50  0001 C CNN
	1    1950 1850
	1    0    0    -1  
$EndComp
$Comp
L +3V3 #PWR030
U 1 1 59C323F6
P 3900 1150
F 0 "#PWR030" H 3900 1000 50  0001 C CNN
F 1 "+3V3" H 3900 1290 50  0000 C CNN
F 2 "" H 3900 1150 50  0001 C CNN
F 3 "" H 3900 1150 50  0001 C CNN
	1    3900 1150
	1    0    0    -1  
$EndComp
Wire Wire Line
	1650 1150 1650 1450
Wire Wire Line
	1650 1450 2400 1450
Wire Wire Line
	1950 1850 1950 1650
Wire Wire Line
	1950 1650 2400 1650
Wire Wire Line
	3300 1650 3900 1650
Wire Wire Line
	3900 1650 3900 1150
$Comp
L R R3
U 1 1 59C3283F
P 2850 1100
F 0 "R3" V 2930 1100 50  0000 C CNN
F 1 "10k" V 2850 1100 50  0000 C CNN
F 2 "Resistors_SMD:R_0603_HandSoldering" V 2780 1100 50  0001 C CNN
F 3 "" H 2850 1100 50  0001 C CNN
	1    2850 1100
	0    1    1    0   
$EndComp
Wire Wire Line
	3000 1100 3300 1100
Wire Wire Line
	3300 1100 3300 1450
Wire Wire Line
	2700 1100 2300 1100
Wire Wire Line
	2300 1100 2300 1450
Connection ~ 2300 1450
$Comp
L C C3
U 1 1 59C330E6
P 3900 1800
F 0 "C3" H 3925 1900 50  0000 L CNN
F 1 "10u" H 3925 1700 50  0000 L CNN
F 2 "Capacitors_SMD:C_1206_HandSoldering" H 3938 1650 50  0001 C CNN
F 3 "" H 3900 1800 50  0001 C CNN
	1    3900 1800
	-1   0    0    1   
$EndComp
$Comp
L C C1
U 1 1 59C33602
P 1650 1600
F 0 "C1" H 1675 1700 50  0000 L CNN
F 1 "10u" H 1675 1500 50  0000 L CNN
F 2 "Capacitors_SMD:C_1206_HandSoldering" H 1688 1450 50  0001 C CNN
F 3 "" H 1650 1600 50  0001 C CNN
	1    1650 1600
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR031
U 1 1 59C3377E
P 1650 1850
F 0 "#PWR031" H 1650 1600 50  0001 C CNN
F 1 "GND" H 1650 1700 50  0000 C CNN
F 2 "" H 1650 1850 50  0001 C CNN
F 3 "" H 1650 1850 50  0001 C CNN
	1    1650 1850
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR032
U 1 1 59C337E3
P 3900 2050
F 0 "#PWR032" H 3900 1800 50  0001 C CNN
F 1 "GND" H 3900 1900 50  0000 C CNN
F 2 "" H 3900 2050 50  0001 C CNN
F 3 "" H 3900 2050 50  0001 C CNN
	1    3900 2050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3900 1950 3900 2050
Wire Wire Line
	1650 1750 1650 1850
$Comp
L AP2112 U1
U 1 1 59C33E1A
P 2850 1550
F 0 "U1" H 2850 1800 60  0000 C CNN
F 1 "AP2112" H 2850 1300 60  0000 C CNN
F 2 "TO_SOT_Packages_SMD:SOT-23-5_HandSoldering" H 2850 1550 60  0001 C CNN
F 3 "" H 2850 1550 60  0001 C CNN
	1    2850 1550
	1    0    0    -1  
$EndComp
$EndSCHEMATC
