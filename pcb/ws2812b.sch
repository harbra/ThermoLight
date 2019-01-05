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
$Descr User 5512 4134
encoding utf-8
Sheet 35 41
Title "WS2812B incl. supporting circuitry"
Date ""
Rev "1"
Comp ""
Comment1 "Din of first pixel should be protected by ~470R"
Comment2 "VCC & GND should be buffered by ~1000u"
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L WS2812B LED1
U 1 1 5A973791
P 2650 1250
AR Path="/5A97371A/5A973791" Ref="LED1"  Part="1" 
AR Path="/5A97592F/5A973791" Ref="LED2"  Part="1" 
AR Path="/5A975C0F/5A973791" Ref="LED3"  Part="1" 
AR Path="/5A975C13/5A973791" Ref="LED4"  Part="1" 
AR Path="/5A9777A7/5A973791" Ref="LED5"  Part="1" 
AR Path="/5A9777AB/5A973791" Ref="LED6"  Part="1" 
AR Path="/5A9777AF/5A973791" Ref="LED7"  Part="1" 
AR Path="/5A9777B3/5A973791" Ref="LED8"  Part="1" 
AR Path="/5A97932F/5A973791" Ref="LED9"  Part="1" 
AR Path="/5A979333/5A973791" Ref="LED10"  Part="1" 
AR Path="/5A979337/5A973791" Ref="LED11"  Part="1" 
AR Path="/5A97933B/5A973791" Ref="LED12"  Part="1" 
AR Path="/5A97933F/5A973791" Ref="LED13"  Part="1" 
AR Path="/5A979343/5A973791" Ref="LED14"  Part="1" 
AR Path="/5A979347/5A973791" Ref="LED15"  Part="1" 
AR Path="/5A97934B/5A973791" Ref="LED16"  Part="1" 
AR Path="/5A97E257/5A973791" Ref="LED17"  Part="1" 
AR Path="/5A97E25B/5A973791" Ref="LED18"  Part="1" 
AR Path="/5A97E25F/5A973791" Ref="LED19"  Part="1" 
AR Path="/5A97E263/5A973791" Ref="LED20"  Part="1" 
AR Path="/5A97E267/5A973791" Ref="LED21"  Part="1" 
AR Path="/5A985C99/5A973791" Ref="LED22"  Part="1" 
AR Path="/5A985C9D/5A973791" Ref="LED23"  Part="1" 
AR Path="/5A985CA1/5A973791" Ref="LED24"  Part="1" 
AR Path="/5AAA575D/5A973791" Ref="LED25"  Part="1" 
AR Path="/5AAA5761/5A973791" Ref="LED26"  Part="1" 
AR Path="/5AAA5765/5A973791" Ref="LED27"  Part="1" 
AR Path="/5AAA5769/5A973791" Ref="LED28"  Part="1" 
AR Path="/5AAA576D/5A973791" Ref="LED29"  Part="1" 
AR Path="/5AAA5776/5A973791" Ref="LED30"  Part="1" 
AR Path="/5AAA577A/5A973791" Ref="LED31"  Part="1" 
AR Path="/5AAA7755/5A973791" Ref="LED32"  Part="1" 
AR Path="/5AAA7759/5A973791" Ref="LED33"  Part="1" 
AR Path="/5AAA775D/5A973791" Ref="LED34"  Part="1" 
AR Path="/5AAA7761/5A973791" Ref="LED35"  Part="1" 
AR Path="/5AAA7765/5A973791" Ref="LED36"  Part="1" 
AR Path="/5AAA776E/5A973791" Ref="LED37"  Part="1" 
AR Path="/5AAA7772/5A973791" Ref="LED38"  Part="1" 
F 0 "LED38" H 2650 1050 50  0000 C CNN
F 1 "WS2812B" H 2650 1350 50  0000 C CNN
F 2 "LEDs:LED_WS2812B-PLCC4" H 2650 950 50  0001 C CNN
F 3 "" H 2650 1200 50  0001 C CNN
	1    2650 1250
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR033
U 1 1 5A9737C0
P 1650 1250
AR Path="/5A97371A/5A9737C0" Ref="#PWR033"  Part="1" 
AR Path="/5A97592F/5A9737C0" Ref="#PWR035"  Part="1" 
AR Path="/5A975C0F/5A9737C0" Ref="#PWR037"  Part="1" 
AR Path="/5A975C13/5A9737C0" Ref="#PWR039"  Part="1" 
AR Path="/5A9777A7/5A9737C0" Ref="#PWR041"  Part="1" 
AR Path="/5A9777AB/5A9737C0" Ref="#PWR043"  Part="1" 
AR Path="/5A9777AF/5A9737C0" Ref="#PWR045"  Part="1" 
AR Path="/5A9777B3/5A9737C0" Ref="#PWR047"  Part="1" 
AR Path="/5A97932F/5A9737C0" Ref="#PWR049"  Part="1" 
AR Path="/5A979333/5A9737C0" Ref="#PWR051"  Part="1" 
AR Path="/5A979337/5A9737C0" Ref="#PWR053"  Part="1" 
AR Path="/5A97933B/5A9737C0" Ref="#PWR055"  Part="1" 
AR Path="/5A97933F/5A9737C0" Ref="#PWR057"  Part="1" 
AR Path="/5A979343/5A9737C0" Ref="#PWR059"  Part="1" 
AR Path="/5A979347/5A9737C0" Ref="#PWR061"  Part="1" 
AR Path="/5A97934B/5A9737C0" Ref="#PWR063"  Part="1" 
AR Path="/5A97E257/5A9737C0" Ref="#PWR065"  Part="1" 
AR Path="/5A97E25B/5A9737C0" Ref="#PWR067"  Part="1" 
AR Path="/5A97E25F/5A9737C0" Ref="#PWR069"  Part="1" 
AR Path="/5A97E263/5A9737C0" Ref="#PWR071"  Part="1" 
AR Path="/5A97E267/5A9737C0" Ref="#PWR073"  Part="1" 
AR Path="/5A985C99/5A9737C0" Ref="#PWR075"  Part="1" 
AR Path="/5A985C9D/5A9737C0" Ref="#PWR077"  Part="1" 
AR Path="/5A985CA1/5A9737C0" Ref="#PWR079"  Part="1" 
AR Path="/5AAA575D/5A9737C0" Ref="#PWR081"  Part="1" 
AR Path="/5AAA5761/5A9737C0" Ref="#PWR083"  Part="1" 
AR Path="/5AAA5765/5A9737C0" Ref="#PWR085"  Part="1" 
AR Path="/5AAA5769/5A9737C0" Ref="#PWR087"  Part="1" 
AR Path="/5AAA576D/5A9737C0" Ref="#PWR089"  Part="1" 
AR Path="/5AAA5776/5A9737C0" Ref="#PWR091"  Part="1" 
AR Path="/5AAA577A/5A9737C0" Ref="#PWR093"  Part="1" 
AR Path="/5AAA7755/5A9737C0" Ref="#PWR095"  Part="1" 
AR Path="/5AAA7759/5A9737C0" Ref="#PWR097"  Part="1" 
AR Path="/5AAA775D/5A9737C0" Ref="#PWR099"  Part="1" 
AR Path="/5AAA7761/5A9737C0" Ref="#PWR0101"  Part="1" 
AR Path="/5AAA7765/5A9737C0" Ref="#PWR0103"  Part="1" 
AR Path="/5AAA776E/5A9737C0" Ref="#PWR0105"  Part="1" 
AR Path="/5AAA7772/5A9737C0" Ref="#PWR0107"  Part="1" 
F 0 "#PWR0107" H 1650 1100 50  0001 C CNN
F 1 "+5V" H 1650 1390 50  0000 C CNN
F 2 "" H 1650 1250 50  0001 C CNN
F 3 "" H 1650 1250 50  0001 C CNN
	1    1650 1250
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR034
U 1 1 5A9737D6
P 3150 1350
AR Path="/5A97371A/5A9737D6" Ref="#PWR034"  Part="1" 
AR Path="/5A97592F/5A9737D6" Ref="#PWR036"  Part="1" 
AR Path="/5A975C0F/5A9737D6" Ref="#PWR038"  Part="1" 
AR Path="/5A975C13/5A9737D6" Ref="#PWR040"  Part="1" 
AR Path="/5A9777A7/5A9737D6" Ref="#PWR042"  Part="1" 
AR Path="/5A9777AB/5A9737D6" Ref="#PWR044"  Part="1" 
AR Path="/5A9777AF/5A9737D6" Ref="#PWR046"  Part="1" 
AR Path="/5A9777B3/5A9737D6" Ref="#PWR048"  Part="1" 
AR Path="/5A97932F/5A9737D6" Ref="#PWR050"  Part="1" 
AR Path="/5A979333/5A9737D6" Ref="#PWR052"  Part="1" 
AR Path="/5A979337/5A9737D6" Ref="#PWR054"  Part="1" 
AR Path="/5A97933B/5A9737D6" Ref="#PWR056"  Part="1" 
AR Path="/5A97933F/5A9737D6" Ref="#PWR058"  Part="1" 
AR Path="/5A979343/5A9737D6" Ref="#PWR060"  Part="1" 
AR Path="/5A979347/5A9737D6" Ref="#PWR062"  Part="1" 
AR Path="/5A97934B/5A9737D6" Ref="#PWR064"  Part="1" 
AR Path="/5A97E257/5A9737D6" Ref="#PWR066"  Part="1" 
AR Path="/5A97E25B/5A9737D6" Ref="#PWR068"  Part="1" 
AR Path="/5A97E25F/5A9737D6" Ref="#PWR070"  Part="1" 
AR Path="/5A97E263/5A9737D6" Ref="#PWR072"  Part="1" 
AR Path="/5A97E267/5A9737D6" Ref="#PWR074"  Part="1" 
AR Path="/5A985C99/5A9737D6" Ref="#PWR076"  Part="1" 
AR Path="/5A985C9D/5A9737D6" Ref="#PWR078"  Part="1" 
AR Path="/5A985CA1/5A9737D6" Ref="#PWR080"  Part="1" 
AR Path="/5AAA575D/5A9737D6" Ref="#PWR082"  Part="1" 
AR Path="/5AAA5761/5A9737D6" Ref="#PWR084"  Part="1" 
AR Path="/5AAA5765/5A9737D6" Ref="#PWR086"  Part="1" 
AR Path="/5AAA5769/5A9737D6" Ref="#PWR088"  Part="1" 
AR Path="/5AAA576D/5A9737D6" Ref="#PWR090"  Part="1" 
AR Path="/5AAA5776/5A9737D6" Ref="#PWR092"  Part="1" 
AR Path="/5AAA577A/5A9737D6" Ref="#PWR094"  Part="1" 
AR Path="/5AAA7755/5A9737D6" Ref="#PWR096"  Part="1" 
AR Path="/5AAA7759/5A9737D6" Ref="#PWR098"  Part="1" 
AR Path="/5AAA775D/5A9737D6" Ref="#PWR0100"  Part="1" 
AR Path="/5AAA7761/5A9737D6" Ref="#PWR0102"  Part="1" 
AR Path="/5AAA7765/5A9737D6" Ref="#PWR0104"  Part="1" 
AR Path="/5AAA776E/5A9737D6" Ref="#PWR0106"  Part="1" 
AR Path="/5AAA7772/5A9737D6" Ref="#PWR0108"  Part="1" 
F 0 "#PWR0108" H 3150 1100 50  0001 C CNN
F 1 "GND" H 3150 1200 50  0000 C CNN
F 2 "" H 3150 1350 50  0001 C CNN
F 3 "" H 3150 1350 50  0001 C CNN
	1    3150 1350
	0    -1   -1   0   
