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

#include <EEPROM.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "RTClib.h"
#include "SparkFunTMP102.h"
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include "icons.h"

// Update these with values suitable for your network.

#include "constants.h"
void publish_int(char* topic, int val);
void publish_off_on(char* topic, int val);
void setup_pins();
void setup_wifi();
void setup_temp();
void setup_display();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void connect();
void arcDemo();

//Adafruit_SSD1306  display(0x3c, 5, 4);
SSD1306  display(0x3c, 5, 4);
void drawTick(int16_t x0, int16_t y0, int16_t radius, double theta_deg, double length){
  double theta = theta_deg * PI / 180;
  double x1, x2, y1, y2;
  
  x1 = (radius - length) * cos(theta) + x0;
  y1 = -(radius - length) * sin(theta) + y0;
  x2 = (radius - 1) * cos(theta) + x0;
  y2 = -(radius - 1) * sin(theta) + y0;
  display.drawLine(x1, y1, x2, y2);
}

void arc_helper(int x0, int y0, int x, int y, int theta0_deg, int theta1_deg){
  double theta0 = theta0_deg * PI / 180;
  double theta1 = theta1_deg * PI / 180;
  double theta = atan2(-y + y0, x - x0);
  bool good0, good1, good2;
  good0 = theta0 <= theta - 2 * PI && theta - 2 * PI <= theta1;
  good1 = theta0 <= theta && theta <= theta1;
  good2 = theta0 <= theta + 2 * PI && theta + 2 * PI <= theta1;
  
  if(good0 || good1 || good2){
    display.setPixel(x, y);
  }
}

void drawArc(int16_t x0, int16_t y0, int16_t radius, double theta0_deg, double theta1_deg){
  int16_t x = 0, y = radius;
  int16_t dp = 1 - radius;
  if(theta0_deg > theta1_deg){
    int16_t swp = theta0_deg;
    theta0_deg = theta1_deg;
    theta1_deg = swp;
  }
  do {
    if (dp < 0)
      dp = dp + 2 * (++x) + 3;
    else
      dp = dp + 2 * (++x) - 2 * (--y) + 5;
    arc_helper(x0, y0, x0 + x, y0 + y, theta0_deg, theta1_deg);
    arc_helper(x0, y0, x0 - x, y0 + y, theta0_deg, theta1_deg);
    arc_helper(x0, y0, x0 + x, y0 - y, theta0_deg, theta1_deg);
    arc_helper(x0, y0, x0 - x, y0 - y, theta0_deg, theta1_deg);
    arc_helper(x0, y0, x0 + y, y0 + x, theta0_deg, theta1_deg);
    arc_helper(x0, y0, x0 - y, y0 + x, theta0_deg, theta1_deg);
    arc_helper(x0, y0, x0 + y, y0 - x, theta0_deg, theta1_deg);
    arc_helper(x0, y0, x0 - y, y0 - x, theta0_deg, theta1_deg);
  } while (x < y);
  
  arc_helper(x0, y0, x0 + radius, y0, theta0_deg, theta1_deg);
  arc_helper(x0, y0, x0, y0 + radius, theta0_deg, theta1_deg);
  arc_helper(x0, y0, x0 - radius, y0, theta0_deg, theta1_deg);
  arc_helper(x0, y0, x0, y0 - radius, theta0_deg, theta1_deg);
}


int count = 0;
float localtemp = 0;
int last_temp = 0;

WiFiClient espClient;
PubSubClient mqtt_client(espClient);
long lastMsg = 0;
char msg[50] = "Hello Mosquitto!\n(from ESP32)";
TMP102 sensor0(0x48);

bool furnace_state = false;
bool ac_state = false;
bool fan_state = false;

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

int targettemp;
char hvacmode[50] = "cooling";

