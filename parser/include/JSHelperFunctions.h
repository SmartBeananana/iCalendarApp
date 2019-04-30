#ifndef JSHelperFunctions_H
#define JSHelperFunctions_H

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CalendarParser.h"
#include "HelperFunctions.h"
#include "LinkedListAPI.h"

char* alarmToJSON(const Alarm* alarm);
char *propertyToJSON(const Property* property);
char *propertyListToJSON(const List* propList);
char *PropToJS(char *filename, int eventNo);
char *AlarmToJS(char* filename, int eventNo);
char* vaildateCalendarToJS(char* filename);
char* writeCalendarToJS(char* filename, const char* jsonCal, const char* jsonEvent, const char* jsonDTSTART, const char* jsonDTSTAMP);
char* addEventToJS(char* filename, const char*json, const char* jsonDTSTART, const char* jsonDTSTAMP);
char* EventListToJS(char* filename);
char* CalendarToJS(char* filename);

#endif
