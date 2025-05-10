#include <Arduino.h>
#include "dd_tab_helpers.h"

#include "../config.h"
#include "../global.h"
#include "../slides_helpers.h"

// #endif

#define PICK_IMAGE_URL "pick://"
#define TAKE_IMAGE_URL "take://"


#ifdef UNSPLASH_CLIENT_ID
  #define UNSPLASH_GET_DOWNLOAD_IMAGE_URL_API (String("https://api.unsplash.com/photos/random/?client_id=") + UNSPLASH_CLIENT_ID)
#endif



namespace {

  GraphicalDDLayer* imageCanvas;
  // LcdDDLayer* firstButton;
  // LcdDDLayer* lastButton;
  LcdDDLayer* leftButton;
  LcdDDLayer* rightButton;
  LcdDDLayer* saveButton;
  LcdDDLayer* deleteButton;
  LcdDDLayer* fromInternetButton;
  LcdDDLayer* fromUnsplashButton;
  LcdDDLayer* fromPhoneButton;
  LcdDDLayer* fromCameraButton;
  ImageDownloadDDTunnel* imageDownloadTunnel;
  ImageRetrieverDDTunnel* imageRetrieverTunnel;
#ifdef UNSPLASH_GET_DOWNLOAD_IMAGE_URL_API  
  JsonDDTunnel *getDownloadImageUrlJsonTunnel;  
#endif

  String autoPinConfig;


  const char* slideCanvasImageName = "slide_canvas.jpg";
  const char* downloadImageName = "downloaded.jpg";

  int currentSlideIdx = -1;

  enum State {
    NOT_INITIALIZED,
    NOTHING,
    DOWNLOADING_FOR_IMAGE,
    DOWNLOADING_IMAGE_GET_URL,
    DOWNLOADING_IMAGE_FROM_URL,
    WAIT_FOR_IMAGE_DOWNLOADED,
    RETRIEVING_IMAGE,
  };
  State state;
  String stateParameters;
  unsigned long retrieveStartMillis;
  DDJpegImage retrievedJpegImage;

  const String urls[] = {
    String("https://picsum.photos/") + String(TFT_WIDTH) + String("/") + String(TFT_HEIGHT),
    String("https://loremflickr.com/") + String(TFT_WIDTH) + String("/") + String(TFT_HEIGHT),
    String("https://picsum.photos/") + String(TFT_WIDTH) + String("/") + String(TFT_HEIGHT),
    String("https://placedog.net/") +  String(TFT_WIDTH) + String("/") + String(TFT_HEIGHT) + "?r",
    String("https://picsum.photos/") + String(TFT_WIDTH) + String("/") + String(TFT_HEIGHT),
  };
  const char* getDownloadImageURL() {
    int urlCount = sizeof(urls) / sizeof(urls[0]);
    int idx = random(urlCount);
    return urls[idx].c_str();
  }
  

  // int syncLeftRightButtons() {
  //   int totalSlideCount = getSavedSlideCount();
  //   bool disableLeft = (totalSlideCount == 0 || (currentSlideIdx <= -1));  // -1 is valid, meaning none
  //   bool disableRight = (totalSlideCount == 0 || (currentSlideIdx >= (totalSlideCount - 1)));
  //   leftButton->disabled(disableLeft);
  //   rightButton->disabled(disableRight);
  //   return totalSlideCount;
  // }

  // void syncForRetrievedJpegImage() {
  //   bool disableSave = !retrievedJpegImage.isValid();
  //   saveButton->disabled(disableSave);
  //   deleteButton->disabled(!disableSave);
  // }

  int syncButtonStates() {
    int totalSlideCount = getSavedSlideCount();
    bool disableLeft = (totalSlideCount == 0 || (currentSlideIdx <= -1));  // -1 is valid, meaning none
    bool disableRight = (totalSlideCount == 0 || (currentSlideIdx >= (totalSlideCount - 1)));
    // firstButton->disabled(disableLeft);
    // lastButton->disabled(disableRight);
    leftButton->disabled(disableLeft);
    rightButton->disabled(disableRight);
    bool disableSave = !retrievedJpegImage.isValid();
    bool disableDelete = totalSlideCount == 0 || currentSlideIdx == -1 || !disableSave;
    saveButton->disabled(disableSave);
    deleteButton->disabled(disableDelete);
    if (state != NOTHING || retrievedJpegImage.isValid()) {
      imageCanvas->border(5, "darkred", "round", 2);
    } else {
      imageCanvas->border(5, "darkgreen", "round", 2);
    }
    return totalSlideCount;
  }

