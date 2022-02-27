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

#include <DNSServer.h>
#include <EEPROM.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <EEPROMAnything.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <credentials.h>
#include "RTClib.h"
#include "SparkFunTMP102.h"
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include "icons.h"
#include "get_time.h"

// Update these with values suitable for your network.

#include "constants.h"

struct config_t{
  int timezone;
  uint8_t brightness;
  uint8_t display_idx;
  bool factory_reset;
  bool use_wifi;
  bool use_ip_timezone;
  uint8_t mqtt_ip[4];
  bool flip_display;
  uint32_t last_tz_lookup; // look up tz info every Sunday at 3:00 AM
  uint8_t solid_color_rgb[3];
  bool use_ntp_time;
  bool wifi_reset;
  uint8_t faceplate_idx;
} my_config;


void publish_int(char* topic, int val);
void publish_off_on(char* topic, int val);
void publish_bool(char* topic, bool val);
void setup_pins();
void setup_wifi();
void setup_temp();
void setup_display();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void mqtt_connect();
void mqtt_reconnect();
void displayTemp();

WiFiManager wifiManager;

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
float localtemp = 50.;
int last_temp = 0;

WiFiClient espClient;
PubSubClient mqtt_client(espClient);
long lastMsg = 0;
char msg[50] = "Hello Mosquitto!\n(from ESP32)";
TMP102 sensor0;
NTPClock ntp_clock;
DS3231Clock ds3231_clock;
DoomsdayClock doomsday_clock;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "us.pool.ntp.org", 0, 60000); //Default

bool furnace_state = false;
bool ac_state = false;
bool fan_state = false;
bool away;

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

bool bytes2bool(byte* payload, unsigned int length){
  bool out = false;
  if(length > 0){
    out = payload[0] == 't';
  }
  return out;
}

uint8_t targettemps[2] = {60, 68};
uint8_t targettimes[2] = { 6, 22};
char hvacmode[50] = "heating";

int get_target_index(){
  uint8_t hh = ds3231_clock.hours();
  uint8_t out;
  if(hh < targettimes[0]){
    out = 0;
  }
  else if(hh < targettimes[1]){
    out = 1;
  }
  else{
    out = 0;
  }
  return out;
}
 
uint8_t get_targettemp(){
  uint8_t index = get_target_index();
  return targettemps[index];
}

uint8_t set_targettemp(uint8_t t){
  uint8_t index = get_target_index();
  targettemps[index] = t;
}

