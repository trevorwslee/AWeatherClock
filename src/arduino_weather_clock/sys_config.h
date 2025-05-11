#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

#include <Arduino.h>


// the version of the sketch
#define ARDUINO_WEATHER_CLOCK_VERSION "1.0"


// comment out to DEBUG use of WIFI MANAGER for WiFi SSID / password
//#define TEST_WIFI_MANAGER

// comment out if you want the program to delay startup for 10 seconds for debugging (examine the serial monitor output)
//#define DELAY_INITIALIZE_FOR_SECONDS 10

// suggested to set the following EEPROM_HEADER to the date you want to reset the saved program settings *** INCLUDING saved slides ***
const int32_t EEPROM_HEADER = 20250505;


// DD_SHOW_IP_INTERVAL_SECONDS MUST be defined before
#define DD_SHOW_IP_INTERVAL_SECONDS 60


#if defined(FOR_PYCLOCK)
  #define TFT_CS      5
  #define TFT_DC      4
  #define TFT_SCLK    6
  #define TFT_MOSI    7  // SDA
  #define TFT_RST     8
  #define TFT_X_OFF   0
  #define BUTTON_PIN  9
#elif defined(FOR_AST_WATCH)
  #define TFT_CS      3
  #define TFT_DC      2
  #define TFT_SCLK    5
  #define TFT_MOSI    6  // SDA
  #define TFT_RST     8
  #define TFT_X_OFF   20
  #define CST_TP_BUS_NUM  0
  #define CST_TP_SCL      7
  #define CST_TP_SDA      11
  #define CST_TP_RST      10
  #define CST_TP_INT      9
  #define BUZZER_PIN      1
#elif defined(FOR_PICOW_GP)
  #define TFT_BL          7
  #define TFT_CS          9
  #define TFT_DC          8
  #define TFT_SCLK        10
  #define TFT_MOSI        11
  #define TFT_RST         12
  #define TFT_X_OFF       0
  #define BUTTON_PIN          27  /* start:27|select:26|L-D-U-R:13-14-16-17|X-A-Y-B:2-3-4-6|LEFT:18|RIGHT:1 */
#elif defined(FOR_PICOW)
  #define TFT_BL      21
  #define TFT_CS      17
  #define TFT_DC      16
  #define TFT_SCLK    18
  #define TFT_MOSI    19
  #define TFT_RST     20
  #define TFT_X_OFF   40
  #define TFT_UN_INVERTED
  #define FT_TP_SCL   5 
  #define FT_TP_SDA   4
  #define FT_TP_INT   6
  #define FT_TP_RST   7
  #define BUZZER_PIN  15
#elif defined(FOR_ESP_SPARKBOT)
  #define TFT_BL          46
  #define TFT_CS          44
  #define TFT_DC          43
  #define TFT_SCLK        21
  #define TFT_MOSI        47
  #define TFT_RST         -1
  #define TFT_X_OFF       0
  #define TOUCH_PIN            3 
  #define TOUCH_THRESHOLD      512 /*90*/
  #define MIN_TOUCH_VALUE      33000
  // #define TOUCH_PIN            1 /* 1: back of top-right-side */
  // #define TOUCH_THRESHOLD      512
  // #define MIN_TOUCH_VALUE      37000
  #define ES8311_PA             46 
  //#define ES8311_I2C_PORT       0x30
  #define ES8311_I2C_SCL        5
  #define ES8311_I2C_SDA        4
  #define ES8311_I2S_PORT       35
  #define ES8311_I2S_MCLK       45
  #define ES8311_I2S_BCK        39  
  #define ES8311_I2S_WS         41
  #define ES8311_I2S_DOUT       42
  #define ES8311_I2S_DIN        40
  #define ES8311_VOLUME         60
#elif defined(FOR_TWATCH)
  #define TFT_X_OFF   0
#elif defined(FOR_ESP32_S3_EYE)
  #define TFT_CS      44
  #define TFT_DC      43
  #define TFT_SCLK    21
  #define TFT_MOSI    47
  #define TFT_RST     -1
  #define TFT_X_OFF   0
  #define BUTTON_PIN  1
#elif defined(FOR_ESP32_S3_BOX)
  #define TFT_BL      47
  #define TFT_CS      5
  #define TFT_DC      4   
  #define TFT_SCLK    7
  #define TFT_MOSI    6    // SDA
  #define TFT_RST     48
  #define TFT_X_OFF   40
  #define TFT_UN_INVERTED
  #define ES8311_PA       46 
  #define ES8311_I2C_SCL  18
  #define ES8311_I2C_SDA  8
  #define ES8311_I2S_PORT 35
  #define ES8311_I2S_MCLK 2
  #define ES8311_I2S_BCK  17
  #define ES8311_I2S_WS   45
  #define ES8311_I2S_DOUT 15
  #define ES8311_I2S_DIN  16
  #define ES8311_VOLUME   50
  #define BUTTON_PIN  0    // boot
  // #define GT911_TP_SCL       18  // TODO: not working very well ... sometimes not working
  // #define GT911_TP_SDA       8
  // #define GT911_TP_INT       3
  // #define GT911_TP_RST       -1
  // #define GT911_TP_WIDTH     320
  // #define GT911_TP_HEIGHT    240
#else
  #error board not suported
#endif


#define TFT_WIDTH   (240 + 2 * TFT_X_OFF)
#define TFT_HEIGHT  240


#endif