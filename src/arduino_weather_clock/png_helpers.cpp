#include <PNGdec.h>


#include "config.h"
#include "screen_helpers.h"
#include "png_helpers.h"

// #if defined(FOR_PYCLOCK) || defined(FOR_PICOW_GP)
//   #include <Adafruit_GFX.h>    // Core graphics library
//   #include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
//   extern Adafruit_ST7789 tft;
// #else
//   #error board not suported
// #endif



PNG png;


struct User {
  int xOff;
  int yOff;
  bool bestErrorTransparent;
};




// void PNGDraw(PNGDRAW *pDraw) {
//   User user = *(User *)pDraw->pUser;
//   int xOff = user.xOff;
//   int yOff = user.yOff; 
//   uint16_t usPixels[320];
//   if (false) {
//     // transparency ... just try the best ... transparent pixels in the middle will be drawn with the background color
//     png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_LITTLE_ENDIAN, /*0*/BACKGROUND_COLOR);  // IMPORTANT just some transparent will be drawn with background color
//     uint8_t ucMask[40];
//     if (png.getAlphaMask(pDraw, ucMask, 255)) { // if any pixels are opaque, draw them
//       // int x = 0;
//       // int y = pDraw->y + 24;
//       int x = xOff;
//       int y = pDraw->y + yOff;
//       uint16_t *pcolors = usPixels;
//       int width = pDraw->iWidth;
//       int maskCount = (width + 7) / 8;
//       // try to skip the consecutive transparent pixels on the left side
//       int newWidth = width;
//       for (int i = 0; i < maskCount; i++) {
//         uint8_t m = ucMask[i];
//         int transparentPixelCount = 0;
//         for (int j = 0; j < 8; j++) {
//           if ((m & 0x80) != 0) {
//             break;
//           }
//           transparentPixelCount++;
//           m <<= 1;
//         }
//         if (transparentPixelCount > 0) {
//           x += transparentPixelCount;
//           pcolors += transparentPixelCount;
//           newWidth -= transparentPixelCount;
//         }
//         if (transparentPixelCount < 8) {
//           break;
//         }
//       }
//       width = newWidth;
//       // try to skip the consecutive transparent pixels on the right side by reducing the width
//       for (int i = maskCount - 1; i >= 0; i--) {
//         uint8_t m = ucMask[i];
//         int transparentPixelCount = 0;
//         for (int j = 0; j < 8; j++) {
//           if ((m & 0x1) != 0) {
//             break;
//           }
//           transparentPixelCount++;
//           m >>= 1;
//         }
//         if (transparentPixelCount > 0) {
//           newWidth -= transparentPixelCount;
//         }
//         if (transparentPixelCount < 8) {
//           break;
//         }
//       }
//       width = newWidth;
//       if (width > 0) {
//         tft.drawRGBBitmap(x, y, pcolors, width, 1);
//       }
//     }
//   } else {
//     png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_LITTLE_ENDIAN, BACKGROUND_COLOR);
//     tft.drawRGBBitmap(xOff, pDraw->y + yOff, usPixels, pDraw->iWidth, 1);
//   }
// }

// void drawPNG(int xOff, int yOff, uint8_t *pData, int iDataSize) {
//   //int rc = png.openFLASH(pData, iDataSize, [](PNGDRAW *pDraw) { PNGDraw(pDraw, 145, 5); });
//   User user = {xOff, yOff};
//   int rc = png.openFLASH(pData, iDataSize, PNGDraw);
//   if (rc == PNG_SUCCESS) {
//     Serial.printf("!!! decode PNG image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
//     rc = png.decode(&user, 0);
//     png.close();
//   }
// }

void PNGDraw(PNGDRAW* pDraw) {
  void* displayHandle = getDisplayHandle();
  User user = *(User *)pDraw->pUser;
  int xOff = user.xOff;
  int yOff = user.yOff; 
  bool bestErrorTransparent = user.bestErrorTransparent;
  uint16_t usPixels[320];
#if defined (ESP32)   
  int iEndianness = PNG_RGB565_LITTLE_ENDIAN;
#else
  int iEndianness = PNG_RGB565_LITTLE_ENDIAN;
#endif 
  if (bestErrorTransparent) {
    // transparency ... just try the best ... transparent pixels in the middle will be drawn with the background color
    png.getLineAsRGB565(pDraw, usPixels, iEndianness, BACKGROUND_COLOR);  // IMPORTANT just some transparent will be drawn with background color
    uint8_t ucMask[40];
    if (png.getAlphaMask(pDraw, ucMask, 255)) { // if any pixels are opaque, draw them
      // int x = 0;
      // int y = pDraw->y + 24;
      int x = xOff;
      int y = pDraw->y + yOff;
      uint16_t *pcolors = usPixels;
      int width = pDraw->iWidth;
      int maskCount = (width + 7) / 8;
      // try to skip the consecutive transparent pixels on the left side
      int newWidth = width;
      for (int i = 0; i < maskCount; i++) {
        uint8_t m = ucMask[i];
        int transparentPixelCount = 0;
        for (int j = 0; j < 8; j++) {
          if ((m & 0x80) != 0) {
            break;
          }
          transparentPixelCount++;
          m <<= 1;
        }
        if (transparentPixelCount > 0) {
          x += transparentPixelCount;
          pcolors += transparentPixelCount;
          newWidth -= transparentPixelCount;
        }
        if (transparentPixelCount < 8) {
          break;
        }
      }
      width = newWidth;
      // try to skip the consecutive transparent pixels on the right side by reducing the width
      for (int i = maskCount - 1; i >= 0; i--) {
        uint8_t m = ucMask[i];
        int transparentPixelCount = 0;
        for (int j = 0; j < 8; j++) {
          if ((m & 0x1) != 0) {
            break;
          }
          transparentPixelCount++;
          m >>= 1;
        }
        if (transparentPixelCount > 0) {
          newWidth -= transparentPixelCount;
        }
        if (transparentPixelCount < 8) {
          break;
        }
      }
      width = newWidth;
      if (width > 0) {
        drawBitmap(displayHandle, x, y, pcolors, width, 1);
        //tft.drawRGBBitmap(x, y, pcolors, width, 1);
      }
    }
  } else {
    png.getLineAsRGB565(pDraw, usPixels, iEndianness, BACKGROUND_COLOR);
    drawBitmap(displayHandle, xOff, pDraw->y + yOff, usPixels, pDraw->iWidth, 1);
    //tft.drawRGBBitmap(xOff, pDraw->y + yOff, usPixels, pDraw->iWidth, 1);
  }
  // png.getLineAsRGB565(pDraw, usPixels, iEndianness, BACKGROUND_COLOR);
  // drawBitmap(xOff, pDraw->y + yOff, usPixels, pDraw->iWidth, 1);
}

void drawPNG(int xOff, int yOff, const uint8_t* pData, int iDataSize, bool bestErrorTransparent) {
  //int rc = png.openFLASH(pData, iDataSize, [](PNGDRAW *pDraw) { PNGDraw(pDraw, 145, 5); });
  User user = {xOff, yOff, bestErrorTransparent};
  int rc = png.openFLASH((uint8_t *) pData, iDataSize, PNGDraw);
  if (rc == PNG_SUCCESS) {
    if (false) {
      Serial.printf("!!! decode PNG image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
    }
    rc = png.decode(&user, 0);
    png.close();
  }
}
