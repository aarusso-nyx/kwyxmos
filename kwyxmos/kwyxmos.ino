#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <Wire.h>
#include <RTClib.h>
#include <NTPClient.h>
#include <TimeLib.h>

#include <PubSubClient.h>
#include <RelayBoard.h>
#include <DHT.h>

#include <AQMS.h>
#include <mdnsResolver.h>

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// Tiny RTC DS 1307
RTC_DS1307 RTC;

// MEASURE 3.3V FROM POWER
ADC_MODE(ADC_VCC);

// Initialize DHT sensor on A5
DHT dht(12, DHT22);

// Relay Board
const int numRelays = 8;
RelayBoard Relay(14, 2, 0, 1);
byte RelayState[numRelays];

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
WiFiClient espClient;

// MTQQ Client
PubSubClient mqtt(espClient);
AlarmId sensorTimer, blinkTimer, pingTimer;

// NTP client
NTPClient ntp(udp, "br.pool.ntp.org", 3600, 60000);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
enum { VERBOSE_QUIET=0, VERBOSE_INFO=1, VERBOSE_DEBUG=2 };
const char *sVerbose[] = { "QUIET", "INFO", "DEBUG" };
const char *sStates[] = { "OFF", "ON", "TOGGLE" };
const int LINE_BUFFER_SIZE = 128;


int bVerb = 0;
int bPing = 0;

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
void listRelays( const bool &bPublish ) {
    for ( int i=0; i<numRelays; i++ ) {
        printf ("[*] Relay #%d set %s\n", i, sStates[RelayState[i]]);
        if ( bPublish && mqtt.connected() ) {
            char topic[LINE_BUFFER_SIZE], value[LINE_BUFFER_SIZE];
            sprintf(topic, "relay/%d/state", i);
            sprintf(value, "%d", RelayState[i]);
            mqtt.publish(topic,value);
        }  
    }
}

