#define leaf_width 32
#define leaf_height 32
const char leaf_bits[] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x06,0x00,0x00,0x80,0x07,0x00,0x00,
0xf0,0x0f,0x00,0x00,0xff,0x0e,0x00,0xe0,0xdf,0x0f,0x00,0xf8,0xfb,0x0e,0x00,0xfe,0xbe,0x0b,0x00,0xdf,0xb7,0x1f,0x00,0xf7,0xdd,0x0d,
0x80,0xbf,0xc7,0x0f,0x80,0xed,0xf1,0x0e,0xc0,0x7f,0xf8,0x07,0xc0,0x0f,0xbe,0x07,0x80,0x83,0xff,0x06,0xc0,0xe0,0xf7,0x03,0x80,0xf8,
0xbd,0x03,0x00,0x7c,0xff,0x01,0x00,0xff,0xef,0x00,0x00,0x6f,0x7b,0x00,0x80,0xff,0x1f,0x00,0xc0,0xd1,0x05,0x00,0xe0,0x00,0x00,0x00,
0x60,0x00,0x00,0x00,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,};


#define flake_width 32
#define flake_height 32
const char flake_bits[] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x01,0x00,0x00,0x80,0x01,0x00,0x00,0x80,0x01,0x00,0x00,0xb0,
0x0f,0x00,0x00,0xf0,0x0f,0x00,0x00,0xc3,0xc3,0x00,0x30,0x82,0x41,0x0c,0xf0,0x83,0xe1,0x0f,0xe0,0xf3,0xc7,0x03,0x80,0xe7,0xf7,0x01,
0xc0,0xff,0xff,0x07,0xe0,0x7c,0x1e,0x06,0x00,0x38,0x1c,0x00,0x00,0x3c,0x3c,0x00,0x20,0x78,0x1e,0x02,0xe0,0xfa,0xbe,0x07,0xc0,0xff,
0xff,0x01,0x80,0xe7,0xe7,0x03,0xe0,0x93,0xc5,0x07,0x70,0x83,0xc1,0x0e,0x30,0xc3,0x41,0x0c,0x00,0xe2,0xc7,0x00,0x00,0xf0,0x0f,0x00,
0x00,0x90,0x09,0x00,0x00,0x80,0x01,0x00,0x00,0x80,0x01,0x00,0x00,0x80,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,};

#define fire_width 32
#define fire_height 32
const char fire_bits[] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0xc0,0x01,0x00,0x00,0xc0,0x07,0x00,0x00,0x60,0x0e,0x00,0x00,0x60,
0x1c,0x00,0x00,0x30,0x30,0x00,0x00,0x30,0x60,0x00,0x00,0x18,0xe0,0x00,0x00,0x0c,0x82,0x01,0x00,0x0e,0x83,0x03,0x00,0x03,0x0e,0x07,
0x00,0x03,0x0f,0x06,0x80,0x01,0x1a,0x0c,0xc0,0x00,0x3a,0x0c,0x60,0x10,0x23,0x18,0x60,0x38,0x62,0x18,0x60,0xf8,0x63,0x18,0x30,0xec,
0x61,0x18,0x20,0x0c,0x60,0x10,0x30,0x0c,0x60,0x18,0x20,0x0c,0x30,0x18,0x60,0x18,0x30,0x18,0x60,0x38,0x18,0x0c,0x60,0x20,0x10,0x0e,
0xc0,0x01,0x00,0x06,0x80,0x03,0x80,0x03,0x00,0x2f,0xc0,0x01,0x00,0x3c,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,};

#define fan_width 32
#define fan_height 32
const char fan_bits[] = {
0x00,0x00,0x00,0x00,0x40,0x81,0x4e,0x01,0xf0,0x03,0xf0,0x07,0xf8,0x07,0xf0,0x0f,0xfc,0x8f,0xfa,0x1f,0xfc,0x0f,0xfc,0x1f,0xfc,0x1f,
0xf8,0x3f,0xfe,0x1f,0xfc,0x1f,0xfc,0x3f,0xfc,0x1f,0xfc,0x1f,0xfc,0x1f,0xfc,0x47,0xf1,0x0f,0xf2,0xe2,0xf3,0x07,0xa2,0xf3,0xe7,0x01,
0x02,0xfa,0x4f,0x00,0x10,0x38,0x0e,0x00,0x02,0x38,0x0e,0x40,0x00,0x3c,0x0e,0x04,0x00,0xb9,0x4e,0x20,0xc0,0xf3,0x67,0x22,0xf0,0xf3,
0xe7,0x27,0xf8,0x47,0xf1,0x1f,0xfc,0x0f,0xf8,0x1f,0xfc,0x1f,0xfe,0x1f,0xfc,0x1f,0xfe,0x1f,0xfe,0x0f,0xfc,0x3f,0xfc,0x1f,0xfc,0x1f,
0xfc,0x2f,0xf8,0x1f,0xfc,0x85,0xf8,0x0b,0xf0,0x07,0xe0,0x0f,0xd0,0x1e,0xa0,0x03,0x00,0xa0,0x00,0x00,0x00,0x00,0x00,0x00,};

