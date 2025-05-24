#include <Arduino.h>

#include "config.h"
#include "screen_helpers.h"


#if defined(FOR_PYCLOCK)
  #include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
  SPIClass spi(FSPI);
  Adafruit_ST7789 tft(&spi, TFT_CS, TFT_DC, TFT_RST);
#elif defined(FOR_AST_WATCH)
  #include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
  SPIClass spi(FSPI);
  Adafruit_ST7789 tft(&spi, TFT_CS, TFT_DC, TFT_RST);
#elif defined(FOR_PICOW_GP)
  #include <Adafruit_ST7789.h>
  Adafruit_ST7789 tft(&SPI1, TFT_CS, TFT_DC, TFT_RST);
#elif defined(FOR_PICOW)
  #include <Adafruit_ST7789.h>
  Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);
#elif defined(FOR_ESP_SPARKBOT)
  #include <Adafruit_GFX.h>    // Core graphics library
  #include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
  #include <SPI.h>             // Arduino SPI library
  SPIClass spi = SPIClass(FSPI);
  Adafruit_ST7789 tft(&spi, TFT_CS, TFT_DC, TFT_RST);
#elif defined(FOR_MUMA) || defined(FOR_MUMA_NT)
  #include <Adafruit_GFX.h>    // Core graphics library
  #include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
  #include <SPI.h>             // Arduino SPI library
  SPIClass spi = SPIClass(FSPI);
  Adafruit_ST7789 tft(&spi, TFT_CS, TFT_DC, TFT_RST);
#elif defined(FOR_TWATCH)
  #include <LilyGoWatch.h>
#elif defined(FOR_ESP32_LCD)  
  #include "Adafruit_ILI9341.h"
  SPIClass spi(HSPI);
  Adafruit_ILI9341 tft(&spi, TFT_DC, TFT_CS);
#elif defined(FOR_ESP32)
  #include <Adafruit_GFX.h>    // Core graphics library
  #include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
  #include <SPI.h>             // Arduino SPI library
  SPIClass spi = SPIClass(HSPI);
  Adafruit_ST7789 tft(&spi, TFT_CS, TFT_DC, TFT_RST);
  // #include <Adafruit_ST7789.h>
  // Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);
#elif defined(FOR_ESP32_S3_EYE)
  #include <Adafruit_GFX.h>    // Core graphics library
  #include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
  #include <SPI.h>             // Arduino SPI library
  SPIClass spi = SPIClass(HSPI);
  Adafruit_ST7789 tft(&spi, TFT_CS, TFT_DC, TFT_RST);
#elif defined(FOR_ESP32_S3_BOX)
  #include "Adafruit_ILI9341.h"
  SPIClass spi(HSPI);
  Adafruit_ILI9341 tft(&spi, TFT_DC, TFT_CS);
#else
  #error board not suported
#endif


void screenSetup() {
#if defined(TFT_BL)
  pinMode(TFT_BL, OUTPUT);
  #if defined(TFT_BL_LOW)
    digitalWrite(TFT_BL, 0);  // light it up (LOW)
  #else
    digitalWrite(TFT_BL, 1);  // light it up
  #endif
#endif

#if defined(FOR_PYCLOCK)
  spi.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(240, 240, SPI_MODE0);
  tft.invertDisplay(true);
  tft.setRotation(2);
  tft.setSPISpeed(40000000);
#elif defined(FOR_AST_WATCH)  
  spi.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(240, 280, SPI_MODE0);
  tft.setRotation(3);
#elif defined(FOR_PICOW_GP)  
  // pinMode(TFT_BL, OUTPUT);
  // digitalWrite(TFT_BL, 1);  // light it up
  if (true) {
    SPI1.setSCK(TFT_SCLK);
    SPI1.setMOSI(TFT_MOSI);
  }
  tft.init(240, 240, SPI_MODE0);
  tft.setRotation(3);
#elif defined(FOR_PICOW)  
  // pinMode(TFT_BL, OUTPUT);
  // digitalWrite(TFT_BL, 1);  // light it up
  tft.init(240, 320, SPI_MODE0);
  tft.invertDisplay(false);
  tft.setRotation(1);
  tft.setSPISpeed(40000000);
#elif defined(FOR_ESP_SPARKBOT)
  // pinMode(TFT_BL, OUTPUT);
  // digitalWrite(TFT_BL, 1);  // light it up
  spi.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.setSPISpeed(40000000);
  tft.init(240, 240, SPI_MODE0);
  tft.setRotation(2);
#elif defined(FOR_MUMA) || defined(FOR_MUMA_NT)
  spi.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.setSPISpeed(40000000);
  tft.init(240, 240, SPI_MODE0);
  tft.setRotation(2);
#elif defined(FOR_ESP32_LCD)  
  spi.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.begin();
  tft.setRotation(1);
  tft.invertDisplay(true);
#elif defined(FOR_TWATCH)  
  TTGOClass *ttgo = TTGOClass::getWatch();
  ttgo->begin();
  ttgo->openBL();
  ttgo->motor_begin();
#elif defined(FOR_ESP32)  
  spi.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.setSPISpeed(40000000);
  tft.init(240, 240, SPI_MODE0);
  tft.setRotation(3);
  // tft.init(240, 240, SPI_MODE0);
  // tft.invertDisplay(false);
  // tft.setRotation(1);
  // tft.setSPISpeed(40000000);
#elif defined(FOR_ESP32_S3_EYE)
  spi.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.setSPISpeed(40000000);
  tft.init(240, 240, SPI_MODE0);
  tft.setRotation(2);
#elif defined(FOR_ESP32_S3_BOX)
  // pinMode(TFT_BL, OUTPUT);
  // digitalWrite(TFT_BL, 1);  // light it up
  spi.begin(TFT_SCLK, -1/*TFT_MISO*/, TFT_MOSI, TFT_CS);
  tft.begin();
  tft.setRotation(2);
#else
  #error board not suported
#endif  
}


