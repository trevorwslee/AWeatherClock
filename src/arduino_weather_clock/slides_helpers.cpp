#include <Arduino.h>

#include <FS.h>
#include <LittleFS.h>

#include <TJpg_Decoder.h>

#include "config.h"
#include "global.h"
#include "currtime_helpers.h"
#include "png_helpers.h"
#include "screen_helpers.h"
#include "slides_helpers.h"
#include "dd_helpers.h"
#include "imgs/temp_png.h"
#include "imgs/temp.h"

#define LOG_SLIDE_INDEX false

#define INVALID_BACKGROUND_COLOR    COLOR_RED

// the following is basically copied from TJpg_Decoder example
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  // Stop further decoding as image is running off bottom of screen
  if ( y >= TFT_HEIGHT/*tft.height()*/) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
#if defined(TFT_ESPI_VERSION)
  tft.pushRect(x, y, w, h, bitmap);
#else
  drawBitmap(getDisplayHandle(), x, y, bitmap, w, h);
  //tft.drawRGBBitmap(x, y, bitmap, w, h);
#endif

  // Return 1 to decode next block
  return 1;
}


bool _initializeStorageFS = false;
int _savedImageCount = 0;
int _nextShowImageIndex = 0;
unsigned long _nextMillis = 0;


String _formatImageName(int index) {
  return "/img_" + String(index);
}
String formatImageFileName(int index) {
  return _formatImageName(index) + ".jpeg";
}
String formatImageMetaFileName(int index) {
  return _formatImageName(index) + ".txt";
}


int getSavedSlideCount() {
  if (!_initializeStorageFS) {
    return 0;
  }
  return _savedImageCount;
}
DDJpegImage& getSavedSlideImage(DDJpegImage& slideImage, int slideIndex) {
  String fileName = formatImageFileName(slideIndex);
  File f = LittleFS.open(fileName, "r");
  if (f) {
    int width = f.readStringUntil('\n').toInt();
    int height = f.readStringUntil('\n').toInt();
    int byteCount = f.readStringUntil('\n').toInt();
    uint8_t* bytes = new uint8_t[byteCount];
    f.readBytes((char*) bytes, byteCount);
    f.close();
    slideImage.width = width;
    slideImage.height = height;
    slideImage.byteCount = byteCount;
    slideImage.bytes = bytes;
  } else {
    dumbdisplay.log(String("unable to open file [") + fileName + "] for reading");
  }
  return slideImage;
}
bool saveSlideImage(DDJpegImage& slideImage, int slideIndex) {
  if (slideImage.isValid()) {
    String saveTempFileName = formatImageFileName(-1);  // index -1 means temp name
    File f = LittleFS.open(saveTempFileName, "w");
    size_t writtenByteCount = 0;
    if (f) {
      f.println(slideImage.width);
      f.println(slideImage.height);
      f.println(slideImage.byteCount);
      writtenByteCount = f.write(slideImage.bytes, slideImage.byteCount);
      f.close();
      dumbdisplay.logToSerial(String("! saved image to temp file [") + saveTempFileName + "] for index " + String(slideIndex));
    } else {
      dumbdisplay.logToSerial(String("unable to open file [") + saveTempFileName + "] for writing");
    }
    if (writtenByteCount != slideImage.byteCount) {
      dumbdisplay.logToSerial(String("XXX written byte count [") + String(writtenByteCount) + "] not equal to image byte count [" + String(slideImage.byteCount) + "]");
      if (LittleFS.exists(saveTempFileName)) {
        LittleFS.remove(saveTempFileName);
      }
      return false;
    }
    String saveFileName = formatImageFileName(slideIndex);
    for (int i = _savedImageCount; i > slideIndex; i--) {
      String oldFileName = formatImageFileName(i - 1);
      String newFileName = formatImageFileName(i);
      if (LittleFS.exists(oldFileName)) {
        LittleFS.rename(oldFileName, newFileName);
        dumbdisplay.logToSerial(String("! rename file [") + oldFileName + "] to [" + newFileName + "]");
      }
    }
    LittleFS.rename(saveTempFileName, saveFileName);
    dumbdisplay.logToSerial(String("! rename file [") + saveTempFileName + "] to [" + saveFileName + "]");
    _savedImageCount = _savedImageCount + 1;
    dumbdisplay.logToSerial(String("$ saved ") + saveFileName + " ==> savedImageCount: " + String(_savedImageCount));
    return true;
  } else {
    return false;
  }
}

bool deleteSlideImage(int slideIndex) {
  String deleteFileName = formatImageFileName(slideIndex);
  if (LittleFS.exists(deleteFileName)) {
    LittleFS.remove(deleteFileName);
    for (int i = slideIndex; i < _savedImageCount; i++) {
      String oldFileName = formatImageFileName(i + 1);
      String newFileName = formatImageFileName(i);
      if (LittleFS.exists(oldFileName)) {
        LittleFS.rename(oldFileName, newFileName);
        dumbdisplay.logToSerial(String("! rename file [") + oldFileName + "] to [" + newFileName + "]");
      }
    }
    _savedImageCount -= 1;
    dumbdisplay.logToSerial(String("$ deleted ") + deleteFileName + " ==> savedImageCount: " + String(_savedImageCount));
    return true;
  } else {
    return false;
  }

}