  void selectSlide(int slideIdx) {
    currentSlideIdx = slideIdx;
    int totalSlideCount = syncButtonStates/*syncLeftRightButtons*/();
    if (currentSlideIdx != -1) {
      imageCanvas->clear();
      // imageCanvas->setCursor(5, 100);
      // imageCanvas->setTextSize(18);
      // imageCanvas->setTextColor("yellow");
      // imageCanvas->write("loading image ...");
      imageCanvas->drawStr(5, 100, "loading image", "yellow", "", 18);
      DDJpegImage jpegImage;
      getSavedSlideImage(jpegImage, currentSlideIdx);
      if (jpegImage.isValid()) {
        imageCanvas->cacheImage(slideCanvasImageName, jpegImage.bytes, jpegImage.byteCount);
        imageCanvas->clear();
        imageCanvas->drawImageFileFit(slideCanvasImageName);
        imageCanvas->setCursor(5, 220);
        imageCanvas->setTextSize(18);
        imageCanvas->setTextColor("white", "a30%black");
        imageCanvas->write(String(currentSlideIdx + 1) + "/" + String(totalSlideCount));
      } else {
        imageCanvas->fillScreen("red");
        dumbdisplay.logToSerial("xxx unable to load image for slide " + String(currentSlideIdx));
      }
      if (ddExclusiveMode) {
        drawSlideImageToScreen(jpegImage, false);
      }
    } else {
      if (totalSlideCount == 0) {
        imageCanvas->clear();
        imageCanvas->setCursor(0, -16);
        imageCanvas->setTextSize(32);
        imageCanvas->setTextColor("red");
        imageCanvas->setHeading(45);
        imageCanvas->forward(75, false);
        imageCanvas->write("no slides", true);
      } else {
        // if has slide but none selected
        imageCanvas->fillScreen("gray");
        imageCanvas->drawStr(60, 100, "~~~ top ~~~", "darkred", "", 18);
      }
    }
    syncButtonStates();
  }
}

void setState(State newState, String newStateParameters = "") {
  state = newState;
  stateParameters = newStateParameters;
  syncButtonStates();
}

void writeErrorMessageToImageCanvas(const String& errorMessage) {
  dumbdisplay.logToSerial("xxx phone failed to download image");
  imageCanvas->clear();
  imageCanvas->setCursor(5, 100);
  imageCanvas->setTextSize(12);
  imageCanvas->setTextColor("red");
  imageCanvas->write(errorMessage);
  dumbdisplay.writeComment("XXX failed to download XXX");
}


LcdDDLayer* createButton(const char* label, int colCount = 2) {
  LcdDDLayer* button = dumbdisplay.createLcdLayer(colCount, 1);
  button->border(0.2, "darkblue");
  button->writeCenteredLine(label);
  button->enableFeedback();
  return button;

}