void _targettemp(int temp){
  if(LOWEST_TEMP <= temp && temp <= HIGHEST_TEMP){
    if(temp != targettemp){
      targettemp = temp;
      arcDemo();
      EEPROM.write(TARGETTEMP_ADDR, targettemp);
      EEPROM.commit();
    }
  }
}
void targettemp_cb(byte *payload, unsigned int length){
  String str_temp;
    
  Serial.print("Set temp cb: ");
  Serial.println(bytes2int(payload, length));
  _targettemp(bytes2int(payload, length));  
}

void requesttemp_cb(byte *payload, unsigned int length){
  Serial.println("Request temp cb.");
  Serial.print("Sending:");
  Serial.println(localtemp);
  publish_hvac_state();
}

void requesttargettemp_cb(byte *payload, unsigned int length){
  Serial.println("Request target temp cb.");
  Serial.print("Sending:");
  Serial.println(targettemp);
  publish_hvac_state();
}

void requestmode_cb(byte *payload, unsigned int length){
  Serial.println("Request mode cb.");
  Serial.print("Sending:");
  Serial.println(hvacmode);
  mqtt_client.publish("wyostat.hvacmode", hvacmode);  
  publish_hvac_state();
}

void publish_hvac_state(){
  char *state;
  if(ac_state){
    state = "cooling";
  }
  else if(furnace_state){
    state = "heating";
  }
  else{
    state = "off";
  }
  Serial.print("Sending:");
  Serial.println(state);
  mqtt_client.publish("wyostat.hvac_state", state);
  publish_int("wyostat.temp", localtemp);
  publish_int("wyostat.targettemp", targettemp);
}
void requeststate_cb(byte *payload, unsigned int length){
  Serial.println("Request state cb.");
  publish_hvac_state();
}

void hvacmode_cb(byte *payload, unsigned int length){
  for(int i = 0; i < length; i++){
    hvacmode[i] = (char)payload[i];
  }
  hvacmode[length] = 0;
  Serial.print("wyostat.hvacmode: ");
  Serial.println(hvacmode);
}

int n_topic_listeners = 6;
const int MAX_TOPIC_LISTENERS = 50;

TopicListener targettemp_listener = {"wyostat.targettemp", targettemp_cb};
TopicListener requesttemp_listener = {"wyostat.requesttemp", requesttemp_cb};
TopicListener requesttargettemp_listener = {"wyostat.requesttargettemp", requesttargettemp_cb};
TopicListener requestmode_listener = {"wyostat.requeststate", requestmode_cb};
TopicListener requeststate_listener = {"wyostat.requeststate", requeststate_cb};
TopicListener hvacmode_listener = {"wyostat.hvacmode", hvacmode_cb};

TopicListener *TopicListeners[MAX_TOPIC_LISTENERS] = {&targettemp_listener,
						      &requesttemp_listener,
						      &requesttargettemp_listener,
						      &requestmode_listener,
						      &hvacmode_listener,
						      &requeststate_listener,
						      };

void loadSettings()
{
  targettemp = EEPROM.read(TARGETTEMP_ADDR);
  if(targettemp == 255){
    targettemp = DEFAULT_TARGETTEMP;
  }
}

void set_furnace(bool state){
  if(state != furnace_state){
    Serial.print("Setting Furnace:");
    Serial.println(furnace_state);
    furnace_state = state;
    digitalWrite(furnaceSCR, state);
    publish_off_on("wyostat.furnace_state", furnace_state);
    publish_hvac_state();
    display.setColor(BLACK);
    display.fillRect(16, 16, 32, 32);
    display.setColor(WHITE);
    if(furnace_state){
      display.drawXbm(16, 16, fire_width, fire_height, fire_bits);
    }
    display.display();
  }
}

void set_ac(bool state){
  if(state != ac_state){
    ac_state = state;
    digitalWrite(acSCR, state);
    Serial.print("Setting AC:");
    publish_off_on("wyostat.furnace_state", ac_state);
    publish_hvac_state();
    display.setColor(BLACK);
    display.fillRect(16, 16, 32, 32);
    display.setColor(WHITE);
    if(ac_state){
      display.drawXbm(16, 16, flake_width, flake_height, flake_bits);
    }
    display.display();
  }
}

