#ifndef TRIGGER_HELPERS_H
#define TRIGGER_HELPERS_H

enum TriggeredState { TS_NONE, TS_IP, TS_SLIDES, TS_ALARM };

TriggeredState getTriggeredState();
void setTriggeredState(TriggeredState triggeredState);

void triggerSetup();
void triggerLoop();

#endif