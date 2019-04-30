#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CalendarParser.h"
#include "LinkedListAPI.h"

typedef struct {
    char name[100];
    int count;
}PropertyName;

char* readICS(FILE *fp);
int strCaseIncluCmp(const char *str1, const char *str2);
int strEquInsens(const char *str1, const char *str2);
ICalErrorCode createProperty(char *str,List *list, ICalErrorCode errCode );
ICalErrorCode createEvent(FILE *fp, Calendar* obj);
ICalErrorCode createDateTime(char *str, DateTime *dateTime);
ICalErrorCode createAlarm(FILE *fp, List *list);
ICalErrorCode validateDateTime(DateTime datetime);
char *alarmListToJSON(const List* alarmList);
DateTime JSONtoDt(const char* str);
void addDTSTARTtoEvent(Event *event, DateTime dt);
void addDTSTAMPtoEvent(Event *event, DateTime dt);
void printLine(char *str);

#endif
