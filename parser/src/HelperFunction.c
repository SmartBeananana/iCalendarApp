#include "HelperFunctions.h"
#include "CalendarParser.h"
#include "LinkedListAPI.h"
#include "JSHelperFunctions.h"

char* readICS(FILE *fp)
{
    char *line;
    int c;         // EOF = INT
    int len = 0;
    int size = 100;

    if(feof(fp))
    {
        return NULL;
    }

    line = malloc(sizeof(char)*100);

    while ((c = getc(fp)) != EOF)
    {
       if(len % 99 == 0)
       {
          size += 100;
          line = realloc(line,size);
       }
       line[len++] = c;
       line[len] = '\0';

       if(len > 1 && line[len-2] == '\n')
       {
          if(line[len-1] == ' ' || line[len-1] == '\t') //folded line
          {
             len-=3;
          }
          else
          {
             fseek(fp, -1, SEEK_CUR);
             line[len-3] = '\0';
            break;
          }
       }
    }
    if(len > 2 && line[len-1]=='\n' && line[len-2]== '\r')
    {
       line[len-2] = '\0';
    }

    return line;
}

int strCaseIncluCmp(const char *str1, const char *str2)
{
    while(*str1 != '\0' && *str2 != '\0')
    {
        if(tolower(*str1) != tolower(*str2))
        {
            return 1;
        }
        str1++;
        str2++;
    }
    return 0;
}

int strEquInsens(const char *str1, const char *str2)
{
    int len1=0, len2=0;

    len1 = strlen(str1);
    len2 = strlen(str2);

    if(strCaseIncluCmp(str1,str2) == 0 && (len1 == len2))
    {
       return 0;
    }
    return 1;
}

ICalErrorCode createProperty(char *str,List *list, ICalErrorCode errCode)
{
    Property *property;
    int len, i;

    len = strcspn(str,";:");
    if (len == 0)
    {
        return (str[0] == ';') ? OK : errCode;
    }
    if (str[len+1] == '\0')
    {
        return errCode;
    }
    property = malloc(sizeof(Property)+(strlen(str+len)+1));
    if(property == NULL)
    {
        return OTHER_ERROR;
    }

    for(i=0; i<len;i++)
    {
        (property->propName)[i] = str[i];
    }
    (property->propName)[len] = '\0';
    strcpy(property->propDescr,str+len+1);
    insertBack(list,property);

    return OK;
}

ICalErrorCode createEvent(FILE *fp, Calendar* obj)
{
    if(fp == NULL)
    {
        return INV_FILE;
    }
    if(obj == NULL)
    {
        return INV_CAL;
    }

    ICalErrorCode errCode = OK;
    Event *ievent;
    int countUID = 0, countDTSTAMP = 0;

    ievent = malloc(sizeof(Event));
    if(ievent == NULL)
    {
        return OTHER_ERROR;
    }

    ievent->properties = initializeList(printProperty,deleteProperty,compareProperties);
    if(ievent->properties == NULL)
    {
        free(ievent);
        return OTHER_ERROR;
    }

    ievent->alarms = initializeList(printAlarm,deleteAlarm,compareAlarms);
    if(ievent->alarms == NULL)
    {
        freeList(ievent->properties);
        free(ievent);
        return OTHER_ERROR;
    }
    if(fp != NULL)
    {
        char *line;
        int endCheck = 0;

        while((line = readICS(fp)) != NULL)
        {
            if(line[0] != ';' && strstr(line,":") == NULL) //check not a comment nor a vaild line
            {
               errCode = INV_EVENT;
               free(line);
               break;
            }
            if(strCaseIncluCmp(line,"Uid:") == 0 || strCaseIncluCmp(line,"Uid;") == 0 )
            {
               if(strEquInsens(line,"UID;")==0 || strEquInsens(line,"UID:")==0 || strEquInsens(line,"UID")==0)
               {
                   errCode = INV_EVENT;
                   free(line);
                   break;
               }
               strcpy(ievent->UID,line+4);
               countUID++;
            }
            else if(strCaseIncluCmp(line,"DTSTAMP:") == 0 || strCaseIncluCmp(line,"DTSTAMP;") == 0)
            {
               errCode = createDateTime(line,&(ievent->creationDateTime));
               countDTSTAMP++;
            }
            else if(strCaseIncluCmp(line,"DTSTART:") == 0 || strCaseIncluCmp(line,"DTSTART;") == 0)
            {
               createDateTime(line,&(ievent->startDateTime));
            }
            else if(strCaseIncluCmp(line,"Begin:VAlarm") == 0 )
            {
                errCode = createAlarm(fp,ievent->alarms);
            }
            else if(strCaseIncluCmp(line,"END:VEVENT") == 0)
            {
               endCheck = 1;
               free(line);
               break;
            }
            else
            {
               errCode = createProperty(line,ievent->properties,INV_EVENT);
            }
            free(line);
            if(errCode != OK)
            {
               break;
            }
        }
        if(errCode == OK && (endCheck != 1 || countUID != 1 || countDTSTAMP != 1))
        {
            errCode = INV_EVENT;
        }
    }
    else
    {
       errCode = INV_FILE;
    }
    if(errCode != OK)
    {
        freeList(ievent->properties);
        freeList(ievent->alarms);
        free(ievent);
        return errCode;
    }
    insertBack(obj->events,ievent);
    return OK;
}

