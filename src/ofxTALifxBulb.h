#ifndef OFXTALIFXBULB_H
#define OFXTALIFXBULB_H

#include "ofxNetwork.h"
#include "Poco/NumberFormatter.h"

#include "defines.h"
#include "protocol.h"
#include "ofxTALifxUdpManager.h"

class ofxTALifxBulb {
  public:
    string      label;
    string      group_label;
    string      group_name; // app side use
    target_t    target;
    uint64_t    target64;
    string      ip_address;

    ofxTALifxUdpManager& udpMan;

    uint64_t last_seen_at = 0;
    bool online = true;
    bool label_received = false;
    bool group_received = false;

    ofxTALifxBulb(ofxTALifxUdpManager& _udpMan);

    static string target_to_hex(uint8_t t[]);
    static void target_from_u64(target_t* t1, uint64_t* t2);
    static uint64_t target_to_64(target_t t);

    void setManager(ofxTALifxUdpManager& _udpMan);

    void GetLabel();
    void StateLabel(lifx::NetworkHeader& header);
    void GetGroup();
    void StateGroup(lifx::NetworkHeader& header);
    void SetColor(float H, float S, float B, uint16_t T = 0);
    void SetWhite(float B, uint16_t K, uint16_t T = 0);
    void SetPower(uint16_t level);
};

#endif // OFXTALIFXBULB_H