void set_fan(bool state){
  if(state != fan_state){
    Serial.print("Setting Fan:");
    if(state){
      Serial.println(" On");
      mqtt_client.publish("wyostat.fan_state", "1");
    }
    else{
      Serial.println(" Off");
      mqtt_client.publish("wyostat.fan_state", "0");
    }
    fan_state = state;
    digitalWrite(fanSCR, state);
  }
}

void setup() {
  String str;
  
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_pins();
  setup_wifi();
  setup_temp();
  
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(mqtt_callback);

  EEPROM.begin(512);
  loadSettings();
  setup_display();
  connect();
}

void setup_display(){
  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();
  display.display();

  arcDemo();
}
void font_demo(){
  // Font Demo1
  // create more fonts at http://oleddisplay.squix.ch/
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Hello world");
  display.display(); delay(1000);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 10, "Hello world");
  display.display(); delay(1000);
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 26, "Hello world");
  display.display(); delay(1000);
}

void xbmDemo(){
  // test 1
  // display.drawXbm(0, -30, xbm_width, xbm_height, xbm_bits);
  // display.invertDisplay();

  // test 2
  display.drawXbm(0, 0, leaf_width, leaf_height, leaf_bits);
  display.drawXbm(0, 32, flake_width, flake_height, flake_bits);
  display.drawXbm(32, 32, fire_width, fire_height, fire_bits);

  return;
}

void arcDemo(){
  double theta_deg;

  int x0 = 128 - 32 - 1;
  int y0 = 32;
  int y;
  
  int theta0, theta1;
  String tempstr;
  double gain = 315 / 50.;
  
  if (targettemp < LOWEST_TEMP){
    theta1 = gain * (80 - LOWEST_TEMP);
  }
  else{
    theta1 = gain * (80 - targettemp);
  }
  if (targettemp > HIGHEST_TEMP){
    theta1 = gain * (80 - HIGHEST_TEMP);
  }
  else{
    theta1 = gain * (80 - targettemp);
  }
  if (localtemp < LOWEST_TEMP){
    theta0 = gain * (80 - LOWEST_TEMP);
  }
  else{
    theta0 = gain * (80 - localtemp);
  }
  if (localtemp > HIGHEST_TEMP){
    theta0 = gain * (80 - HIGHEST_TEMP);
  }
  else{
    theta0 = gain * (80 - localtemp);
  }

  display.setColor(BLACK); // clear right half of display
  display.fillRect(64, 0, 64, 64);
  display.setColor(WHITE);

  drawArc(x0, y0, 31, -67, 247);
  drawArc(x0, y0, 32, -67, 247);
  drawArc(x0, y0, 33, -67, 247);
	     
  drawArc(x0, y0, 27, theta0, theta1);
  drawArc(x0, y0, 26, theta0, theta1);

  int n_tick = 6;
  for(int i=0; i<n_tick; i++){
    theta_deg = (int)(-67.5 + 315. / (n_tick - 1) * i);
    //drawTick(x0, y0, 32, theta_deg, 5);
  }
  
  // display target temp
  display.setFont(ArialMT_Plain_24);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  tempstr = String(targettemp);
  display.drawString(x0, y0 - 12, tempstr.c_str());

  // display local temp
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  tempstr = String((int)localtemp);
  if(localtemp < targettemp){
    y = y0 - 24;
  }
  else{
    y = y0 + 12;
  }
  display.drawString(x0, y, tempstr.c_str());

  /*
  if(furnace_state){
    display.setColor(BLACK);
    display.fillRect(16, 16, 32, 32);
    display.setColor(WHITE);
    display.drawXbm(16, 16, fire_width, fire_height, fire_bits);
    display.display();
  }
  if(ac_state){
    display.setColor(BLACK);
    display.fillRect(16, 16, 32, 32);
    display.setColor(WHITE);
    display.drawXbm(16, 16, flake_width, flake_height, flake_bits);
    display.display();
    }
  */
  display.display();
}

  
void setup_pins(){
  pinMode(furnaceSCR,OUTPUT);
  pinMode(fanSCR,OUTPUT);
  pinMode(acSCR,OUTPUT);
  digitalWrite(furnaceSCR,LOW);
  digitalWrite(fanSCR,LOW);
  digitalWrite(acSCR,LOW);
}