void _targettemp(int temp){
  if(LOWEST_TEMP <= temp && temp <= HIGHEST_TEMP){
    if(temp != get_targettemp()){
      set_targettemp(temp);
      displayTemp();
      EEPROM.write(TARGETTEMP_ADDR, temp);
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

void away_cb(byte *payload, unsigned int length){
  String str_temp;
  bool tmp = bytes2bool(payload, length);
  if(tmp != away){
    away = tmp;
    EEPROM.write(AWAY_ADDR, away);
    EEPROM.commit();
    displayTemp();
  }

  Serial.print("Set away cb: ");
  Serial.println(away);
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
  Serial.println(get_targettemp());
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
  publish_bool("wyostat.away", away);  
  publish_int("wyostat.temp", localtemp);
  publish_int("wyostat.targettemp", get_targettemp());
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
TopicListener away_listener = {"wyostat.away", away_cb};
TopicListener requesttemp_listener = {"wyostat.requesttemp", requesttemp_cb};
TopicListener requesttargettemp_listener = {"wyostat.requesttargettemp", requesttargettemp_cb};
TopicListener requestmode_listener = {"wyostat.requeststate", requestmode_cb};
TopicListener requeststate_listener = {"wyostat.requeststate", requeststate_cb};
TopicListener hvacmode_listener = {"wyostat.hvacmode", hvacmode_cb};

TopicListener *TopicListeners[MAX_TOPIC_LISTENERS] = {&targettemp_listener,
						      &away_listener,
						      &requesttemp_listener,
						      &requesttargettemp_listener,
						      &requestmode_listener,
						      &hvacmode_listener,
						      &requeststate_listener,
						      };


// return json value for name identified (or "not found")
String jsonLookup(String s, String name){
  String out;
  int start, stop;
  
  name = String("\"") + name + String("\""); // add bounding quotes

  start = s.indexOf(name);
  if(start < 0){
    out = String("not found");
  }
  else{
    start +=  name.length() + 2;
    stop = s.indexOf('"', start);
    out = s.substring(start, stop);
  }
  return out;
}

void set_timezone_from_ip(){

  HTTPClient http;
  String payload;
  
  Serial.print("[HTTP] begin...\n");
  // configure traged server and url
  //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
  // http.begin("http://example.com/index.html"); //HTTP

  //http.begin("https://timezoneapi.io/api/ip");// no longer works!
  String url = String("https://www.wyolum.com/utc_offset/utc_offset.py") +
    String("?refresh=") + String(millis()) +
    String("&localip=") +
    String(WiFi.localIP()[0]) + String('.') + 
    String(WiFi.localIP()[1]) + String('.') + 
    String(WiFi.localIP()[2]) + String('.') + 
    String(WiFi.localIP()[3]) + String('&') +
    String("macaddress=") + WiFi.macAddress() + String('&') + 
    String("dev_type=ClockIOT");
  Serial.println(url);
  http.begin(url);
  
  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
  
  // httpCode will be negative on error
  //httpCode = -1; // force error
  if(httpCode < 0){
    http.end();
    url = String("https://ipapi.co/json/?key=");
    Serial.print("Using backup url:");
    Serial.println(url + "<SECRET KEY>");
    http.begin(url + ipapikey);
  
    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    httpCode = http.GET();
  }
  payload = http.getString();
  if(httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    
    // file found at server
    //String findme = String("offset_seconds");
    if(httpCode == HTTP_CODE_OK) {
      Serial.print("payload:");
      Serial.println(payload);
      payload.replace(" ", "");
      String offset_str = jsonLookup(payload, String("utc_offset"));
      int hours = offset_str.substring(0, 3).toInt();
      int minutes = offset_str.substring(3, 5).toInt();
      if(hours < 0){
	minutes *= -1;
      }
      int offset = hours * 3600 + minutes * 60;

      String utc_str =jsonLookup(payload, String("utc"));
      Serial.print("  UTC:"); Serial.println(utc_str);
      if(!utc_str.equals("not found")){
	uint32_t local = utc_str.toInt() + offset;
	Serial.print("Local:");Serial.println(local);
	ds3231_clock.set(local);
	Serial.println("local set");
      }
      Serial.println("is NTP Alive?");
      if(false && doomsday_clock.master->initialized){
	Serial.println("NTP is clock alive!");
      }
      else{
	Serial.println("No NTP, fall back to DS3231");
      }
      Serial.println();
      Serial.print("timezone_offset String:");
      Serial.println(offset_str);
      Serial.print("timezone_offset:");
      Serial.println(offset);
      //set_timezone_offset(offset, gmt);
      //my_config.last_tz_lookup = doomsday_clock.gmt();
      //saveSettings();
    }
    else{
      Serial.println("No timezone found");
      Serial.println("Payload:");
      Serial.println(payload);
    }
  }
}

void loadSettings()
{
  byte tmp;

  //targettemp = EEPROM.read(TARGETTEMP_ADDR);
  //if(targettemp == 255){
  //  targettemp = DEFAULT_TARGETTEMP;
  // }
  tmp = EEPROM.read(AWAY_ADDR);
  if(tmp == 255){
    away = false;
  }
  else{
    away = (bool)tmp;
  }
}

unsigned int last_furnace_state_change = 0;
void set_furnace(bool state){
  if(state != furnace_state &&
     ds3231_clock.now() - last_furnace_state_change > MIN_CHANGE_DURATION){
    Serial.print("Setting Furnace:");
    Serial.println(furnace_state);
    furnace_state = state;
    digitalWrite(furnaceSCR, state);
    last_furnace_state_change = ds3231_clock.now();
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
  Wire.begin(5, 4);
  Serial.begin(115200);
  setup_pins();
  setup_wifi();
   setup_temp();
  
  uint8_t mqtt_server[4] = {192, 168, 7, 159};
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(mqtt_callback);

  EEPROM.begin(512);
  loadSettings();
  setup_rtc();
  setup_display();
  if(true){ // test icons?
////////////////////////////////////////////////////////////////////////////////
  // TEST  (works !!!)
    display.clear();
    display.drawXbm(16, 16, fire_width, fire_height, fire_bits);
    display.display();
    delay(1000);

    display.clear();
    display.drawXbm(16, 16, flake_width, flake_height, flake_bits);
    display.display();
    delay(1000);

    display.clear();
    display.drawXbm(16, 16, leaf_width, leaf_height, leaf_bits);
    display.display();
    delay(1000);
////////////////////////////////////////////////////////////////////////////////
  }
  set_timezone_from_ip();
  mqtt_connect();
  ntp_clock.setup(&timeClient);
  ntp_clock.setOffset(0);
  doomsday_clock.setup(&ntp_clock, &ds3231_clock);
  
  Serial.println(ds3231_clock.now());
}

void setup_rtc(){
  //ds3231_clock.begin();
  //ds3231_clock.adjust(DateTime(2022, 2, 13, 15, 17, 0));
}

void setup_display(){
  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();
  display.display();

  displayTemp();
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

void displayTemp(){
  double theta_deg;

  int x0 = 128 - 32 - 1;
  int y0 = 32;
  int y;
  
  int theta0, theta1;
  String tempstr;
  double gain = 315 / 50.;
  
  if (get_targettemp() < LOWEST_TEMP){
    theta1 = gain * (80 - LOWEST_TEMP);
  }
  else{
    theta1 = gain * (80 - get_targettemp());
  }
  if (get_targettemp() > HIGHEST_TEMP){
    theta1 = gain * (80 - HIGHEST_TEMP);
  }
  else{
    theta1 = gain * (80 - get_targettemp());
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
  if(away){
    theta1 = theta0 + 2;
    theta0 = theta1 - 3;
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
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  if(away){
    display.setFont(ArialMT_Plain_16);
    tempstr = String("AWAY");
  }
  else{
    display.setFont(ArialMT_Plain_24);
    tempstr = String(get_targettemp());
  }
  display.drawString(x0, y0 - 12, tempstr.c_str());

  // display local temp
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  tempstr = String((int)localtemp);
  if(localtemp < get_targettemp()){
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

void publish_bool(char* topic, bool val){
  if(val){
    mqtt_client.publish(topic, "true");
  }
  else{
    mqtt_client.publish(topic, "false");
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

void mqtt_connect(){
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
      publish_int("wyostat.targettemp",    get_targettemp());
      
      Serial.print("TARGETTEMP:");
      Serial.println(get_targettemp());
    }
    else{
      Serial.print("Try again in 5 seconds.");
      delay(5000);
    }
  }
}

void mqtt_reconnect() {
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

unsigned long loop_counter = 0;
void loop() {
  float temp = localtemp;
  int sensor_temp;
  long now;
  String str_temp;
  String topic = String("wyostat.temp");

  if(loop_counter++ % 1000 == 0){
    sensor0.wakeup();
    sensor_temp =  sensor0.readTempF();
    if(0 < sensor_temp && sensor_temp < 200){
      localtemp = 0.9999 * localtemp + 0.0001 * sensor_temp;
    }
    //Serial.println(localtemp);
  }
  else{
    delay(1);
  }
  //localtemp = 73; // previous line breaks display!!
  
  // Turn sensor on to start temperature measurement.
  // Current consumtion typically ~10uA.
  /*
  */
  if (!mqtt_client.connected()) {
    mqtt_reconnect();
  }
  mqtt_client.loop();
  if(hvacmode[0] == 'h'){ // heating mode
    set_ac(false);
    if(away){
      if(localtemp < AWAYMIN - HILOGAP / 2){
	set_furnace(true);
      }
      else if(localtemp > AWAYMIN + HILOGAP / 2){
	set_furnace(false);
      }
      set_fan(furnace_state);
    }
    else{
      if (int(localtemp) > get_targettemp() - HILOGAP / 2){ // too hot
	set_furnace(false);
      }
      else if(int(localtemp) < get_targettemp() + HILOGAP / 2){ // too cold
	set_furnace(true);
      }
      else{ // just right
      }
      set_fan(furnace_state);      
    }
  }
  else if(hvacmode[0] == 'c'){ // cooling mode
    set_furnace(false);
    if(away){
      if(localtemp > AWAYMAX){
	set_fan(true);
	set_ac(true);
      }
      else if(localtemp < AWAYMAX - HILOGAP){
	set_fan(false);
	set_ac(false);
      }
    }
    else{
      if (int(localtemp) > get_targettemp()){ // too hot
	set_ac(true);
	set_fan(true);
      }
      else if (int(localtemp) <= get_targettemp() - HILOGAP){ // too cold
	set_ac(false);
	set_fan(false);
      }
      else{ // just right
      }
    }
  }
  else{
    set_ac(false);
    set_furnace(false);
    set_fan(false);
  }

  if((int)temp != int(localtemp)){
    displayTemp();
  }
  now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    if(temp != last_temp){
      last_temp = temp;
      Serial.print("temp: ");
      Serial.println(temp);
      str_temp = String(temp);
      mqtt_client.publish(topic.c_str(), str_temp.c_str());
    }
  }

  static int loop_counter = 0;
  if(loop_counter++ % 10000 == 0){
    Serial.print(ds3231_clock.hours());
    Serial.print(" ");
    Serial.print(get_targettemp());
    Serial.print(" ");
    Serial.print(targettemps[0]);
    Serial.print(" ");
    Serial.print(targettemps[1]);
    Serial.print(" ");
    if(ds3231_clock.now() - last_furnace_state_change < MIN_CHANGE_DURATION){
      Serial.print(ds3231_clock.now() - last_furnace_state_change);
    }
    Serial.println();
  }
}

void set_timezone_offset(int32_t offset, NTPClock ntp_clock){
  my_config.timezone = offset % 86400;
  saveSettings();
  if(my_config.use_wifi){
    ntp_clock.setOffset(my_config.timezone);
  }
}