void setRelay ( const int relay, const int state ) {
    if ( relay < 0 || relay >= numRelays ) {
        return;
    }
    
    if ( state == 0 || state == 1 ) {
        RelayState[relay] = state;
    } else if ( state == 2 ) {
        RelayState[relay] = !RelayState[relay];
    }
    
    Relay.set(0, relay, RelayState[relay]);
    if ( mqtt.connected() ) {
        char topic[LINE_BUFFER_SIZE], value[LINE_BUFFER_SIZE];
        sprintf(topic, "relay/%d/state", relay);
        sprintf(value, "%d", RelayState[relay]);
        mqtt.publish(topic,value,true);
    }  

    if ( bVerb > VERBOSE_INFO ) {
        printf ("[>] Relay #%d set %s\n", relay, sStates[state]);
    }
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
void listAlarms( const bool &bPublish ) {
    for ( byte a=nextAlarm(); a != dtINVALID_ALARM_ID; a=nextAlarm(a) ) {
        char sLine[LINE_BUFFER_SIZE];
        readAlarm(a, sLine, sizeof(sLine));
        printf("[*] %s\n", sLine);
        if ( bPublish && mqtt.connected() ) {
            char topic[LINE_BUFFER_SIZE];
            sprintf(topic, "alarm/%d/pretty", a);
            mqtt.publish(topic,sLine);

            rawAlarm(a, sLine, sizeof(sLine));
            sprintf(topic, "alarm/%d/state", a);
            mqtt.publish(topic,sLine);
        }       
    }
}

void postAlarm(const char *value) {
    byte alarm;
    
    int fail = setAlarm(value, &alarm);
    if ( !fail ) {
        char sLine[LINE_BUFFER_SIZE], sRaw[LINE_BUFFER_SIZE];
        readAlarm(alarm, sLine, sizeof(sLine) );
        rawAlarm (alarm, sRaw,  sizeof(sRaw)  );
        
        if ( mqtt.connected() ) {
            char topic[LINE_BUFFER_SIZE];
            sprintf(topic, "alarm/%d/pretty", alarm);
            mqtt.publish(topic,sLine,true);

            sprintf(topic, "alarm/%d/state", alarm);
            mqtt.publish(topic,sRaw,true);
        }        

        if ( bVerb > VERBOSE_QUIET ) {
            printf("[>] %s\n", sLine);
        }
    } else {
        switch(fail) {
            case  ALARM_CLEARED:
                printf("[!] AlarmId: %d cleared\n", alarm);
                break;
                    
            case ALARM_IGNORED:
                printf("[!] AlarmId: %d already set. Skipping\n", alarm);
                break;
                
            case ALARM_INVALID_PARAM:
                Serial.println("[!] Unable to create Alarm. Invalid param");
                break;
                
            case ALARM_INVALID_RELAY:
                Serial.println("[!] Unable to create Alarm. Invalid relay");
                break;
                
            case ALARM_INVALID_STATE:
                Serial.println("[!] Unable to create Alarm. Invalid state");
                break;
                
            case ALARM_INVALID_TMODE:
                Serial.println("[!] Unable to create Alarm. Invalid tmode");
                break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
void subscriber ( char *topic, byte *val, unsigned int length) {
    val[length] = '\0';
    char *t=topic, *value=(char *) val;
    const char *routes[] = {
        "config/reset",   // 0
        "config/verbose", // 1
        "config/ping",    // 2
        "alarm/clear",    // 3
        "alarm/list",     // 4
        "alarm/set",      // 5
        "relay/all",      // 6
        "relay/list",     // 7
        "relay"           // 8
    };

    if ( bVerb > VERBOSE_INFO ) {
        printf("[>] topic: %s, value: %s\n", topic, value);
    }

    const int rSize = sizeof(routes) / sizeof(char *);
    int nRoute=0, nVal=strtol(value, NULL, 0);
    for ( int n=0; n<rSize; n++ ) {
        if ( strstr(topic, routes[n]) == topic ) {
            nRoute = n;          
            break; 
        }
    }
    
    switch(nRoute) {
        case 0:     // config/reset
            if ( nVal == 0xF0DA5E ) {
                Serial.println("[*] Rebooting...\n\n");
                ESP.restart();
            }
            break;
      
        case 1:     // config/verbose
            bVerb = nVal;
            printf("[*] Verbose mode is %s\n", sVerbose[bVerb]);
            break;
            
        case 2:     // config/ping
            bPing = nVal;
            bPing ? Alarm.enable(pingTimer) : Alarm.disable(pingTimer);
            printf("[*] Turning ping %s (1s)\n", sStates[bPing] );
            break;
            
        case 3:     // alarm/clear
            clearAlarms();
            break;
            
        case 4:     // alarm/list
            listAlarms(nVal);
            break;
            
        case 5:     // alarm/set
            postAlarm(value);
            break;
            
        case 6:     // relay/all
            for ( int i=0; i<numRelays; setRelay(i++, nVal) );
            break;
            
        case 7:     // relay/list
            listRelays(nVal);
            break;
            
        case 8:     // relay/*
            strsep(&t,"/");
            setRelay(strtol(t, NULL, 0), nVal);
            break;
            
        default:    // Unknown topic
            printf("[!] Unknown topic: %s\n", topic);
            break;
    }
}

/////////////////////////////////////////////////////////////////////////
// Connect to MQTT Broker 
/////////////////////////////////////////////////////////////////////////
void mqttConnect() {
    mdnsEntry *mqttAddress = mdnsResolve("_mqtt._tcp.local");

    mqtt.setCallback(subscriber);
    mqtt.setServer(mqttAddress->addr, mqttAddress->port);
 
    if (mqtt.connect("ACME Aquarius")) {
        mqtt.subscribe("config/+");         // Config
        mqtt.subscribe("alarm/+");          // Alarms
        mqtt.subscribe("relay/+/action");   // Relays
        mqtt.subscribe("relay/list");       // Relays List
        
        printf("[*] MQTT connected to %s (%s:%d)\n",  
            mqttAddress->serv, mqttAddress->addr, mqttAddress->port);
    } else {
        printf("[!] MQTT connection failed, rc=%d\n", mqtt.state());
    }
}

/////////////////////////////////////////////////////////////////////////
// Read DHT Data
/////////////////////////////////////////////////////////////////////////
void dhtUpdate() {  
    float fH,    fT,    fI,    fV;
    char  sH[6], sT[6], sI[6], sV[6];

    fV = ESP.getVcc() / 1000.0;
    fH = dht.readHumidity();
    fT = dht.readTemperature();

    if ( !isnan(fH) && !isnan(fT) ) {
        fI = dht.computeHeatIndex(fT, fH, true);

        dtostrf(fV, 5, 3, sV);
        dtostrf(fH, 5, 1, sH);
        dtostrf(fT, 5, 1, sT);
        dtostrf(fI, 5, 1, sI);

        if ( mqtt.connected() ) {
            mqtt.publish("dht/temperature", sT);
            mqtt.publish("dht/humidity",    sH);
            mqtt.publish("dht/heatIndex",   sI);             
            mqtt.publish("dht/tension_3v",  sV);             
        }

        if ( bVerb > VERBOSE_QUIET ) {
            printf("[>] DHT Loop: V=%sV, T=%sºC, H=%s%%, HI=%sºC\n", sV, sT, sH, sI);
        }            
    }        
}

/////////////////////////////////////////////////////////////////////////
// Blink LED_BUILTIN Callback
/////////////////////////////////////////////////////////////////////////
void ledToggle() {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN) );
}

/////////////////////////////////////////////////////////////////////////
// PING Callback
/////////////////////////////////////////////////////////////////////////
void ping() {
    if ( bVerb > VERBOSE_QUIET ) {
        time_t t = now();
        printf ("[|] %ld, 0x%lX, %s", t, (unsigned long)t, ctime(&t) );        
    }
}

/////////////////////////////////////////////////////////////////////////
// NTP and RTC callbacks
/////////////////////////////////////////////////////////////////////////
time_t getEpochNTP() {
    Serial.println("[*] NTP time sync");
    return (time_t) ntp.getEpochTime();
}

time_t getEpochRTC() {
    Serial.println("[*] RTC time sync");
    return (time_t) RTC.now().unixtime();
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, 0);

    for ( int i=0; i<numRelays; i++ ) {
        RelayState[i] = 0;
        Relay.set(0,i,RelayState[i]);
    }
    
    dht.begin();
    Wire.begin();
    RTC.begin();
    
    Serial.begin(115200);

    // We start by connecting to a WiFi network
    WiFi.mode(WIFI_STA);
    WiFi.begin("NYXK", "It\'s$14.99!");

    Serial.print("\n\n");
    Serial.print("[*] Connecting to ");
    Serial.print(WiFi.SSID());
  
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
      
    Serial.println(WiFi.localIP());

    ntp.begin();
    if ( ntp.forceUpdate() ) {
        setSyncProvider ( getEpochNTP );
    } else if ( RTC.isrunning() ) {
        setSyncProvider ( getEpochRTC );
    } else {
        // Unable to acquire time rebooting...
        Serial.println("[!] Unable to acquire time. Rebooting in 1h...");
        
        // wait for an hour then reboot;
        delay(3600000);
        ESP.restart();
    }
    
    mqttConnect();
    dhtUpdate();

    setAlarmCallback(setRelay);

    sensorTimer = Alarm.timerRepeat(15, dhtUpdate); 
    Alarm.enable(sensorTimer);
    
    blinkTimer = Alarm.timerRepeat(1, ledToggle);
    Alarm.disable(blinkTimer);
    
    pingTimer = Alarm.timerRepeat(1, ping); 
    Alarm.disable(pingTimer);
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
void loop() {
    ntp.update();
  
    ////////////////////
    // Try to stay always connected to the MQTT Server
    ////////////////////    
    if ( !mqtt.connected() ) {
        Alarm.enable(blinkTimer);
        Serial.println("[!] Retrying to connect to MQTT Broker");
        mqttConnect();
    } else {
        Alarm.disable(blinkTimer);
        digitalWrite(LED_BUILTIN, 1);   // LED OFF == 1
        mqtt.loop();
    }
    
    Alarm.delay(100);
    yield();
}
