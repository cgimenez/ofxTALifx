// (C) 2016/2017 Christophe Gimenez for TACTIF Compagnie / Bordeaux - France (tactif.com)

#ifndef OFXTALIFXBULB_H
#define OFXTALIFXBULB_H

#include "ofxNetwork.h"
#include "Poco/NumberFormatter.h"

#include "defines.h"
#include "protocol.h"
#include "ofxTALifxUdpManager.h"

namespace ofxtalifx {

class ofxTALifxBulb {
  private:
    ofxTALifxUdpManager& udp_man;

  public:
    string      label;
    string      group_label;
    target_t    target;
    uint64_t    target64;
    string      ip_address;

    uint64_t    last_seen_at = 0;
    bool        online = true;
    bool        label_received = false;
    bool        group_received = false;

    ofxTALifxBulb(ofxTALifxUdpManager& _udpMan);

    const string toString();

    static string targetToHex(const uint8_t t[]);
    static void targetFrom64(target_t* t1, const uint64_t* t2);
    static uint64_t targetTo64(const target_t t);

    void GetLabel();
    void StateLabel(lifx::NetworkHeader& header);
    void GetGroup();
    void StateGroup(lifx::NetworkHeader& header);
    void SetColor(const float H, const float S, const float B, const uint16_t T = 0);
    void SetWhite(uint16_t K, const float B = 1.0, const uint16_t T = 0);
    void SetPower(const uint16_t level);
};

}

#endif // OFXTALIFXBULB_H
