/*

*/
#include <string.h>
#include <mdns.h>

#include "mdnsResolver.h"

static mdnsEntry _mdns;
static char query[MAX_MDNS_NAME_LEN];


static void answerCallback(const mdns::Answer* answer) {    
    switch ( answer->rrtype ) {
        case MDNS_TYPE_PTR:
            if ( strstr(answer->name_buffer, query) != 0 ) {
                strncpy(_mdns.serv, answer->rdata_buffer, MAX_MDNS_NAME_LEN);
            }
        break;

        case MDNS_TYPE_A: 
            if ( strstr(answer->name_buffer, _mdns.host) != 0 ) {
                strncpy(_mdns.addr, answer->rdata_buffer, MAX_MDNS_NAME_LEN);
            }
        break;

        case MDNS_TYPE_SRV:
            if ( strstr(answer->name_buffer, _mdns.serv) != 0 ) {
                char *p = strstr(answer->rdata_buffer, "port=");
                if (p) {
                    _mdns.port = atoi(p+5);
                }

                char *h = strstr(answer->rdata_buffer, "host=");
                if (h) {
                    strncpy(_mdns.host, h+5, MAX_MDNS_NAME_LEN);
                }
            }
        break;            
    }
}


mdnsEntry* mdnsResolve ( const char* _service ) {
    memset( &_mdns, 0, sizeof(mdnsEntry) );
    strncpy(query, _service, MAX_MDNS_NAME_LEN);

    mdns::MDns my_mdns(NULL, NULL, answerCallback);    
    
    // Query for all host information for a paticular service. ("_mqtt" in this case.)
    my_mdns.Clear();
    struct mdns::Query query_mqtt;
    strncpy(query_mqtt.qname_buffer, query, MAX_MDNS_NAME_LEN);
    query_mqtt.qtype = MDNS_TYPE_PTR;
    query_mqtt.qclass = 1;    // "INternet"
    query_mqtt.unicast_response = 0;
    
    my_mdns.AddQuery(query_mqtt);
    my_mdns.Send();
    
    while ( !strlen(_mdns.addr) ) {
        my_mdns.loop();
    }
    
    return &_mdns;
}