void drawCurrTimeOnSlide(void* displayHandle) {
  ClockCurrTime currTime = getClockCurrTime(internationalTimeFormat);
  int timeExtraXOff = 2 * TFT_X_OFF;
  int timeXOff = internationalTimeFormat ? 0 : -7;
  drawText(displayHandle, currTime.HH + ":" + currTime.mm, timeExtraXOff + timeXOff + 101, 205, COLOR_CYAN, 4, true);
  if (!internationalTimeFormat) {
    if (currTime.apm == "a") {
      drawText(displayHandle, "AM", timeExtraXOff + 219, 207, COLOR_WHITE, 1, true);
    } else if (currTime.apm == "p") {
      drawText(displayHandle, "PM", timeExtraXOff + 219, 227, COLOR_WHITE, 1, true);
    }
  }
  drawPNG(47, 205, (const uint8_t *) temp_png, TEMP_PNG_SIZE, true);
  //drawBitmap(TFT_X_OFF + 39, 205, temp565, TEMP565_WIDTH, TEMP565_HEIGHT);
  drawText(displayHandle, String(currWeatherInfo->getTempCelsius()), 17, 209, COLOR_WHITE, 3, true);
}


void drawSlideImageToScreen(DDJpegImage& jpegImage, bool drawTimeOnSlide) {
  void* displayHandle = getDisplayHandle();
  if (jpegImage.isValid()) {
    int x = (TFT_WIDTH - jpegImage.width) / 2;
    int y = (TFT_HEIGHT - jpegImage.height) / 2;
    if (x > 0 || y > 0) {
      clearScreen(displayHandle);
    }
    TJpgDec.drawJpg(x, y, jpegImage.bytes, jpegImage.byteCount);
    if (drawTimeOnSlide) {
      drawCurrTimeOnSlide(displayHandle);
    }
  } else {
    fillScreen(displayHandle, INVALID_BACKGROUND_COLOR);
  }
}


void deleteAllSavedImage() {
  for (int i = 0; i < _savedImageCount; i++) {
    String fileName = formatImageFileName(i);
    if (LittleFS.exists(fileName)) {
      LittleFS.remove(fileName);
    }
  }
  _savedImageCount = 0;
  dumbdisplay.log("!!! Deleted all saved images !!!");
}




DDJpegImage& getNextShowSavedImage(DDJpegImage& tempImage) {
  return getSavedSlideImage(tempImage, _nextShowImageIndex);
}



void slidesSetup() {
#if defined(TFT_ESPI_VERSION) || defined(FOR_TWATCH)
  TJpgDec.setSwapBytes(true);
#endif
  TJpgDec.setCallback(tft_output);
}


bool checkReadyToShowSlides() {
  if (!_initializeStorageFS) {
    _savedImageCount = 0;
    dumbdisplay.logToSerial("### Initialize STORAGE_FS");
    bool begun = false;
    if (LittleFS.begin()) {
      if (systemStartupSettingsInvalid) {
        dumbdisplay.logToSerial("### !!! system started up with invalid settings ==> reformatting storage file system !!!");
      } else {
        dumbdisplay.logToSerial("### ... existing STORAGE_FS ...");
        for (int i = 0;/* i < MAX_IMAGE_COUNT*/; i++) {
            String fileName = formatImageFileName(i);
            if (LittleFS.exists(fileName)) {
              _savedImageCount++;
            } else {
              break;
            }
        }
        begun = true;
      }
    }
    if (!begun) {
#ifdef ESP32      
      bool started = LittleFS.begin(true);
#else
      bool started = LittleFS.begin();
#endif
      if (!started) {
        dumbdisplay.logToSerial("XXX Unable to begin(), aborting\n");
        delay(2000);
        return false;
      }
      if (LittleFS.format()) {
        dumbdisplay.logToSerial("### !!! formatted STORAGE_FS !!!");
      } else {
        dumbdisplay.logToSerial("XXX Unable to format(), aborting");
        delay(2000);
        return false;
      }
    }
    dumbdisplay.logToSerial("### STORAGE_FS initialized, which has " + String(_savedImageCount) + " images");
#if defined(ESP32)    
    size_t totalBytes = LittleFS.totalBytes();
    size_t usedBytes = LittleFS.usedBytes();
    size_t freeBytes = totalBytes - usedBytes;
    dumbdisplay.logToSerial("   . totalBytes: " + String(totalBytes));
    dumbdisplay.logToSerial("   . usedBytes: " + String(usedBytes));
    dumbdisplay.logToSerial("   . freeBytes: " + String(freeBytes));
#endif    
    _initializeStorageFS = true;
  }
  return _savedImageCount > 0;
}

void slidesLoop(bool reset) {
  void* displayHandle = getDisplayHandle();
  unsigned long now = millis();
  if (reset) {
    _nextShowImageIndex = 0;
    _nextMillis = now + 1000;
    clearScreen(displayHandle);
  }
  if (now >= _nextMillis) {
    if (_savedImageCount > 0) {
      // display the "next" saved image
      DDJpegImage dummyImage;
      DDJpegImage& jpegImage = getNextShowSavedImage(dummyImage);
      if (LOG_SLIDE_INDEX) {
        dumbdisplay.logToSerial("# nextShowImageIndex: " + String(_nextShowImageIndex));
      }
      if (true) {
        drawSlideImageToScreen(jpegImage, showTimeOnSlide);
      } else {  
        if (jpegImage.isValid()) {
          int x = (TFT_WIDTH - jpegImage.width) / 2;
          int y = (TFT_HEIGHT - jpegImage.height) / 2;
          if (x > 0 || y > 0) {
            clearScreen(displayHandle);
          }
          TJpgDec.drawJpg(x, y, jpegImage.bytes, jpegImage.byteCount);
          if (showTimeOnSlide) {
            drawCurrTimeOnSlide(displayHandle);
          }
        } else {
          fillScreen(displayHandle, INVALID_BACKGROUND_COLOR);
        }
      }
      _nextShowImageIndex = (_nextShowImageIndex + 1) % _savedImageCount;
    } else {
      if (LOG_SLIDE_INDEX) {
        dumbdisplay.logToSerial("# <nothing>");
      }
    }
    _nextMillis = now + slideDurationSecs * 1000;
  }
}