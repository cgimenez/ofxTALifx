#ifndef DEFINES_H
#define DEFINES_H

#define OFX_TALIFX_BROADCAST_ADDRESS "192.168.1.255"
#define OFX_TALIFX_PORT 56700
#define DISCOVER_DELAY 20000
#define DUMP_DELAY 20000
#define OFFLINE_DELAY 30000

#ifdef DEBUG
#define DLOG(S) (ofLog() << S)
//#define DLOG(S) ;
#else
#define DLOG(S) ;
#endif

typedef uint8_t target_t[8];

#endif // DEFINES_H