ICalErrorCode createDateTime(char* str, DateTime *dateTime)
{
    if(str == NULL || dateTime == NULL)
    {
       return OTHER_ERROR;
    }

    dateTime->UTC = false;
    int len,i;
    len = strcspn(str,";:");
    for(i=len+1; str[i] != 'T' && str[i] != '\0';i++)
    {
    }

    if(str[i] == '\0' || i-(len+1) != 8 || (strlen(str+i+1) != 6 && strlen(str+i+1) != 7))
    {
       return INV_DT;
    }
    for(i=0;i<8;i++)
    {
       (dateTime->date)[i] = str[i+len+1];
    }
    (dateTime->date)[8] = '\0';
    for(i=0;i<6;i++)
    {
       (dateTime->time)[i] = str[i+len+10];
    }
    (dateTime->time)[6] = '\0';
    len = strlen(str);

    if(str[len-1] == 'Z')
    {
       dateTime->UTC = true;
    }

    return OK;
}

ICalErrorCode createAlarm(FILE *fp, List *list)
{
    if(fp == NULL || list == NULL)
    {
        return OTHER_ERROR;
    }
    Alarm *alarm;

    alarm = malloc(sizeof(Alarm));

    if(alarm == NULL)
    {
        return OTHER_ERROR;
    }

    alarm->properties = initializeList(printProperty,deleteProperty,compareProperties);
    if(alarm->properties == NULL)
    {
        free(alarm);
        return OTHER_ERROR;
    }
    alarm->trigger = NULL;
    ICalErrorCode errCode = OK;
    int endCheck = 0, countTrig = 0, countAction = 0;
    if(fp != NULL)
    {
        char *line;

        while((line = readICS(fp)) != NULL)
        {
            if(line[0] != ';' && strstr(line,":") == NULL)
            {
                errCode = INV_EVENT;
                free(line);
                break;
            }
            else if(strCaseIncluCmp(line,"Action:") == 0 || strCaseIncluCmp(line,"Action;") == 0)
            {
                if(strEquInsens(line,"Action:")==0 || strEquInsens(line,"Action;")==0 || strEquInsens(line,"Action")==0)
                {
                    errCode = INV_ALARM;
                    free(line);
                    break;
                }
                strcpy(alarm->action,line+7);
                countAction++;
            }
            else if(strCaseIncluCmp(line,"Trigger:") == 0 || strCaseIncluCmp(line,"Trigger;") == 0)
            {
                if(strEquInsens(line,"Trigger:")==0 || strEquInsens(line,"Trigger;")==0 || strEquInsens(line,"Trigger")==0)
                {
                    errCode = INV_ALARM;
                    free(line);
                    break;
                }
                alarm->trigger = malloc(strlen(line));
                strcpy(alarm->trigger,line+8);
                countTrig++;
            }
            else if(strCaseIncluCmp(line,"END:VALARM") == 0)
            {
                endCheck = 1;
                free(line);
                break;
            }
            else
            {
                errCode = createProperty(line,alarm->properties,INV_ALARM);
            }
            free(line);
            if(errCode != OK)
            {
                break;
            }
        }
        if(errCode == OK && (endCheck != 1 || countTrig != 1 || countAction != 1))
        {
            errCode = INV_ALARM;
        }
    }
    else
    {
       errCode = INV_FILE;
    }
    if(errCode != OK)
    {
        if(alarm->trigger)
        {
            free(alarm->trigger);
        }
        freeList(alarm->properties);
        free(alarm);
        return errCode;
    }
    insertBack(list,alarm);
    return OK;
}