void dd_slides_setup(bool recreateLayers) {
  if (recreateLayers) {
    imageCanvas = dumbdisplay.createGraphicalLayer(TFT_WIDTH, TFT_HEIGHT);

    // firstButton = createButton("⏪");
    // lastButton = createButton("⏩");
    leftButton = createButton("⬅️");
    rightButton = createButton("➡️");

    // leftButton = dumbdisplay.createLcdLayer(2, 1);
    // leftButton->border(0.5, "darkblue");
    // leftButton->writeCenteredLine("⬅️");
    // leftButton->enableFeedback();

    // rightButton = dumbdisplay.createLcdLayer(2, 1);
    // rightButton->border(0.5, "darkblue");
    // rightButton->writeCenteredLine("➡️");
    // rightButton->enableFeedback();

    saveButton = createButton("💾");
    deleteButton = createButton("🗑️");

    // saveButton = dumbdisplay.createLcdLayer(2, 1);
    // saveButton->border(0.5, "darkblue");
    // saveButton->writeCenteredLine("💾");
    // saveButton->enableFeedback();

    // deleteButton = dumbdisplay.createLcdLayer(2, 1);
    // deleteButton->border(0.5, "darkblue");
    // deleteButton->writeCenteredLine("🗑️");
    // deleteButton->enableFeedback();

    fromInternetButton = createButton("🌍");
    fromUnsplashButton = createButton("💦");
    fromPhoneButton = createButton("📱");
    fromCameraButton = createButton("📷");

    // fromInternetButton = dumbdisplay.createLcdLayer(2, 1);
    // fromInternetButton->border(0.5, "darkblue");
    // fromInternetButton->writeCenteredLine("🌍");
    // fromInternetButton->enableFeedback();

    // fromPhoneButton = dumbdisplay.createLcdLayer(2, 1);
    // fromPhoneButton->border(0.5, "darkblue");
    // fromPhoneButton->writeCenteredLine("📱");
    // fromPhoneButton->enableFeedback();

#ifndef UNSPLASH_GET_DOWNLOAD_IMAGE_URL_API  
    fromUnsplashButton->disabled(true);
#endif
    if (dumbdisplay.getCompatibilityVersion() <= 12) {
      fromCameraButton->disabled(true);
    }

    autoPinConfig = DDAutoPinConfig('H')
      .beginGroup(DDAutoPinGridGroupHeader(2, 4, "", true, true))
        .addLayer(leftButton)
        .addLayer(rightButton)
        .addLayer(saveButton)
        .addLayer(deleteButton)
        .addLayer(fromInternetButton)
        .addLayer(fromUnsplashButton)
        .addLayer(fromPhoneButton)
        .addLayer(fromCameraButton)
      .endGroup()
      .addLayer(imageCanvas)
      .build();

      imageDownloadTunnel = dumbdisplay.createImageDownloadTunnel("", downloadImageName);
      imageRetrieverTunnel = dumbdisplay.createImageRetrieverTunnel();
#ifdef UNSPLASH_GET_DOWNLOAD_IMAGE_URL_API  
      getDownloadImageUrlJsonTunnel = dumbdisplay.createFilteredJsonTunnel(UNSPLASH_GET_DOWNLOAD_IMAGE_URL_API, "urls", false, 10);  
#endif
      }

  dumbdisplay.pinAutoPinLayers(autoPinConfig, PF_TAB_LEFT, PF_TAB_TOP, PF_TAB_WIDTH, PF_TAB_HEIGHT, "T");

  ddExclusiveMode = true;
  if (true) {
    setState(NOT_INITIALIZED);
  } else {
    state = NOT_INITIALIZED;
    syncButtonStates();
  }
}

