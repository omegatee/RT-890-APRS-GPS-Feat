#ifndef TASK_APRS_H
#define TASK_APRS_H

#include <stdbool.h>
#include <stdint.h>
#include "radio/channels.h"

//#define ONE_MIN 56000 //56000=1min+1sec
#define ONE_MIN 55080

extern uint32_t gAPRSInterval;
extern uint32_t gAPRSCounter;

void Task_APRSBeacon(void);

#endif