void deleteEvent(void* toBeDeleted)
{
    if(toBeDeleted == NULL)
    {
        return;
    }
    Event *event;

    event = (Event*)toBeDeleted;
    if(event->properties != NULL)
    {
        freeList(event->properties);
    }
    if(event->alarms != NULL)
    {
        freeList(event->alarms);
    }
    free(event);
}
int compareEvents(const void* first, const void* second)
{
    return 0;
}
char* printEvent(void* toBePrinted)
{
    if(toBePrinted == NULL)
    {
        return NULL;
    }
    char *ret, *retDate;
    Event *event;

    ret = malloc(sizeof(char)*1000);
    event = (Event*)toBePrinted;
    strcpy(ret,event->UID);
    sprintf(ret, "UID:%s\n", event->UID);

    retDate = printDate(&(event->creationDateTime));
    strcat(ret, "DTSTAMP:");
    strcat(ret, retDate);
    free(retDate);

    retDate = printDate(&(event->startDateTime));
    strcat(ret,"DTSTART:");
    strcat(ret,retDate);
    free(retDate);

    ListIterator itr = createIterator(event->properties);
    Property* property ;
    while ((property = nextElement(&itr))!= NULL)
    {
       char *propStr;
       propStr = printProperty(property);
       printf("event property: .%s.\n",propStr);
       free(propStr);
    }

    char *temp;
    temp = eventToJSON(event);
    printf("%s\n",temp);
    free(temp);
    return ret;
}

void deleteAlarm(void* toBeDeleted)
{
    if(toBeDeleted == NULL)
    {
        return;
    }
    Alarm *alarm;

    alarm = (Alarm*)toBeDeleted;
    if(alarm->trigger != NULL)
    {
        free(alarm->trigger);
    }
    if(alarm->properties != NULL)
    {
        freeList(alarm->properties);
    }
    free(alarm);
}
int compareAlarms(const void* first, const void* second)
{
    return 0;
}
char* printAlarm(void* toBePrinted)
{
    if(toBePrinted == NULL)
    {
        return NULL;
    }
    char *ret;
    Alarm *alarm;

    ret = malloc(200);
    alarm = (Alarm*)toBePrinted;
    strcpy(ret,alarm->action);
    return ret;
}

void deleteProperty(void* toBeDeleted)
{
     if(toBeDeleted == NULL)
     {
         return;
     }
     Property *property;

     property = (Property*)toBeDeleted;
     free(property);
}
int compareProperties(const void* first, const void* second)
{
     return 0;
}
char* printProperty(void* toBePrinted)
{
     if(toBePrinted == NULL)
     {
         return NULL;
     }
     char *ret;
     Property *property;

     ret = malloc(sizeof(char)* 200);
     property = (Property*)toBePrinted;
     sprintf(ret, "%s:", property->propName);
     strcat(ret, property->propDescr);
     return ret;
}
void deleteDate(void* toBeDeleted)
{
     if(toBeDeleted == NULL)
     {
         return;
     }
     DateTime *datetime;

     datetime = (DateTime*)toBeDeleted;
     free(datetime);
}
int compareDates(const void* first, const void* second)
{
    return 0;
}
char* printDate(void* toBePrinted)
{
    if(toBePrinted == NULL)
    {
        return NULL;
    }
    char *ret;
    DateTime *datetime;

    datetime = (DateTime*)toBePrinted;
    ret = malloc(6+1+6+6);
    sprintf(ret, "%sT%s", datetime->date, datetime->time);
    if (datetime->UTC)
    {
      strcat(ret, "Z");
    }
    strcat(ret, "\n");

    char *temp;
    temp = dtToJSON(*datetime);
    printf("%s\n",temp);
    free(temp);
    return ret;
}