bool dd_slides_loop() {
  //const DDFeedback *firstFeedback = firstButton->getFeedback();
  //const DDFeedback *lastFeedback = lastButton->getFeedback();
  const DDFeedback* leftFeedback = leftButton->getFeedback();
  const DDFeedback* rightFeedback = rightButton->getFeedback();
  const DDFeedback* saveFeedback = saveButton->getFeedback();
  const DDFeedback* deleteFeedback = deleteButton->getFeedback();
  const DDFeedback* fromInternetFeedback = fromInternetButton->getFeedback();
  const DDFeedback* fromUnsplashFeedback = fromUnsplashButton->getFeedback();
  const DDFeedback* fromPhoneFeedback = fromPhoneButton->getFeedback();
  const DDFeedback* fromCameraFeedback = fromCameraButton->getFeedback();

  if (state == NOT_INITIALIZED) {
    int savedSlideCount = getSavedSlideCount();
    int idx = currentSlideIdx != -1 ? currentSlideIdx : (savedSlideCount - 1);
    selectSlide(savedSlideCount - 1);
    if (true) {
      setState(NOTHING);
    } else {
      state = NOTHING;
      syncButtonStates();
    }
  }

  if (state == NOTHING) { 
    int savedSlideCount = getSavedSlideCount();
    bool invalidateRetrieved = false;
    if (leftFeedback != nullptr) {
      if (savedSlideCount > 0 && currentSlideIdx >= 0) {
        leftButton->flash();
        selectSlide(currentSlideIdx - 1);
        invalidateRetrieved = true;
      }
    } else if (rightFeedback != nullptr) {
      if (savedSlideCount > 0 &&  currentSlideIdx < (savedSlideCount - 1)) {
        rightButton->flash();
        selectSlide(currentSlideIdx + 1);
        invalidateRetrieved = true;
      }
    }/* else if (firstFeedback != nullptr) {
        if (savedSlideCount > 0 && currentSlideIdx >= 0) {
          firstButton->flash();
          selectSlide(-1);
          invalidateRetrieved = true;
        }
    } else if (lastFeedback != nullptr) {
        if (savedSlideCount > 0 &&  currentSlideIdx < (savedSlideCount - 1)) {
          lastButton->flash();
          selectSlide(savedSlideCount - 1);
          invalidateRetrieved = true;
        }
    }*/ else if (fromInternetFeedback != nullptr) {
      fromInternetButton->flash();
      setState(DOWNLOADING_FOR_IMAGE);
      invalidateRetrieved = true;
    } else if (fromUnsplashFeedback != nullptr) {
#ifdef UNSPLASH_GET_DOWNLOAD_IMAGE_URL_API  
      fromUnsplashButton ->flash();
      setState(DOWNLOADING_FOR_IMAGE, "unsplash");
#endif
    } else if (fromPhoneFeedback != nullptr) {
      fromInternetButton->flash();
      setState(DOWNLOADING_FOR_IMAGE, "pick");
      invalidateRetrieved = true;
    } else if (fromCameraFeedback != nullptr) {
      fromInternetButton->flash();
      setState(DOWNLOADING_FOR_IMAGE, "take");
      invalidateRetrieved = true;
    } else if (saveFeedback != nullptr) {
      if (retrievedJpegImage.isValid()) {
        saveButton->flash();
        int saveSlideIndex = currentSlideIdx + 1;
        bool saved = saveSlideImage(retrievedJpegImage, saveSlideIndex);
        if (saved) {
          invalidateRetrieved = true;
          selectSlide(saveSlideIndex);
        } else {
          dumbdisplay.log("XXX unable to save image XXX", true);
        }
      }
    } else if (deleteFeedback != nullptr) {
      if (deleteFeedback->type == DDFeedbackType::DOUBLECLICK) {
        deleteButton->flash();
        if (currentSlideIdx >= 0) {
          bool deleted = deleteSlideImage(currentSlideIdx);
          if (deleted) {
            savedSlideCount = getSavedSlideCount();
            if (currentSlideIdx >= savedSlideCount) {
              currentSlideIdx = savedSlideCount - 1;
            }
            selectSlide(currentSlideIdx);
          } else {
            dumbdisplay.log("XXX unable to delete image XXX", true);
          }
        }  
      } else {
        dumbdisplay.writeComment("!!! double tab to delete !!!");
      }
    }
    if (invalidateRetrieved) {
      if (retrievedJpegImage.isValid()) {
        retrievedJpegImage.release();
        syncButtonStates/*syncForRetrievedJpegImage*/();
      }
    }
  }

  if (state == DOWNLOADING_FOR_IMAGE) {
#ifdef UNSPLASH_GET_DOWNLOAD_IMAGE_URL_API  
    if (stateParameters == "unsplash") {
      getDownloadImageUrlJsonTunnel->reconnect();
      stateParameters += ":";
      dumbdisplay.logToSerial("*** getting unsplash image URL -- stateParameters=" + stateParameters);
    }
#endif
    setState(DOWNLOADING_IMAGE_GET_URL, stateParameters);
  }

  if (state == DOWNLOADING_IMAGE_GET_URL) {
    String url;
    bool enableCropUI = false;
    String errorMessage;
    if (stateParameters == "pick") {
      url = PICK_IMAGE_URL;
      enableCropUI = true;
    } else if (stateParameters == "take") {
      url = TAKE_IMAGE_URL;
      enableCropUI = true;
  #ifdef UNSPLASH_GET_DOWNLOAD_IMAGE_URL_API  
    } else if (stateParameters.startsWith("unsplash")) {
      if (!getDownloadImageUrlJsonTunnel->eof()) {
        String fieldId;
        String fieldValue;
        while (getDownloadImageUrlJsonTunnel->read(fieldId, fieldValue)) {
          if (false) {
            dumbdisplay.logToSerial("!!! unslash JSON response: " + fieldId + " = " + fieldValue);
          }
          if (fieldId == "urls.small") {
            if (stateParameters == "unsplash:") {
              stateParameters += fieldValue;
              if (false) {
                dumbdisplay.logToSerial("*** from JSON, got unsplash image URL ... stateParameters=" + stateParameters);
              }
            }
          }
        }
      } else {
        url = stateParameters.substring(stateParameters.indexOf(":") + 1);
        enableCropUI = true;
        dumbdisplay.logToSerial("!!! got unslash image URL: '" + url + "'");
        if (url == "") {
          errorMessage = "failed to get image URL";
          //dumbdisplay.logToSerial(String("!!! error: '") + errorMessage + "'");
        }
      }
#endif
    } else {
      url = getDownloadImageURL();
    }
    if (errorMessage != "") {
      writeErrorMessageToImageCanvas(errorMessage);
      setState(NOTHING);
    } else if (url != "") {
      if (enableCropUI) {
        url += "**CUI";
      }
      setState(DOWNLOADING_IMAGE_FROM_URL, url);
    }
  }

  if (state == DOWNLOADING_IMAGE_FROM_URL) {
    // set the URL to download web image 
    String url = stateParameters;
    String actionHeader;
    bool isForPickImage = url.startsWith(PICK_IMAGE_URL);
    bool isForTakeImage = url.startsWith(TAKE_IMAGE_URL);
    if (isForPickImage) {
      actionHeader = "picking image";
    } else if (isForPickImage) {
      actionHeader = "taking image";
    } else {
      actionHeader = "downloading image";
    }
    bool enableCropUI = false;
    if (url.endsWith("**CUI")) {
      url = url.substring(0, url.length() - 5);
      enableCropUI = true;
    }
    dumbdisplay.logToSerial("*** " + actionHeader + " with " + url);
    if (enableCropUI) {
      imageDownloadTunnel->reconnectTo(url, String(TFT_WIDTH) + "x" + String(TFT_HEIGHT));
    } else {
      imageDownloadTunnel->reconnectTo(url);
    }
    imageCanvas->clear();
    // imageCanvas->setCursor(5, 100);
    // imageCanvas->setTextSize(18);
    // imageCanvas->setTextColor("green");
    // imageCanvas->write("downloading image ...");
    imageCanvas->drawStr(5, 100, actionHeader + " ...", "green", "", 18);
    if (true) {
      setState(WAIT_FOR_IMAGE_DOWNLOADED, stateParameters);
    } else {
      state = WAIT_FOR_IMAGE_DOWNLOADED;
      syncButtonStates();
    }
  }

  if (state == WAIT_FOR_IMAGE_DOWNLOADED) {
    bool isForPickImage = stateParameters.startsWith(PICK_IMAGE_URL);
    bool isForTakeImage = stateParameters.startsWith(TAKE_IMAGE_URL);
    int result = imageDownloadTunnel->checkResult();
    if (result == 1) {
      // web image downloaded ... retrieve JPEG data of the image
      String actionHeader;
      if (isForPickImage) {
        actionHeader = "picking image";
      } else if (isForTakeImage) {
        actionHeader = "taking image";
      } else {
        actionHeader = "downloading image";
      }
        dumbdisplay.logToSerial("*** phone " + actionHeader + " ... sending to MCU");
      imageRetrieverTunnel->reconnectForJpegImage(downloadImageName, TFT_WIDTH, TFT_HEIGHT);
      imageCanvas->clear();
      imageCanvas->drawImageFileFit(downloadImageName);
      if (true) {
        imageCanvas->setCursor(5, 210);
        imageCanvas->setTextSize(22);
        imageCanvas->setTextColor("red", "a50%orange");
        imageCanvas->write("! unsaved !");
      }
      if (true) { 
        setState(RETRIEVING_IMAGE);
      } else {
        state = RETRIEVING_IMAGE;
        syncButtonStates();
      }
      retrieveStartMillis = millis();
    } else if (result == -1) {
      if (!(isForPickImage || isForTakeImage)) {
        // failed to download the image
        if (true) {
          writeErrorMessageToImageCanvas("failed to download image");
        } else {
          dumbdisplay.logToSerial("xxx phone failed to download image");
          imageCanvas->clear();
          imageCanvas->setCursor(5, 100);
          imageCanvas->setTextSize(12);
          imageCanvas->setTextColor("red");
          imageCanvas->write("!!! failed to download image !!!");
          dumbdisplay.writeComment("XXX failed to download XXX");
        }
      } else {
        selectSlide(currentSlideIdx);
      }
      if (true) {
        setState(NOTHING);
      } else {
        state = NOTHING;
        syncButtonStates();
      }
    }
  }

  if (state == RETRIEVING_IMAGE) {
    // read the retrieved image (if it is available)
    bool retrievedImage = imageRetrieverTunnel->readJpegImage(retrievedJpegImage);
    if (retrievedImage) {
      dumbdisplay.logToSerial("*** MCU received image");
      unsigned long retrieveTakenMillis = millis() - retrieveStartMillis;
      dumbdisplay.writeComment(String("* ") + retrievedJpegImage.width + "x" + retrievedJpegImage.height + " (" + String(retrievedJpegImage.byteCount / 1024.0) + " KB) in " + String(retrieveTakenMillis / 1000.0) + "s");
      if (retrievedJpegImage.isValid()) {
        if (ddExclusiveMode) {
          drawSlideImageToScreen(retrievedJpegImage, false);
        }
      } else {
        dumbdisplay.logToSerial("XXX MCU received image is invalid");
      }
      syncButtonStates/*syncForRetrievedJpegImage*/();
      if (true) {
        setState(NOTHING);
      } else {
        state = NOTHING;
      }
    }
  }


  return false;
}

void dd_slides_done() {
  ddExclusiveMode = false;
}


