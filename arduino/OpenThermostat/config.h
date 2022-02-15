#ifndef _CONFIG_H
#define _CONFIG_H

//#include <credentials.h>

#define ENTER 17
#define INC 5
#define DECR 18
#define MODE 19

/* added to config struct */
//bool FLIP_DISPLAY = true; // true: cord out bottom, false: cord out top
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

#endif