ICalErrorCode validateDateTime(DateTime datetime)
{
    if(strcmp(datetime.date,"") == 0 || memchr(datetime.date,'\0',9) == NULL || strlen(datetime.date) != 8)
    {
          return INV_EVENT;
    }
    if(strcmp(datetime.time,"") == 0 || memchr(datetime.time,'\0',7) == NULL || strlen(datetime.time) != 6)
    {
          return INV_EVENT;
    }

    return OK;
}

char *alarmListToJSON(const List* alarmList)
{
    char *ret;
    int len = 0;

    ret = malloc(6000);

    if(alarmList == NULL)
    {
        sprintf(ret,"[]");
        return ret;
    }

    len = getLength((List*)alarmList);

    if(len == 0)
    {
        sprintf(ret,"[]");
        return ret;
    }
    int count=0;
    sprintf(ret,"[");
    ListIterator itrAlarm = createIterator((List*)alarmList);
    Alarm *alarm;
    while((alarm = nextElement(&itrAlarm)) != NULL)
    {
          char *retAlarm;
          retAlarm = alarmToJSON(alarm);
          strcat(ret,retAlarm);
          count++;
          if(count < len)
          {
              strcat(ret,",");
          }
          free(retAlarm);
    }

    strcat(ret,"]");
    return ret;
}

DateTime JSONtoDt(const char* str)
{
    DateTime dt;

    strcpy(dt.date,"00000000");
    strcpy(dt.time,"000000");
    dt.UTC = false;

    char *str2 = "{\"date\":";
    char *str3 = ",\"time\":";
    char *str4 = ",\"isUTC\":";
    char *ret;

    ret = strstr(str,str2);
    if(ret == NULL || ret != str)
    {
        return dt;
    }
    str = str + strlen(str2)+1;
    int i = 0;
    while(str[i] != '\0' && (str[i] != '\"' && str[i+1] != ','))
    {
        dt.date[i] = str[i];
        dt.date[++i] = '\0';
    }
    //printf("date: %s\n",dt.date);
    if(str[i] == '\0')
    {
        return dt;
    }

    str = str + strlen(dt.date)+1;
    ret = strstr(str,str3);
    if(ret == NULL || ret != str)
    {
        return dt;
    }
    str = str + strlen(str3)+1;
    i = 0;
    while(str[i] != '\0' && (str[i] != '\"' && str[i+1] != ','))
    {
        dt.time[i] = str[i];
        dt.time[++i] = '\0';
    }
    //printf("time: %s\n",dt.time);
    if(str[i] == '\0')
    {
        return dt;
    }

    str = str+strlen(dt.time)+1;
    ret = strstr(str,str4);
    if(ret == NULL || ret != str)
    {
        return dt;
    }
    str = str + strlen(str4);
    i = 0;
    ret = strstr(str,"true");

    if(ret == NULL || ret != str)
    {
        ret = strstr(str,"false");
        if(ret != NULL || ret == str)
        {
            dt.UTC = false;
        }
        else
        {
            return dt;
        }
    }
    else
    {
        dt.UTC = true;
    }
    // if(str[strlen(str)-1] != '}' && str[strlen(str)-2] != '"')
    // {
    //     return dt;
    // }

    return dt;
}

void addDTSTARTtoEvent(Event *event, DateTime dt)
{
    if(event != NULL)
    {
         strcpy(event->startDateTime.time,dt.time);
         strcpy(event->startDateTime.date,dt.date);
         if(dt.UTC)
         {
             event->startDateTime.UTC = true;
         }
         else
         {
             event->startDateTime.UTC = false;
         }
    }
}

void addDTSTAMPtoEvent(Event *event, DateTime dt)
{
    if(event != NULL)
    {
         strcpy(event->creationDateTime.time,dt.time);
         strcpy(event->creationDateTime.date,dt.date);
         if(dt.UTC)
         {
             event->creationDateTime.UTC = true;
         }
         else
         {
             event->creationDateTime.UTC = false;
         }
    }
}

void printLine(char *str)
{
    for (int i=0; str[i]!='\0';i++)
    {
       if(str[i] == '\r'||str[i] == '\n')
       {
          printf("*");
       }
       else
       {
          printf("%c",str[i]);
       }
   }
    printf("\n");
}
