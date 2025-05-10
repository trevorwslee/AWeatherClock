#ifndef DISPLAY_HELPERS_H
#define DISPLAY_HELPERS_H

#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_YELLOW  0xFFE0
#define COLOR_ORANGE  0xFC00

#define BACKGROUND_COLOR      COLOR_BLACK
#define CONTROL_FONT_COLOR    COLOR_WHITE
#define CONTROL_FONT_SIZE               2



void screenSetup();

void* getDisplayHandle();

void clearScreen(void* displayHandle);
void fillScreen(void* displayHandle, uint16_t color);
void drawRect(void* displayHandle, int x, int y, int w, int h, uint16_t color);
void fillRect(void* displayHandle, int x, int y, int w, int h, uint16_t color);
void drawBitmap(void* displayHandle, int x, int y, const uint16_t* bitmap, int w, int h);
void drawText(void* displayHandle, const String& text, int x, int y, uint16_t color, uint8_t size, bool transparentBackground = false);
//void turnDisplayOnOff(void* displayHandle, bool turnOn);
  // uint8_t getDisplayRotation(void* displayHandle);
// void setDisplayRotation(void* displayHandle, uint8_t rotation);
void invertDisplay(void* displayHandle, bool invert);

#endif