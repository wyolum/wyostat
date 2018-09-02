EESchema Schematic File Version 4
LIBS:wyostat-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 2
Title "WYOSTAT"
Date "2018-04-05"
Rev "rev 1"
Comp "WyoLum"
Comment1 "https://github.com/wyolum/wyostat"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L wyostat:R R28
U 1 1 5A448924
P 2525 2750
F 0 "R28" H 2375 2750 50  0000 C CNB
F 1 "4K7" V 2532 2751 40  0000 C CNN
F 2 "wyostat_fp:R_0805" V 2455 2750 30  0001 C CNN
F 3 "~" H 2525 2750 30  0000 C CNN
F 4 "mfr_pn" H 2525 2750 50  0001 C CNN "manf#"
	1    2525 2750
	1    0    0    1   
$EndComp
$Comp
L wyostat:GND #PWR?
U 1 1 5A44892B
P 3675 4175
F 0 "#PWR?" H 3675 4175 30  0001 C CNN
F 1 "GND" H 3675 4105 30  0001 C CNN
F 2 "" H 3675 4175 60  0000 C CNN
F 3 "" H 3675 4175 60  0000 C CNN
F 4 "mfr_pn" H 3675 4175 50  0001 C CNN "manf#"
	1    3675 4175
	1    0    0    -1  
$EndComp
$Comp
L wyostat:R R26
U 1 1 5A448933
P 4900 2550
F 0 "R26" H 4750 2550 50  0000 C CNB
F 1 "10k" V 4907 2551 40  0000 C CNN
F 2 "wyostat_fp:R_0805" V 4830 2550 30  0001 C CNN
F 3 "~" H 4900 2550 30  0000 C CNN
F 4 "mfr_pn" H 4900 2550 50  0001 C CNN "manf#"
	1    4900 2550
	1    0    0    -1  
$EndComp
$Comp
L wyostat:R R27
U 1 1 5A44893A
P 5050 2550
F 0 "R27" H 5200 2550 50  0000 C CNB
F 1 "10k" V 5057 2551 40  0000 C CNN
F 2 "wyostat_fp:R_0805" V 4980 2550 30  0001 C CNN
F 3 "~" H 5050 2550 30  0000 C CNN
F 4 "mfr_pn" H 5050 2550 50  0001 C CNN "manf#"
	1    5050 2550
	1    0    0    -1  
$EndComp
Text Label 4600 3000 0    50   ~ 10
RESET
$Comp
L wyostat:PWR_FLAG #FLG?
U 1 1 5A448942
P 2000 2325
F 0 "#FLG?" H 2000 2595 30  0001 C CNN
F 1 "PWR_FLAG" H 2000 2555 30  0000 C CNN
F 2 "" H 2000 2325 60  0000 C CNN
F 3 "" H 2000 2325 60  0000 C CNN
F 4 "mfr_pn" H 2000 2325 50  0001 C CNN "manf#"
	1    2000 2325
	1    0    0    -1  
$EndComp
$Comp
L wyostat:DS3231N U6
U 1 1 5A448948
P 3675 3200
F 0 "U6" H 3075 3825 50  0000 L CNB
F 1 "DS3231N" H 3075 3750 40  0000 L CNN
F 2 "wyostat_fp:DS3231" H 3675 3200 60  0001 C CNN
F 3 "~" H 3675 3200 60  0000 C CNN
F 4 "mfr_pn" H 3675 3200 50  0001 C CNN "manf#"
	1    3675 3200
	1    0    0    -1  
$EndComp
$Comp
L wyostat:R R29
U 1 1 5A44894F
P 2675 2750
F 0 "R29" H 2825 2750 50  0000 C CNB
F 1 "4K7" V 2682 2751 40  0000 C CNN
F 2 "wyostat_fp:R_0805" V 2605 2750 30  0001 C CNN
F 3 "~" H 2675 2750 30  0000 C CNN
F 4 "mfr_pn" H 2675 2750 50  0001 C CNN "manf#"
	1    2675 2750
	1    0    0    1   
$EndComp
$Comp
L wyostat:Batt_RTC BT1
U 1 1 5A448957
P 2000 3200
F 0 "BT1" V 1900 3125 50  0000 C CNB
F 1 "Batt_RTC" H 2150 3125 40  0000 C CNN
F 2 "wyostat_fp:CR2032_SMD2" H 2000 3200 60  0001 C CNN
F 3 "" H 2000 3200 60  0000 C CNN
F 4 "mfr_pn" H 2000 3200 50  0001 C CNN "manf#"
F 5 "BU2032SM-HD-GCT-ND " H 2100 3500 60  0001 C CNN "Digikey"
	1    2000 3200
	0    1    1    0   
$EndComp
Text Label 2075 2400 0    50   ~ 10
BAT
Text Label 7225 2225 0    50   ~ 10
3V3
Text Label 7225 4100 0    50   ~ 10
GND
Wire Wire Line
	4575 3250 4625 3250
Wire Wire Line
	3675 2225 3675 2400
Connection ~ 4900 2225
Wire Wire Line
	4900 2225 4900 2400
Wire Wire Line
	2525 2225 2675 2225
Wire Wire Line
	5050 2225 5050 2400
Wire Wire Line
	2000 2325 2000 2400
Wire Wire Line
	4575 3000 7550 3000
Wire Wire Line
	4625 3250 4625 3350
Wire Wire Line
	4625 3650 4575 3650
Connection ~ 4625 3650
Wire Wire Line
	4625 3550 4575 3550
Connection ~ 4625 3550
Wire Wire Line
	4625 3450 4575 3450
Connection ~ 4625 3450
Wire Wire Line
	4625 3350 4575 3350
