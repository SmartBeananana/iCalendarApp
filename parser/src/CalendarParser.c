#include "CalendarParser.h"
#include "HelperFunctions.h"
#include "LinkedListAPI.h"

#include <string.h>

ICalErrorCode createCalendar(char* fileName, Calendar** obj)
{
   char *retName;
   ICalErrorCode errCode = OK;
   if(fileName != NULL)
   {
      retName = strstr(fileName,".ics");
   }
   if(fileName == NULL || retName == NULL || strcmp(fileName,"")==0)
   {
      *obj = NULL;
      return INV_FILE;
   }
   FILE *fp = NULL;

   *obj = malloc(sizeof(Calendar));
   if(*obj == NULL)
   {
      return OTHER_ERROR;
   }
   (*obj)->events = NULL;
   (*obj)->properties = NULL;

   (*obj)->events = initializeList(printEvent,deleteEvent,compareEvents);
   if((*obj)->events == NULL)
   {
      free(*obj);
      *obj = NULL;
      return OTHER_ERROR;
   }
   (*obj)->properties = initializeList(printProperty,deleteProperty,compareProperties);
   if((*obj)->properties == NULL)
   {
      freeList((*obj)->events);
      free(*obj);
      *obj = NULL;
      return OTHER_ERROR;
   }

   fp = fopen(fileName,"r");

   if(fp != NULL)
   {
      char *line = NULL;
      line = readICS(fp);

      if(strEquInsens(line,"Begin:VCalendar") == 0 || strEquInsens(line,"Begin;VCalendar") == 0)
      {
         free(line);
         int countVer = 0, countProd = 0, countEvent = 0, endCheck = 0;
         while((line = readICS(fp)) != NULL)
         {
            if(line[0] != ';' && strstr(line,":") == NULL)
            {
               errCode = INV_CAL;
               free(line);
               break;
            }
            if(strCaseIncluCmp(line,"Version:") == 0 || strCaseIncluCmp(line,"Version;") == 0 )
            {
                if(strEquInsens(line,"Version:")==0 || strEquInsens(line,"Version;")==0 ||strEquInsens(line,"Version")==0)
                {
                    errCode = INV_VER;
                    free(line);
                    break;
                }
                double check = atof(line+8);
                if(check < 0.1 && check > -0.1)
                {
                   errCode = INV_VER;
                   free(line);
                   break;
                }
                (*obj)->version = atof(line+8);
                countVer++;
            }
            else if(strCaseIncluCmp(line,"Prodid:") == 0 || strCaseIncluCmp(line,"Prodid;") == 0)
            {
               if(strEquInsens(line,"Prodid:")==0 || strEquInsens(line,"Prodid;")==0 ||strEquInsens(line,"Prodid")==0)
               {
                  errCode = INV_PRODID;
                  free(line);
                  break;
               }
               strcpy((*obj)->prodID, line+7);
               countProd++;
            }
            else if(strEquInsens(line,"Begin:Vevent") == 0 || strEquInsens(line,"Begin;Vevent") == 0)
            {
               errCode = createEvent(fp,*obj);
               if(errCode != OK)
               {
                   free(line);
                   break;
               }
               countEvent++;
            }
            else if(strEquInsens(line,"END:VCALENDAR") == 0)
            {
               endCheck++;
               free(line);
               break;
            }
            else
            {
               errCode  = createProperty(line,(*obj)->properties,INV_CAL);
               if(errCode != OK)
               {
                   free(line);
                   break;
               }
            }
            if(countVer > 1)
            {
               errCode = DUP_VER;
               free(line);
               break;
            }
            if(countProd > 1)
            {
               errCode = DUP_PRODID;
               free(line);
               break;
            }
            free(line);
         }
         if(errCode == OK && (endCheck == 0 || countProd == 0 || countVer == 0 || countEvent == 0))
         {
            errCode = INV_CAL;
         }
      }
      else
      {
          errCode = INV_CAL;
          free(line);
      }
   }
   else
   {
      errCode = INV_FILE;
   }
   if(errCode != OK)    //set isErrorExits to 1 when an error occurred
   {
      freeList((*obj)->events);
      freeList((*obj)->properties);
      free(*obj);
      *obj = NULL;
      if (fp)
      {
        fclose(fp);
      }
      return errCode;
   }
   fclose(fp);
   return OK;
}

