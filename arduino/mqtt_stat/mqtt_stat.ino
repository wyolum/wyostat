/*
 Basic ESP32 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.



*/

#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "RTClib.h"
#include <EEPROM.h>
#include "SparkFunTMP102.h"
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
// Update these with values suitable for your network.

#include "constants.h"

WiFiClient espClient;
PubSubClient mqtt_client(espClient);
long lastMsg = 0;
char msg[50] = "Hello Mosquitto!\n(from ESP32)";
TMP102 sensor0(0x48);

// this is a typedef for topic callback funtions
typedef void (* TopicCallback_p)(byte* payload, unsigned int length);
struct TopicListener{
  char topic[50]; // topic string
  TopicCallback_p callback_p; // action
};

int bytes2int(byte* payload, unsigned int length){
  char str_payload[length + 1];
  for (int i = 0; i < length; i++) {
    str_payload[i] = (char)payload[i];
  }
  str_payload[length] = 0;
  return String(str_payload).toInt();
}

int settemplo;
int settemphi;

void settemplo_cb(byte *payload, unsigned int length){
  Serial.print("Set temp low cb: ");
  settemplo = bytes2int(payload, length);
  Serial.println(settemplo);
}

void settemphi_cb(byte *payload, unsigned int length){
  Serial.print("Set temp high cb: ");
  settemphi = bytes2int(payload, length);
  Serial.println(settemphi);
}

int n_topic_listeners = 2;
const int MAX_TOPIC_LISTENERS = 50;
TopicListener settemplo_listener = {"wyostat.settemplo", settemplo_cb};
TopicListener settemphi_listener = {"wyostat.settemphi", settemphi_cb};
TopicListener *TopicListeners[MAX_TOPIC_LISTENERS] = {&settemplo_listener,
						      &settemphi_listener};

void subscribe(char* topic, TopicCallback_p cb){
  //char topic_str[50];
  //strcpy(topic, topic_str);
  //const TopicListener listener = {topic_str, cb};
  //TopicListener listener = {"wyostat.settemplo", settemplo_cb};
  //TopicListeners[n_topic_listeners++] = &listener;
  //mqtt_client.subscribe(topic);
}

void set_furnace(bool state){
  Serial.print("Setting Furnace:");
  if(state){
    Serial.println(" On");
  }
  else{
    Serial.println(" Off");
  }
  digitalWrite(furnaceSCR, state);
}
void set_ac(bool state){
  Serial.print("Setting AC:");
  if(state){
    Serial.println(" On");
  }
  else{
    Serial.println(" Off");
  }
  digitalWrite(acSCR, state);
}
void set_fan(bool state){
  Serial.print("Setting Fan:");
  if(state){
    Serial.println(" On");
  }
  else{
    Serial.println(" Off");
  }
  digitalWrite(fanSCR, state);
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  setup_temp();
  
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(mqtt_callback);
  //subscribe("wyostat.settemplo", settemplo_cb);
}

void setup_temp(){
  sensor0.begin();
  // set the Conversion Rate (how quickly the sensor gets a new reading)
  //0-3: 0:0.25Hz, 1:1Hz, 2:4Hz, 3:8Hz
  sensor0.setConversionRate(2);
  //set Extended Mode.
  //0:12-bit Temperature(-55C to +128C) 1:13-bit Temperature(-55C to +150C)
  sensor0.setExtendedMode(0);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  bool handled = false;

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  for(int i=0; i < n_topic_listeners && !handled; i++){
    if(strcmp(topic, TopicListeners[i]->topic) == 0){
      TopicListeners[i]->callback_p(payload, length);
      handled = true;
    }
  }
  if(!handled){
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    Serial.println("Not handled.");
  }
}

void reconnect() {

  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect("ESP32Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // ... and resubscribe
      mqtt_client.subscribe("wyostat.settemplo");
      mqtt_client.subscribe("wyostat.settemphi");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
int count = 0;
float localtemperature = 0;
int last_temp = 0;
String group = String("wyostat");

void loop() {
  int temp = localtemperature;
  long now;
  String str_temp;
  String topic = String(".temp");
  
  localtemperature = sensor0.readTempF();
  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop();

  now = millis();
  if (now - lastMsg > 200) {
    lastMsg = now;
    if(temp != last_temp){
      last_temp = temp;
      Serial.println(temp);
      str_temp = String(temp);
      mqtt_client.publish((group + topic).c_str(), str_temp.c_str());
    }
  }
}
