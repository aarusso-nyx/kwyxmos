#ifndef __AQMS__
#define __AQMS__

#define dtNBR_ALARMS 128;
#include <TimeAlarms.h>


typedef enum { 
    ALARM_CREATED = 0,
    ALARM_CLEARED, 
    ALARM_IGNORED,
    ALARM_INVALID_PARAM,
    ALARM_INVALID_RELAY,
    ALARM_INVALID_STATE,
    ALARM_INVALID_TMODE
} AlarmRet; 


typedef enum {
    ALARM_UNDEFINED    = 0,
    ALARM_ONCE         = 1,  
    ALARM_REPEAT       = 2,  
    ALARM_DAILY_ONCE   = 3,  
    ALARM_DAILY_REPEAT = 4,  
    ALARM_WEEKLY_ONCE  = 5,  
    ALARM_WEELY_REPEAT = 6,    
    ALARM_TRIGGER_ONCE = 7
} AlarmModes;


typedef struct _alarmItem {
    byte alarm;
    byte relay;
    byte state;
    byte tmode;
    time_t param;
    struct _alarmItem *next;
} AlarmItem;


typedef void (*callback_t)(const int, const int);


void setAlarmCallback(callback_t cb);
void clearAlarms();

byte nextAlarm ( const byte &b = dtINVALID_ALARM_ID );

size_t rawAlarm ( const byte alarm, char *out, const size_t size );
size_t readAlarm ( const byte alarm, char *out, const size_t size );
int setAlarm ( const char *value, byte *alarm );

#endif