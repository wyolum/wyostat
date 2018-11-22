/******************************************************************************
TMP102_example.ino
Example for the TMP102 I2C Temperature Sensor
Alex Wende @ SparkFun Electronics
April 29th 2016
~

This sketch configures the TMP102 temperature sensor and prints the
temperature and alert state (both from the physical pin, as well as by
reading from the configuration register.

Resources:
Wire.h (included with Arduino IDE)
SparkFunTMP102.h

Development environment specifics:
Arduino 1.0+
Hardware Version 13

This code is beerware; if you see me (or any other SparkFun employee) at
the local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.   
******************************************************************************/

#include <Wire.h> // Used to establied serial communication on the I2C bus
#include "SparkFunTMP102.h" // Used to send and recieve specific information from our sensor
#include <WiFiManager.h>
//#include<WiFi.h>


// Connections
// VCC = 3.3V
// GND = GND
// SDA = A4
// SCL = A5
const int ALERT_PIN = A3;

TMP102 sensor0(0x48); // Initialize sensor at I2C address 0x48
// Sensor address can be changed with an external jumper to:
// ADD0 - Address
//  VCC - 0x49
//  SDA - 0x4A
//  SCL - 0x4B

////////////////////////////////////////////////////////////////////////////////
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
SSD1306  display(0x3c, 5, 4);
WiFiManager wifiManager;

void setup_display(){
  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();
  display.display();
}
void font_demo(){
  // Font Demo1
  // create more fonts at http://oleddisplay.squix.ch/
  int temp = sensor0.readTempF();
  display.clear();
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 26, String((int)temp));
  display.display(); 
}
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  // reset network?
  // wifiManager.startConfigPortal("WyoStat");
  wifiManager.autoConnect("WyoStat");

  Serial.println("Yay connected!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

////////////////////////////////////////////////////////////////////////////////

void setup_sensor(){
  sensor0.begin();  // Join I2C bus

  // Initialize sensor0 settings
  // These settings are saved in the sensor, even if it loses power
  
  // set the number of consecutive faults before triggering alarm.
  // 0-3: 0:1 fault, 1:2 faults, 2:4 faults, 3:6 faults.
  sensor0.setFault(0);  // Trigger alarm immediately
  
  // set the polarity of the Alarm. (0:Active LOW, 1:Active HIGH).
  sensor0.setAlertPolarity(1); // Active HIGH
  
  // set the sensor in Comparator Mode (0) or Interrupt Mode (1).
  sensor0.setAlertMode(0); // Comparator Mode.
  
  // set the Conversion Rate (how quickly the sensor gets a new reading)
  //0-3: 0:0.25Hz, 1:1Hz, 2:4Hz, 3:8Hz
  sensor0.setConversionRate(2);
  
  //set Extended Mode.
  //0:12-bit Temperature(-55C to +128C) 1:13-bit Temperature(-55C to +150C)
  sensor0.setExtendedMode(0);

  //set T_HIGH, the upper limit to trigger the alert on
  sensor0.setHighTempF(85.0);  // set T_HIGH in F
  //sensor0.setHighTempC(29.4); // set T_HIGH in C
  
  //set T_LOW, the lower limit to shut turn off the alert
  sensor0.setLowTempF(84.0);  // set T_LOW in F
  //sensor0.setLowTempC(26.67); // set T_LOW in C
  // read temperature data
}

void setup() {
  Serial.begin(115200); // Start serial communication at 9600 baud
  pinMode(ALERT_PIN,INPUT);  // Declare alertPin as an input
  setup_sensor();
  setup_display();
  setup_wifi();// comment out this line and sensor and display both wo
}
 
void loop()
{
  float temperature;
  boolean alertPinState, alertRegisterState;
  
  // Turn sensor on to start temperature measurement.
  // Current consumtion typically ~10uA.
  sensor0.wakeup();

  // read temperature data
  temperature = sensor0.readTempF();
  //temperature = sensor0.readTempC();

  // Check for Alert
  alertPinState = digitalRead(ALERT_PIN); // read the Alert from pin
  alertRegisterState = sensor0.alert();   // read the Alert from register
  
  // Place sensor in sleep mode to save power.
  // Current consumtion typically <0.5uA.
  sensor0.sleep();

  // Print temperature and alarm state
  Serial.print("Temperature: ");
  Serial.print(temperature);
  
  Serial.print("\tAlert Pin: ");
  Serial.print(alertPinState);
  
  Serial.print("\tAlert Register: ");
  Serial.println(alertRegisterState);
  
  font_demo();
  Serial.print('loop()');
  delay(1000);  // Wait 1000ms
}