void deleteCalendar(Calendar* obj)
{
    if(obj == NULL)
    {
       return;
    }
    if(obj->events != NULL)
    {
       freeList(obj->events);
    }
    if(obj->properties != NULL)
    {
       freeList(obj->properties);
    }
    free(obj);
}

char* printCalendar(const Calendar* obj)
{
    if(obj == NULL)
    {
       return NULL;
    }
    else
    {
        char *ret = malloc(100);
        strcpy(ret, "HELLO\n");
        return ret;
    }

    printf("Version:%f\nProduct ID:%s\n",obj->version, obj->prodID);
    ListIterator itr = createIterator(obj->events);

  	Event* event;
  	while ((event = nextElement(&itr))!= NULL)
  	{
  		 char *eventStr;
       eventStr = printEvent(event);
       printf("event: %s\n",eventStr);
       free(eventStr);

  	}

    itr = createIterator(obj->properties);

  	Property* property;
  	while ((property = nextElement(&itr))!= NULL)
  	{
  		 char *propStr;
       propStr = printProperty(property);
       printf("property: .%s.\n",propStr);
       free(propStr);
  	}

    return NULL;
}

char* printError(ICalErrorCode err)
{
    char *retCode;
    char *code[] = {"OK", "INV_FILE", "INV_CAL", "INV_VER", "DUP_VER", "INV_PRODID", "DUP_PRODID",
                    "INV_EVENT", "INV_DT", "INV_ALARM", "WRITE_ERROR", "OTHER_ERROR" };
    retCode = malloc(20);
    if(retCode == NULL)
    {
        return NULL;
    }
    strcpy(retCode,code[err]);

    return retCode;
}

ICalErrorCode writeCalendar(char* fileName, const Calendar* obj)
{
    ICalErrorCode errCode = OK;
    char *retName;
    if(fileName != NULL)
    {
       retName = strstr(fileName,".ics");
    }
    if(fileName == NULL || retName == NULL || strcmp(fileName,"")==0 || obj == NULL)
    {
       printf("1/n");
       return WRITE_ERROR;
    }

    FILE *fp = NULL;
    fp = fopen(fileName,"w");
    if(fp != NULL)
    {
        fprintf(fp,"BEGIN:VCALENDAR\r\n");
        fprintf(fp,"VERSION:%.2f\r\nPRODID:%s\r\n",obj->version, obj->prodID);

        if(getLength(obj->properties) > 0)
        {
            ListIterator itrProp = createIterator(obj->properties);
            Property *property;
            while((property = nextElement(&itrProp))!=NULL)
            {
               fprintf(fp,"%s:%s\r\n",property->propName,property->propDescr);
            }
        }

        ListIterator itr = createIterator(obj->events);
        Event* event;
        while ((event = nextElement(&itr))!= NULL)
        {
           fprintf(fp,"BEGIN:VEVENT\r\n");
           fprintf(fp,"UID:%s\r\n",event->UID);
           fprintf(fp,"DTSTAMP:%sT%s",event->creationDateTime.date,event->creationDateTime.time);
           if(event->creationDateTime.UTC == true)
           {
              fprintf(fp,"Z\r\n");
           }
           else
           {
              fprintf(fp, "\r\n");
           }

           fprintf(fp,"DTSTART:%sT%s",event->startDateTime.date,event->startDateTime.time);
           if(event->startDateTime.UTC == true)
           {
              fprintf(fp,"Z\r\n");
           }
           else
           {
              fprintf(fp,"\r\n");
           }

           if(getLength(event->properties) > 0)
           {
               ListIterator itrProp = createIterator(event->properties);
               Property *property;
               while((property = nextElement(&itrProp))!= NULL)
               {
                  fprintf(fp,"%s:%s\r\n",property->propName,property->propDescr);
               }
           }

           if(getLength(event->alarms) > 0)
           {
               ListIterator itrAlarm = createIterator(event->alarms);
               Alarm *alarm;
               while((alarm = nextElement(&itrAlarm))!= NULL)
               {
                  fprintf(fp, "BEGIN:VALARM\r\n");
                  fprintf(fp,"ACTION:%s\r\n",alarm->action);
                  fprintf(fp,"TRIGGER:%s\r\n",alarm->trigger);

                  if(getLength(alarm->properties) > 0)
                  {
                      ListIterator itrProp = createIterator(alarm->properties);
                      Property *property;
                      while((property = nextElement(&itrProp))!= NULL)
                      {
                         fprintf(fp,"%s:%s\r\n",property->propName,property->propDescr);
                      }
                  }
                  fprintf(fp, "END:VALARM\r\n");
               }
            }
            fprintf(fp,"END:VEVENT\r\n");
        }
        fprintf(fp,"END:VCALENDAR\r\n");
    }
    else
    {
        printf("Error!\n");
        errCode = WRITE_ERROR;
        return errCode;
    }

    fclose(fp);
    return errCode;
}