void* getDisplayHandle() {
#if defined(FOR_TWATCH)  
  TTGOClass *ttgo = TTGOClass::getWatch();
  return ttgo->tft;
#else
  return &tft;
#endif
}
void clearScreen(void* displayHandle) {
#if defined(FOR_TWATCH)  
  TFT_eSPI& tft = *((TFT_eSPI *)displayHandle);
#endif
  tft.fillScreen(BACKGROUND_COLOR);
}
void fillScreen(void* displayHandle, uint16_t color) {
#if defined(FOR_TWATCH)  
  TFT_eSPI& tft = *((TFT_eSPI *)displayHandle);
#endif
  tft.fillScreen(color);
}
void drawRect(void* displayHandle, int x, int y, int w, int h, uint16_t color) {
#if defined(FOR_TWATCH)  
  TFT_eSPI& tft = *((TFT_eSPI *)displayHandle);
#endif
  tft.drawRect(x, y, w, h, color);
}
void fillRect(void* displayHandle, int x, int y, int w, int h, uint16_t color) {
#if defined(FOR_TWATCH)  
  TFT_eSPI& tft = *((TFT_eSPI *)displayHandle);
#endif
  tft.fillRect(x, y, w, h, color);
}
void drawBitmap(void* displayHandle, int x, int y, const uint16_t* bitmap, int w, int h) {
#if defined(FOR_TWATCH)  
  TFT_eSPI& tft = *((TFT_eSPI *)displayHandle);
#endif
#if defined(TFT_ESPI_VERSION)
  tft.pushRect(x, y, w, h, (uint16_t *) bitmap);
#elif defined(LGFX_VERSION_MAJOR) 
  tft.pushRect(x, y, w, h, (uint16_t *) bitmap);
#else
  tft.drawRGBBitmap(x, y, bitmap, w, h);
#endif
}
void drawText(void* displayHandle, const String& text, int x, int y, uint16_t color, uint8_t size, bool transparentBackground) {
#if defined(FOR_TWATCH)  
  TFT_eSPI& tft = *((TFT_eSPI *)displayHandle);
#endif
  tft.setCursor(x, y);
  if (transparentBackground) {
    tft.setTextColor(color);
  } else {
    tft.setTextColor(color, BACKGROUND_COLOR);
  }
  if (size > 0) {
    tft.setTextSize(size);
  }
  tft.print(text);
}

void invertDisplay(void* displayHandle, bool invert) {
#if defined(FOR_TWATCH)  
  TFT_eSPI& tft = *((TFT_eSPI *)displayHandle);
#endif
// #if !defined(FOR_ESP32_S3_BOX) && !defined(FOR_PICOW)
//     invert = !invert;  // normally inverted
// #endif
#if !defined(TFT_UN_INVERTED)
  // NOT normally un-inverted (i.e. normally inverted) ==> reverse invert
  invert = !invert;
#endif
  tft.invertDisplay(invert); 
}

