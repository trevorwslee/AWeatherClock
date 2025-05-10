#ifndef PNG_HELPERS_H
#define PNG_HELPERS_H

#include <Arduino.h>

void drawPNG(int xOff, int yOff, const uint8_t* pData, int iDataSize, bool bestErrorTransparent = false);


#endif