Connection ~ 4625 3350
Wire Wire Line
	2000 2400 3425 2400
Wire Wire Line
	2000 3400 2000 3475
Wire Wire Line
	2200 2850 2200 3025
Wire Wire Line
	2000 2850 2200 2850
Connection ~ 2000 2850
Wire Wire Line
	2200 3325 2200 3475
Wire Wire Line
	2200 3475 2000 3475
Connection ~ 2000 3475
Connection ~ 2000 2400
Wire Wire Line
	2000 4100 3675 4100
Wire Wire Line
	3675 4050 3675 4100
Wire Wire Line
	3875 4100 3875 4050
Connection ~ 3675 4100
Wire Wire Line
	4075 4100 4075 4050
Wire Wire Line
	3975 4100 3975 4050
Connection ~ 3875 4100
Connection ~ 3975 4100
Connection ~ 4075 4100
Wire Wire Line
	2525 2225 2525 2600
Wire Wire Line
	2675 2225 2675 2600
Wire Wire Line
	2675 2900 2675 3150
Wire Wire Line
	2675 3150 2775 3150
Wire Wire Line
	2525 2900 2525 3300
Wire Wire Line
	2525 3300 2775 3300
Connection ~ 2675 3150
Connection ~ 2525 3300
Wire Wire Line
	4575 2850 4900 2850
Wire Wire Line
	4900 2850 4900 2700
Wire Wire Line
	5050 2700 5050 3150
Wire Wire Line
	5050 3150 4575 3150
Connection ~ 2675 2225
Connection ~ 3675 2225
Connection ~ 5050 2225
Connection ~ 4625 4100
Text HLabel 7775 2225 2    40   Input ~ 0
3V3
Text HLabel 7775 4100 2    40   Input ~ 0
GND
Text HLabel 7775 5025 2    40   Input ~ 0
SDA
Text HLabel 7775 5225 2    40   Input ~ 0
SCL
$Comp
L wyostat:C_NP C8
U 1 1 5A613CC0
P 2200 3175
F 0 "C8" H 2303 3213 50  0000 L CNB
F 1 "100nF" H 2303 3138 40  0000 L CNN
F 2 "wyostat_fp:C_0805" H 2200 3175 60  0001 C CNN
F 3 "" H 2200 3175 60  0000 C CNN
F 4 "mfr_pn" H 2200 3175 50  0001 C CNN "manf#"
	1    2200 3175
	1    0    0    -1  
$EndComp
Wire Wire Line
	4900 2225 5050 2225
Wire Wire Line
	4625 3650 4625 4100
Wire Wire Line
	4625 3550 4625 3650
Wire Wire Line
	4625 3450 4625 3550
Wire Wire Line
	4625 3350 4625 3450
Wire Wire Line
	2000 2850 2000 2950
Wire Wire Line
	2000 3475 2000 4100
Wire Wire Line
	2000 2400 2000 2850
Wire Wire Line
	3675 4100 3875 4100
Wire Wire Line
	3675 4100 3675 4175
Wire Wire Line
	3875 4100 3975 4100
Wire Wire Line
	3975 4100 4075 4100
Wire Wire Line
	4075 4100 4625 4100
Wire Wire Line
	2675 3150 2675 5025
Wire Wire Line
	2525 3300 2525 5225
Wire Wire Line
	2675 2225 3675 2225
Wire Wire Line
	3675 2225 4900 2225
Wire Wire Line
	5050 2225 7775 2225
Wire Wire Line
	4625 4100 7775 4100
$Comp
L wyostat:Conn_01x01 TP1
U 1 1 5A48EA43
P 7750 3000
F 0 "TP1" H 7825 3000 50  0000 L CNN
F 1 "~RST" H 8000 3000 50  0000 L CNN
F 2 "wyostat_fp:Pin_Header_Straight_1x01" H 7750 3000 50  0001 C CNN
F 3 "~" H 7750 3000 50  0001 C CNN
F 4 "mfr_pn" H 7750 3000 50  0001 C CNN "manf#"
	1    7750 3000
	1    0    0    -1  
$EndComp
Text Label 4600 2850 0    50   ~ 10
SQR
Text Label 4600 3150 0    50   ~ 10
32kHz
Wire Wire Line
	7775 5025 2675 5025
Wire Wire Line
	2525 5225 7775 5225
Text Label 7225 5025 0    50   ~ 10
SDA
Text Label 7225 5225 0    50   ~ 10
SCL
$Comp
L wyostat:Conn_01x01 TP2
U 1 1 5A95B66E
P 7750 2850
F 0 "TP2" H 7825 2850 50  0000 L CNN
F 1 "SQR" H 8000 2850 50  0000 L CNN
F 2 "wyostat_fp:Pin_Header_Straight_1x01" H 7750 2850 50  0001 C CNN
F 3 "~" H 7750 2850 50  0001 C CNN
F 4 "mfr_pn" H 7750 2850 50  0001 C CNN "manf#"
	1    7750 2850
	1    0    0    -1  
$EndComp
$Comp
L wyostat:Conn_01x01 TP3
U 1 1 5A95B721
P 7750 3150
F 0 "TP3" H 7825 3150 50  0000 L CNN
F 1 "32kHz" H 8000 3150 50  0000 L CNN
F 2 "wyostat_fp:Pin_Header_Straight_1x01" H 7750 3150 50  0001 C CNN
F 3 "~" H 7750 3150 50  0001 C CNN
F 4 "mfr_pn" H 7750 3150 50  0001 C CNN "manf#"
	1    7750 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	7550 2850 4900 2850
Connection ~ 4900 2850
Wire Wire Line
	5050 3150 7550 3150
Connection ~ 5050 3150
$EndSCHEMATC
