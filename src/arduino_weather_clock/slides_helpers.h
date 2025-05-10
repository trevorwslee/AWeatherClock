
#ifndef SLIDE_SHOW_HELPERS_H
#define SLIDE_SHOW_HELPERS_H

#include "dumbdisplay.h"

int getSavedSlideCount();
DDJpegImage& getSavedSlideImage(DDJpegImage& tempImage, int imageIndex);
bool saveSlideImage(DDJpegImage& slideImage, int slideIndex);
bool deleteSlideImage(int slideIndex);
void drawSlideImageToScreen(DDJpegImage& jpegImage, bool drawTimeOnSlide);

void slidesSetup();
bool checkReadyToShowSlides();
void slidesLoop(bool reset);


#endif