$EndComp
$Comp
L C C6
U 1 1 5A9737EE
P 2650 1650
AR Path="/5A97371A/5A9737EE" Ref="C6"  Part="1" 
AR Path="/5A97592F/5A9737EE" Ref="C7"  Part="1" 
AR Path="/5A975C0F/5A9737EE" Ref="C8"  Part="1" 
AR Path="/5A975C13/5A9737EE" Ref="C9"  Part="1" 
AR Path="/5A9777A7/5A9737EE" Ref="C10"  Part="1" 
AR Path="/5A9777AB/5A9737EE" Ref="C11"  Part="1" 
AR Path="/5A9777AF/5A9737EE" Ref="C12"  Part="1" 
AR Path="/5A9777B3/5A9737EE" Ref="C13"  Part="1" 
AR Path="/5A97932F/5A9737EE" Ref="C14"  Part="1" 
AR Path="/5A979333/5A9737EE" Ref="C15"  Part="1" 
AR Path="/5A979337/5A9737EE" Ref="C16"  Part="1" 
AR Path="/5A97933B/5A9737EE" Ref="C17"  Part="1" 
AR Path="/5A97933F/5A9737EE" Ref="C18"  Part="1" 
AR Path="/5A979343/5A9737EE" Ref="C19"  Part="1" 
AR Path="/5A979347/5A9737EE" Ref="C20"  Part="1" 
AR Path="/5A97934B/5A9737EE" Ref="C21"  Part="1" 
AR Path="/5A97E257/5A9737EE" Ref="C22"  Part="1" 
AR Path="/5A97E25B/5A9737EE" Ref="C23"  Part="1" 
AR Path="/5A97E25F/5A9737EE" Ref="C24"  Part="1" 
AR Path="/5A97E263/5A9737EE" Ref="C25"  Part="1" 
AR Path="/5A97E267/5A9737EE" Ref="C26"  Part="1" 
AR Path="/5A985C99/5A9737EE" Ref="C27"  Part="1" 
AR Path="/5A985C9D/5A9737EE" Ref="C28"  Part="1" 
AR Path="/5A985CA1/5A9737EE" Ref="C29"  Part="1" 
AR Path="/5AAA575D/5A9737EE" Ref="C30"  Part="1" 
AR Path="/5AAA5761/5A9737EE" Ref="C31"  Part="1" 
AR Path="/5AAA5765/5A9737EE" Ref="C32"  Part="1" 
AR Path="/5AAA5769/5A9737EE" Ref="C33"  Part="1" 
AR Path="/5AAA576D/5A9737EE" Ref="C34"  Part="1" 
AR Path="/5AAA5776/5A9737EE" Ref="C35"  Part="1" 
AR Path="/5AAA577A/5A9737EE" Ref="C36"  Part="1" 
AR Path="/5AAA7755/5A9737EE" Ref="C37"  Part="1" 
AR Path="/5AAA7759/5A9737EE" Ref="C38"  Part="1" 
AR Path="/5AAA775D/5A9737EE" Ref="C39"  Part="1" 
AR Path="/5AAA7761/5A9737EE" Ref="C40"  Part="1" 
AR Path="/5AAA7765/5A9737EE" Ref="C41"  Part="1" 
AR Path="/5AAA776E/5A9737EE" Ref="C42"  Part="1" 
AR Path="/5AAA7772/5A9737EE" Ref="C43"  Part="1" 
F 0 "C43" H 2675 1750 50  0000 L CNN
F 1 "0.1u" H 2675 1550 50  0000 L CNN
F 2 "Capacitors_SMD:C_0603_HandSoldering" H 2688 1500 50  0001 C CNN
F 3 "" H 2650 1650 50  0001 C CNN
	1    2650 1650
	0    1    1    0   
$EndComp
Wire Wire Line
	3050 1350 3150 1350
Wire Wire Line
	1650 1250 2250 1250
Wire Wire Line
	2800 1650 3150 1650
Wire Wire Line
	3150 1650 3150 1350
Wire Wire Line
	2500 1650 1650 1650
Wire Wire Line
	1650 1650 1650 1250
Text HLabel 3450 1250 2    60   Output ~ 0
DOut
Wire Wire Line
	3450 1250 3050 1250
Text HLabel 1900 1350 0    60   Input ~ 0
DIn
Wire Wire Line
	1900 1350 2250 1350
$EndSCHEMATC