void setup_temp(){
  sensor0.begin();
  // set the Conversion Rate (how quickly the sensor gets a new reading)
  //0-3: 0:0.25Hz, 1:1Hz, 2:4Hz, 3:8Hz
  sensor0.setConversionRate(2);
  //set Extended Mode.
  //0:12-bit Temperature(-55C to +128C) 1:13-bit Temperature(-55C to +150C)
  sensor0.setExtendedMode(0);
  localtemp = sensor0.readTempF();
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
  Serial.println("] ");
  for(int i=0; i < n_topic_listeners && !handled; i++){
    if(strcmp(topic, TopicListeners[i]->topic) == 0){
      TopicListeners[i]->callback_p(payload, length);
      handled = true;
    }
  }
  if(!handled){
    for (int i = 0; i < length; i++) {
      Serial.print(payload[i]);
    }
    Serial.println();
    Serial.println("Not handled.");
  }
}

void publish_off_on(char* topic, int val){
  if(val){
    mqtt_client.publish(topic, "on");
  }
  else{
    mqtt_client.publish(topic, "off");
  }
}

void publish_int(char* topic, int val){
  String val_str = String(val);
  mqtt_client.publish(topic, val_str.c_str());
}
void subscribe(){
  for(int i=0; i < n_topic_listeners; i++){
    mqtt_client.subscribe(TopicListeners[i]->topic);
  }
  //mqtt_client.subscribe("wyostat.targettemp");
  //mqtt_client.subscribe("wyostat.requesttemp");
  //mqtt_client.subscribe("wyostat.requesttargettemp");
  //mqtt_client.subscribe("wyostat.requeststate");
}

void connect(){
  String str;
  
  while (!mqtt_client.connected()) {
    if(mqtt_client.connect("ESP32Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // ... and resubscribe
      subscribe();
      
      publish_int("wyostat.fan_state",     fan_state);
      publish_int("wyostat.ac_state",      ac_state);
      publish_int("wyostat.furnace_state", furnace_state);
      publish_int("wyostat.targettemp",    targettemp);
      
      Serial.print("TARGETTEMP:");
      Serial.println(targettemp);
    }
    else{
      Serial.print("Try again in 5 seconds.");
      delay(5000);
    }
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
      subscribe();
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  int temp = localtemp;
  long now;
  String str_temp;
  String topic = String("wyostat.temp");
  
  localtemp = .9999 * localtemp + .0001 * sensor0.readTempF();
  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop();

  if(hvacmode[0] == 'h'){ // heating mode
    set_ac(false);
    if (int(localtemp) > targettemp){ // too hot
      set_furnace(false);
      set_fan(false);
    }
    else if(int(localtemp) <= targettemp + HILOGAP){ // too cold
      set_furnace(true);
      set_fan(true);
    }
    else{ // just right
    }
  }
  else if(hvacmode[0] == 'c'){ // cooling mode
    set_furnace(false);
    if (int(localtemp) > targettemp){ // too hot
      set_ac(true);
      set_fan(true);
    }
    else if (int(localtemp) <= targettemp - HILOGAP){ // too cold
      set_ac(false);
      set_fan(false);
    }
    else{ // just right
    }
  }
  else{
    set_ac(false);
    set_furnace(false);
    set_fan(false);
  }

  if((int)temp != int(localtemp)){
    arcDemo();
  }
  now = millis();
  if (now - lastMsg > 200) {
    lastMsg = now;
    if(temp != last_temp){
      last_temp = temp;
      Serial.println(temp);
      str_temp = String(temp);
      mqtt_client.publish(topic.c_str(), str_temp.c_str());
    }
  }
}