ICalErrorCode validateCalendar(const Calendar* obj)
{
    if(obj == NULL || obj->events == NULL || !getLength(obj->events) ||  obj->properties  == NULL)
    {
        return INV_CAL;
    }
    if(strcmp(obj->prodID,"") == 0 || memchr(obj->prodID,'\0',1000) == NULL)
    {
        return INV_CAL;
    }

    if(getLength(obj->properties) > 0)
    {
        ListIterator itrProp = createIterator(obj->properties);
        Property *property;
        int countCalscale = 0, countMethod = 0;
        while((property = nextElement(&itrProp))!=NULL)
        {
            if(strcmp(property->propDescr,"") == 0)
            {
                return INV_CAL;
            }
            if(strcmp(property->propName,"CALSCALE") == 0)
            {
                countCalscale++;
            }
            else if(strcmp(property->propName,"METHOD") == 0)
            {
                countMethod++;
            }
            else
            {
                return INV_CAL;
            }
            if(countCalscale > 1 || countMethod > 1)
            {
                return INV_CAL;
            }
        }
    }
    if(getLength(obj->events) > 0)
    {
        ListIterator itrEvent = createIterator(obj->events);
        Event *event;
        int checkDtend = 0, checkDuration = 0;

        while((event = nextElement(&itrEvent)) != NULL)
        {
            if(event->properties == NULL || event->alarms == NULL)
            {
                return INV_EVENT;
            }
            if(strcmp(event->UID,"") == 0 || memchr(event->UID,'\0',1000) == NULL )
            {
                 return INV_EVENT;
            }

            ICalErrorCode errCode = OK;
            errCode = validateDateTime(event->creationDateTime);
            if(errCode != OK)
            {
                 return errCode;
            }
            errCode = validateDateTime(event->startDateTime);
            if(errCode != OK)
            {
                return errCode;
            }

            int i;
            PropertyName pNNotMoreThanOnce[100], pNMoreThanOnce[100];
            char *propNameNotMoreThanOnce[] = {"CLASS","CREATED","DESCRIPTION","GEO",
                                               "LAST-MODIFIED","LOCATION","ORGANIZER","PRIORITY",
                                               "SEQUENCE","STATUS","SUMMARY","TRANSP","URL","RECURRENCE-ID",NULL};
            char *propNameMoreThanOnce[] = {"ATTACH","ATTENDEE","CATEGORIES","COMMENT",
                                            "CONTACT","EXDATE","RELATED",
                                            "RESOURCES","RDATE",NULL};

            for(i=0; propNameNotMoreThanOnce[i] != NULL; i++)
            {
                strcpy(pNNotMoreThanOnce[i].name,propNameNotMoreThanOnce[i]);
                pNNotMoreThanOnce[i].count = 0;
            }
            for(i=0; propNameMoreThanOnce[i] != NULL; i++)
            {
                strcpy(pNMoreThanOnce[i].name,propNameMoreThanOnce[i]);
                pNMoreThanOnce[i].count = 0;
            }
//check event
            ListIterator itrProp = createIterator(event->properties);
            Property *property;

            while((property = nextElement(&itrProp)) != NULL)
            {
                int flag1 = 0, flag2 = 0;
                for(i=0; propNameNotMoreThanOnce[i] != NULL; i++)
                {
                    if(strcmp(property->propName,pNNotMoreThanOnce[i].name) == 0)
                    {
                        pNNotMoreThanOnce[i].count++;
                        if(pNNotMoreThanOnce[i].count > 1)
                        {
                            return INV_EVENT;
                        }
                        flag1 = 1;
                    }
                }
                if(flag1 == 0)
                {
                    for(i=0; propNameMoreThanOnce[i] != NULL; i++)
                    {
                        if(strcmp(property->propName,propNameMoreThanOnce[i]) == 0)
                        {
                            pNMoreThanOnce[i].count++;
                            flag2 = 1;
                        }
                    }
                    if(flag2 == 0)
                    {
                        if(strcmp(property->propName,"DTEND") == 0)
                        {
                            checkDtend++;
                            if(checkDuration != 0)
                            {
                                return INV_EVENT;
                            }
                        }
                        else if(strcmp(property->propName,"DURATION") == 0)
                        {
                            checkDuration++;
                            if(checkDtend != 0)
                            {
                                return INV_EVENT;
                            }
                        }
                        else if(strcmp(property->propName,"RRULE") != 0)
                        {
                            return INV_EVENT;
                        }
                    }
                }
            }
//check alarm
            ListIterator itrAlarm = createIterator(event->alarms);
            Alarm *alarm;
            int countTrigger = 0;
            while((alarm = nextElement(&itrAlarm)) != NULL)
            {
                if(strcmp(alarm->action,"") == 0 || memchr(alarm->action,'\0',200) == NULL)
                {
                    return INV_ALARM;
                }
                if(alarm->trigger == NULL || strcmp(alarm->trigger,"") == 0)
                {
                    return INV_ALARM;
                }
                if(strcmp(alarm->trigger,"TRIGGER") == 0)
                {
                    countTrigger++;
                }

                if(alarm->properties == NULL)
                {
                    return INV_ALARM;
                }
                ListIterator itrProp = createIterator(alarm->properties);
                Property *property;
                int countDuration = 0, countRepeat = 0, flagDuration = 0, flagRepeat = 0, countAttach = 0;
                while((property = nextElement(&itrProp)) != NULL)
                {
                    if(strcmp(property->propName,"DURATION") == 0)
                    {
                         countDuration++;
                         flagDuration = 1;
                    }
                    else if(strcmp(property->propName,"REPEAT") == 0)
                    {
                         countRepeat++;
                         flagRepeat = 1;
                    }
                    else if(strcmp(property->propName,"ATTACH") == 0)
                    {
                         countAttach++;
                    }
                    else if(strcmp(property->propName,"TRIGGER") == 0 || strcmp(property->propName,"DTSTART") == 0)
                    {
                         return INV_ALARM;
                    }

                    if(countTrigger > 1 || countDuration > 1 || countRepeat > 1 || countAttach > 1)
                    {
                         return INV_ALARM;
                    }
                }
                if(flagRepeat != flagDuration)
                {
                    return INV_ALARM;
                }
            }
        }
    }
    else
    {
        return INV_CAL;
    }
    return OK;
}

