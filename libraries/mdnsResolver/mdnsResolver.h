/*

*/
#ifndef __MDNS_RESOLVER__
#define __MDNS_RESOLVER__

#include <mdns.h>

typedef struct {
    char serv[MAX_MDNS_NAME_LEN];
    char host[MAX_MDNS_NAME_LEN];
    char addr[MAX_MDNS_NAME_LEN];
    int port;
} mdnsEntry;

mdnsEntry* mdnsResolve ( const char* _service );

#endif