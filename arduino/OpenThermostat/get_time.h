#ifndef GET_TIME_H
#define GET_TIME_H

#include "TimeLib.h"
#include <DNSServer.h>
#include <stdint.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <RTClib.h>
#include <EEPROM.h>

void saveSettings();

class Clock{
 public:
  Clock();
  virtual uint32_t now();
  bool isCurrent();
  virtual bool set(uint32_t _t);
  int year();
  int month();
  int day();
  int hours();
  int minutes();
  int seconds();
};

class DummyClock : public Clock{
 public:
  DummyClock();
  bool set(uint32_t _t);
  uint32_t now();
};

class NTPClock : public Clock{
 public:
  bool initialized = false;
  NTPClock();
  uint32_t offset_seconds;
  bool set(uint32_t _t);  
  bool isCurrent();
  void setup(NTPClient *_timeClient);
  void setOffset(int32_t offset_seconds);
  NTPClient *timeClient;
  uint32_t now();
  uint32_t gmt();
  bool update();
};

/*
void set_timezone_offset(int32_t offset, NTPClock ntp_clock){
  my_config.timezone = offset % 86400;
  saveSettings();
  if(my_config.use_wifi){
    ntp_clock.setOffset(my_config.timezone);
  }
}
*/

class DS3231Clock : public Clock{
 public:
  RTC_DS3231 rtc;
  DS3231Clock();
  void setup();
  uint32_t now();
  bool set(uint32_t t);
};

class DoomsdayClock : public Clock{
 public:
  NTPClock *master;
  Clock *backup;

  DoomsdayClock();
  bool set(uint32_t _t);
  void setup(NTPClock* _master, Clock* _backup);
  uint32_t now();
  uint32_t gmt();
};

#endif
