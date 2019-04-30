#include "JSHelperFunctions.h"

char* alarmToJSON(const Alarm* alarm)
{
    char *ret;

    ret = malloc(200);

    if(alarm ==NULL)
    {
        sprintf(ret,"{}");
        return ret;
    }
    int numProp = 0;

    numProp = getLength(alarm->properties);

    sprintf(ret,"{\"Action\":\"%s\",\"Trigger\":\"%s\",\"numProps\":%d}",alarm->action,alarm->trigger,numProp);

    return ret;
}

char *propertyToJSON(const Property* property)
{
    char *ret;

    ret = malloc(300);

    if(property == NULL)
    {
         sprintf(ret,"{}");
         return ret;
    }
    sprintf(ret,"{\"%s\":\"%s\"}",property->propName,property->propDescr);

    return ret;
}

char *propertyListToJSON(const List* propList)
{
    char *ret;
    int len=0;

    ret = malloc(2000);

    if(propList == NULL)
    {
        sprintf(ret,"[]");
        return ret;
    }
    len = getLength((List*)propList);

    if(len == 0)
    {
        sprintf(ret,"[]");
        return ret;
    }

    int count=0;
    sprintf(ret,"[");
    ListIterator itrProp = createIterator((List*)propList);
    Property *prop;
    while((prop = nextElement(&itrProp)) != NULL)
    {
          char *retProp;
          retProp = propertyToJSON(prop);
          strcat(ret,retProp);
          count++;
          if(count < len)
          {
              strcat(ret,",");
          }
          free(retProp);
    }

    strcat(ret,"]");
    return ret;
}

char *PropToJS(char *filename, int eventNo)
{
    Calendar *ical;
    ICalErrorCode errCode;
    errCode = createCalendar(filename,&(ical));
    if(ical == NULL)
    {
        printf("ical is null %s\n",printError(errCode));
    }

    ListIterator itr = createIterator(ical->events);
    Event *event;
    char* ret;
    int counter = 0;
    while((event = nextElement(&itr)) != NULL)
    {
        counter++;
        if(counter == eventNo)
        {
            ret = propertyListToJSON(event->properties);
        }
    }
    deleteCalendar(ical);
    return ret;
}

char *AlarmToJS(char* filename, int eventNo)
{
    Calendar *ical;
    ICalErrorCode errCode;
    errCode = createCalendar(filename,&(ical));
    if(ical == NULL)
    {
        printf("ical is null %s\n",printError(errCode));
    }

    ListIterator itr = createIterator(ical->events);
    Event *event;
    char* ret;
    int counter = 0;
    while((event = nextElement(&itr)) != NULL)
    {
        counter++;
        if(counter == eventNo)
        {
            ret = alarmListToJSON(event->alarms);
        }
    }
    deleteCalendar(ical);
    return ret;
}

char* vaildateCalendarToJS(char* filename)
{
    Calendar *ical;
    ICalErrorCode errCode;
    char* ret;
    errCode = createCalendar(filename,&(ical));
    ret = printError(errCode);
    errCode = validateCalendar(ical);
    deleteCalendar(ical);
    ret = printError(errCode);
    return ret;
}

char* writeCalendarToJS(char* filename, const char* jsonCal, const char* jsonEvent, const char* jsonDTSTART, const char* jsonDTSTAMP)
{
    Calendar *ical;
    DateTime dtstart, dtstamp;
    Event *event;
    ICalErrorCode errCode;

    dtstart = JSONtoDt(jsonDTSTART);
    dtstamp = JSONtoDt(jsonDTSTAMP);

    ical = JSONtoCalendar(jsonCal);
    event = JSONtoEvent(jsonEvent);

    addDTSTARTtoEvent(event,dtstart);
    addDTSTAMPtoEvent(event,dtstamp);

    addEvent(ical,event);
    errCode = validateCalendar(ical);
    if(errCode == OK)
    {
        errCode = writeCalendar(filename,ical);
    }
    deleteCalendar(ical);
    char* ret;
    ret = printError(errCode);
    return ret;
}

char* addEventToJS(char* filename, const char*json, const char* jsonDTSTART, const char* jsonDTSTAMP)
{
    Calendar *ical;
    DateTime dtstart, dtstamp;
    Event *event;
    ICalErrorCode errCode;
    errCode = createCalendar(filename,&(ical));
    event = JSONtoEvent(json);

    dtstart = JSONtoDt(jsonDTSTART);
    dtstamp = JSONtoDt(jsonDTSTAMP);

    addDTSTARTtoEvent(event,dtstart);
    addDTSTAMPtoEvent(event,dtstamp);

    addEvent(ical,event);
    if(ical == NULL)
    {
        printf("ical is null\n");
    }
    errCode = writeCalendar(filename,ical);
    deleteCalendar(ical);
    char* ret;
    ret = printError(errCode);
    return ret;
}

char* EventListToJS(char* filename)
{
    Calendar *ical;
    ICalErrorCode errCode;
    errCode = createCalendar(filename,&(ical));
    char* ret;
    ret = eventListToJSON(ical->events);
    deleteCalendar(ical);
    return ret;
}

char* CalendarToJS(char* filename)
{
    Calendar *ical;
    ICalErrorCode errCode;
    errCode = createCalendar(filename,&(ical));
    char* ret = calloc(20000,sizeof(char));
    ret = calendarToJSON(ical);
    deleteCalendar(ical);
    return ret;
}
