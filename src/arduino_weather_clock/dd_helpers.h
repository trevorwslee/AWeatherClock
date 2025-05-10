#ifndef DD_HELPERS_H
#define DD_HELPERS_H

#include "dumbdisplay.h"

extern DumbDisplay dumbdisplay;


bool checkDDRenderedRedrawScreen();
bool updateKnownLocationFromDD();

void ddSetup();
bool ddLoop();

#endif