char* dtToJSON(DateTime prop)
{
    char *ret;

    ret = malloc(200);
    sprintf(ret, "{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":", prop.date, prop.time);
    if (prop.UTC)
    {
        strcat(ret, "true}");
    }
    else
    {
        strcat(ret, "false}");
     }

    return ret;
}

// char* eventToJSON(const Event* event)
// {
//     char *ret;
//
//     ret = malloc(3000*sizeof(char));
//
//     if(event ==NULL)
//     {
//         sprintf(ret,"{}");
//         free(ret);
//         return ret;
//     }
//     int numOptionalProp = 0, numAlarm = 0, summaryCheck = 0;
//     char *retDt, *retSummary;
//
//     retDt = dtToJSON(event->startDateTime);
//
//     numOptionalProp = getLength(event->properties);
//     numAlarm = getLength(event->alarms);
//
//     ListIterator itrProp = createIterator(event->properties);
//     Property *property;
//     while((property = nextElement(&itrProp))!= NULL)
//     {
//        if(strcmp(property->propName,"SUMMARY") == 0)
//        {
//            retSummary = malloc(strlen(property->propDescr)+2);
//            sprintf(retSummary,"%s",property->propDescr);
//            summaryCheck = 1;
//        }
//     }
//     if(summaryCheck == 0)
//     {
//        sprintf(ret,"{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"\"}",retDt,3+numOptionalProp,numAlarm);
//        free(retDt);
//     }
//     else if(summaryCheck == 1)
//     {
//        sprintf(ret,"{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"%s\"}",retDt,3+numOptionalProp,numAlarm,retSummary);
//        free(retSummary);
//        free(retDt);
//     }
//
//     return ret;
// }

char* eventToJSON(const Event* event)
{
    char *ret;

    ret = malloc(3000);

    if(event ==NULL)
    {
        sprintf(ret,"{}");
        free(ret);
        return ret;
    }
    int numOptionalProp = 0, numAlarm = 0, summaryCheck = 0, locationCheck = 0, orgCheck = 0;
    char *retDt, *retSummary, *retLocation, *retOrg;

    retDt = dtToJSON(event->startDateTime);

    numOptionalProp = getLength(event->properties);
    numAlarm = getLength(event->alarms);

    ListIterator itrProp = createIterator(event->properties);
    Property *property;
    while((property = nextElement(&itrProp))!= NULL)
    {
       if(strcmp(property->propName,"SUMMARY") == 0)
       {
           retSummary = malloc(strlen(property->propDescr)+2);
           sprintf(retSummary,"%s",property->propDescr);
           summaryCheck = 1;
       }
       if(strcmp(property->propName,"LOCATION") == 0)
       {
           retLocation = malloc(strlen(property->propDescr)+2);
           sprintf(retLocation,"%s",property->propDescr);
           locationCheck = 1;
       }
       if(strcmp(property->propName,"ORGANIZER") == 0)
       {
           retOrg = malloc(strlen(property->propDescr)+2);
           sprintf(retOrg,"%s",property->propDescr);
           orgCheck = 1;
       }
    }
    if(summaryCheck == 0)
    {
       retSummary = malloc(3);
       retSummary[0] = 0;
    }
    if(locationCheck == 0)
    {
       retLocation = malloc(3);
       retLocation[0] = 0;
    }
    if(orgCheck == 0)
    {
       retOrg = malloc(3);
       retOrg[0] = 0;
    }

     sprintf(ret,"{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"%s\",\"location\":\"%s\",\"organizer\":\"%s\"}",retDt,3+numOptionalProp,numAlarm,retSummary,retLocation,retOrg);
     free(retSummary);
     free(retLocation);
     free(retOrg);
     free(retDt);

    return ret;
}

char* eventListToJSON(const List* eventList)
{
    char *ret;
    int len = 0 ,count = 0;

    ret = malloc(5000*sizeof(char));

    if(eventList == NULL)
    {
        sprintf(ret,"[]");
        return ret;
    }

    len = getLength((List*)eventList);

    if(len == 0)
    {
        sprintf(ret,"[]");
        return ret;
    }
    ListIterator itr = createIterator((List*)eventList);
    Event *event;
    sprintf(ret,"[");
    while((event = nextElement(&itr)) != NULL)
    {
        char *retEvent;
        retEvent = eventToJSON(event);
        strcat(ret,retEvent);
        free(retEvent);
        count++;
        if(count < len)
        {
            strcat(ret,",");
        }
    }
    strcat(ret,"]");
    return ret;
}

char* calendarToJSON(const Calendar* cal)
{
    char *ret;
    ret = malloc(2000);
    if(cal == NULL)
    {
        sprintf(ret,"{}");
        return ret;
    }

    int numProps = 0, numEvents = 0;
    numProps = getLength(cal->properties);
    numEvents = getLength(cal->events);

    sprintf(ret,"{\"version\":%d,\"prodID\":\"%s\",\"numProps\":%d,\"numEvents\":%d}",(int)cal->version,cal->prodID,numProps+2,numEvents);
    return ret;
}

Calendar* JSONtoCalendar(const char* str)
{
    if(str == NULL)
    {
        return NULL;
    }
    Calendar *ical;
    ical = malloc(sizeof(Calendar));
    if(ical == NULL)
    {
        return NULL;
    }

    char *str1 = "{\"version\":";
    char *str2 = ",\"prodID\":";
    char *ret, temp[1000];
    ret = strstr(str,str1);
    if(ret == NULL || ret != str)
    {
        return NULL;
    }

    str = str + strlen(str1); // move the pointer to the content of version

    if(isdigit(str[0]) != 0)
    {
        ical->version = atoi(str);
    }
    str++;

    ret = strstr(str,str2);
    if(ret == NULL || ret != str)
    {
        return NULL;
    }

    ret = ret + strlen(str2) +1; // move the pointer to the content of the prodID
    strcpy(temp,ret);
    if(temp[strlen(ret)-1] != '}' && temp[strlen(ret)-2] != '"')
    {
        return NULL;
    }
    temp[strlen(ret)-2] = '\0';
    strcpy(ical->prodID,temp);

    ical->events = initializeList(printEvent,deleteEvent,compareEvents);
    if(ical->events == NULL)
    {
       free(ical);
       ical = NULL;
       return NULL;
    }
    ical->properties = initializeList(printProperty,deleteProperty,compareProperties);
    if(ical->properties == NULL)
    {
       freeList(ical->events);
       free(ical);
       ical = NULL;
       return NULL;
    }

    return ical;
}

Event* JSONtoEvent(const char* str)
{
    if(str == NULL)
    {
        return NULL;
    }

    Event *event;
    event = malloc(sizeof(Event));
    if(event == NULL)
    {
        return NULL;
    }

    char *str1 = "{\"UID\":";
    char *str2 = ",\"summary\":";
    char *ret, temp[2000];

    event->properties = initializeList(printProperty,deleteProperty,compareProperties);
    if(event->properties == NULL)
    {
        free(event);
        return NULL;
    }
    event->alarms = initializeList(printAlarm,deleteAlarm,compareAlarms);
    if(event->alarms == NULL)
    {
        freeList(event->properties);
        free(event);
        return NULL;
    }

    ret = strstr(str,str1);
    if(ret == NULL || ret != str)
    {
        return NULL;
    }
    str = str + strlen(str1)+1;//move the string pointer to "UID"
    int i = 0;
    while(str[i] != '\0' && (str[i] != '\"' && str[i+1] != ','))
    {
        event->UID[i] = str[i];
        event->UID[++i] = '\0';
    }

    if(str[i] == '\0')
    {
        return NULL;
    }
    str = str + strlen(event->UID)+1;

    if(strEquInsens(str,",\"summary\":\"\"}") != 0)
    {
        ret = strstr(str,str2);
        if(ret == NULL || ret != str)
        {
            return NULL;
        }
        str = str + strlen(str2) +1;//move str pointer to the content of 'summary'

        strcpy(temp,str);

        if(temp[strlen(str)-1] != '}' && temp[strlen(str)-2] != '"') // remove the } and " at the end of the string
        {
            return NULL;
        }

        Property *property = malloc(sizeof(Property)+(sizeof(char)*100));
        if(property == NULL)
        {
            return NULL;
        }

        temp[strlen(str)-2] = '\0';

        strcpy(property->propName,"SUMMARY");
        strcpy(property->propDescr,temp);
        insertBack(event->properties,property);
    }

    return event;
}

void addEvent(Calendar* cal, Event* toBeAdded)
{
    if(cal != NULL && toBeAdded != NULL)
    {
        insertBack(cal->events,toBeAdded);
    }
}
