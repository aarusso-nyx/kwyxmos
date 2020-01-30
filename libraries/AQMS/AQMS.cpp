#include <AQMS.h>

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
static callback_t callback  = NULL;
static AlarmItem *alarmList = NULL;


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
static size_t snprintfAlarm ( char *out, const size_t &len, const AlarmItem *item) {
    if ( out == NULL || len == 0 || item == NULL ) {
        return 0;
    }
    
    const char *tPower[] = { "ON", "OFF", "TOGGLE" };
    const char *tDoW[] = {"---", "sun", "mon", "tue", "wed", "thu", "fri", "sat"};
    const char *tModes[] = { 
        "timerUndefined",
        "timerOnce", 
        "timerRepeat", 
        "alarmOnceDaily", 
        "alarmRepeatDaily", 
        "alarmOnceWeekly", 
        "alarmRepeatWeekly",
        "triggerOnce" };
    
    byte *dhms = (byte *) &item->param;
    
    char sTime[len];
    switch(item->tmode) {
      case ALARM_TRIGGER_ONCE:
          ctime_r(&item->param, sTime);
          break;
      case ALARM_ONCE:
          sprintf(sTime,"start in %lds", item->param);
          break;
      case ALARM_REPEAT:
          sprintf(sTime,"trigger every %lds", item->param);
          break;
      case ALARM_DAILY_ONCE:
          sprintf(sTime,"start at next %02dh%02dm%02ds", dhms[2], dhms[1], dhms[0]);
          break;
      case ALARM_DAILY_REPEAT:
          sprintf(sTime,"trigger every day at %02dh%02dm%02ds", dhms[2], dhms[1], dhms[0]);
          break;
      case ALARM_WEEKLY_ONCE:
          sprintf(sTime,"start at next %s, %02dh%02dm%02ds", tDoW[dhms[3]], dhms[2], dhms[1], dhms[0]);
          break;
      case ALARM_WEELY_REPEAT:
          sprintf(sTime,"trigger every %s at %02dh%02dm%02ds", tDoW[dhms[3]], dhms[2], dhms[1], dhms[0]);
          break;
    }
    
    return snprintf(out, len, "AlarmId: %d, Relay: #%d %s, tMode: %s set to %s", 
    item->alarm, item->relay, tPower[item->state], tModes[item->tmode], sTime);
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
static void dropAlarm ( AlarmItem *item, AlarmItem *last ) {
    if ( !item ) {
        return;
    }
    
    if ( last == NULL ) {
        alarmList = item->next; 
    } else {
        last->next = item->next;
    }

    Serial.print("[A] Dropping AlarmId: ");
    Serial.println(item->alarm);

    Alarm.free(item->alarm);
    free(item);    
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
static void pollAlarms() {
    for ( AlarmItem *last=NULL, *item=alarmList; item; last=item, item=item->next ) {
        if ( item->alarm == Alarm.getTriggeredAlarmId() ) {
            if ( callback ) {
                callback(item->relay, item->state);
            }
            
            // All  *_ONCE are mapped to odd values on AlarmModes
            if ( item->tmode % 2) {
                dropAlarm(item, last);
            }
            
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void setAlarmCallback(callback_t cb) {
    callback = cb;
}


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
byte nextAlarm( const byte &alarm ) {
    if ( alarm == dtINVALID_ALARM_ID ) {
        return alarmList ? alarmList->alarm : dtINVALID_ALARM_ID;
    } else {
        for ( AlarmItem *item=alarmList; item; item=item->next ) {
            if ( item->alarm == alarm ) {
                return item->next ? item->next->alarm : dtINVALID_ALARM_ID;
            }
        }        
    }
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void clearAlarms() {
    while (alarmList) {
        dropAlarm(alarmList, NULL);
    }
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
size_t readAlarm ( const byte alarm, char *out, const size_t size ) {
    for ( AlarmItem *item=alarmList; item; item=item->next ) {
        if ( item->alarm == alarm ) {
            return snprintfAlarm(out, size, item);
        }
    }
};

size_t rawAlarm ( const byte alarm, char *out, const size_t size ) {
    for ( AlarmItem *item=alarmList; item; item=item->next ) {
        if ( item->alarm == alarm ) {
            return snprintf(out, size, "%d,%d,%d,%lX", 
                item->relay, item->state, item->tmode, item->param);
        }
    }
};

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
int setAlarm ( const char *value, byte *alarm ) {
    time_t param;
    byte relay, state, tmode;
    byte *dhms = (byte *) &param;
    
    // parse input 
    sscanf ( value, "%hhu,%hhu,%hhu,%lx", &relay, &state, &tmode, (unsigned long *)&param);
    
    // set alarm default value
    *alarm = dtINVALID_ALARM_ID;

    // check if Alarm already exists
    for ( AlarmItem *last=NULL, *item=alarmList; item; last=item, item=item->next ) {
        if (item->relay == relay && item->state == state && item->param == param) {

            *alarm = item->alarm;
            
            if ( tmode == ALARM_UNDEFINED ) {
                dropAlarm(item, last);  // delete item 
                return ALARM_CLEARED;               // Cleared 
            } else {
                return ALARM_IGNORED;               // Repeated
            }
        }
    }
    
    if ( relay < 0 || relay >= 8 ) {
        return ALARM_INVALID_RELAY;
    }

    if ( !(state == 0 || state == 1 || state == 2) ) {
        return ALARM_INVALID_STATE;
    }

    
    switch(tmode) {
        case ALARM_TRIGGER_ONCE:
          *alarm = Alarm.triggerOnce(param, pollAlarms);
          break;
        case ALARM_ONCE:
          *alarm = Alarm.timerOnce(param, pollAlarms);
          break;
        case ALARM_REPEAT:
          *alarm = Alarm.timerRepeat(param, pollAlarms);
          break;
        case ALARM_DAILY_ONCE:
          *alarm = Alarm.alarmOnce(dhms[2], dhms[1], dhms[0], pollAlarms);
          break;
        case ALARM_DAILY_REPEAT:
          *alarm = Alarm.alarmRepeat(dhms[2], dhms[1], dhms[0], pollAlarms);
          break;
        case ALARM_WEEKLY_ONCE:
          *alarm = Alarm.alarmOnce((timeDayOfWeek_t)dhms[3], dhms[2], dhms[1], dhms[0], pollAlarms);
          break;
        case ALARM_WEELY_REPEAT:
          *alarm = Alarm.alarmRepeat((timeDayOfWeek_t)dhms[3], dhms[2], dhms[1], dhms[0], pollAlarms);
          break;
            
        default:
            return ALARM_INVALID_TMODE;
    }

    if ( *alarm == dtINVALID_ALARM_ID) {
        return ALARM_INVALID_PARAM;       // Fail to parse data
    }

    // Create a new Alarm 
    AlarmItem *item = (AlarmItem *) calloc ( 1, sizeof(AlarmItem) );
    
    item->relay = relay;
    item->state = state;
    item->tmode = tmode;
    item->param = param;
    item->alarm =*alarm;
    item->next  = alarmList;
    alarmList = item;

    return ALARM_CREATED;
}
