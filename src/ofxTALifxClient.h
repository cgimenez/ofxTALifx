#ifndef OFXTALIFXCLIENT_H
#define OFXTALIFXCLIENT_H

#include "ofxNetwork.h"
#include "Poco/NumberFormatter.h"
#include "protocol.h"

#define BROADCAST_ADDRESS "192.168.1.255"
#define DISCOVER_DELAY 5000

typedef uint8_t target_type[8];

class ofxTALifxBulb {
  public:
    string label;
    target_type target;
    uint64_t last_seen_at = 0;
    bool online = true;

    static string target_to_hex(uint8_t t[]) {
        string s;
        for (int i=0; i<8; i++) {
            Poco::NumberFormatter::appendHex(s, t[i], 2);
        }
        return s;
    }

    static void target_from_u64(target_type* t1, uint64_t* t2) {
        strncpy((char*)t1[0], (char*)t2, 8);
    }

    static uint64_t target_to_64(target_type t) {
        uint64_t res = 0;
        int shift = 0;

        for (int i=0; i<8; i++) {
            res = res << 8 | t[i];
            shift += 8;
        }
        return res;
    }
};

typedef std::unordered_map<uint64_t, ofxTALifxBulb> bulb_map;

class ofxTALifxClient {
  private:
    ofxUDPManager udpCnx;
    uint64_t last_discovered;
    bulb_map bulbs;
    string target_to_hex(uint8_t t[]);
    enum statuses { idle, init, get_labels };
    int status;

    void refresh_online();

  public:
    ofxTALifxClient();

    template<typename T>
    void set_header(lifx::NetworkHeader& header, target_type target, bool tagged, T message, bool _res_required = false) {
        header.origin = 0;
        header.tagged = tagged;
        header.addressable = true;
        header.protocol = 1024;
        header.source = 20; // A générer

        memcpy(header.target, target, sizeof(lifx::NetworkHeader::target));
        memset(header.site, 0, sizeof(lifx::NetworkHeader::site));
        header.ack_required = 0;
        header.res_required = _res_required;
        header.sequence = 2; // A générer ?

        header.type = message.type;
        unsigned long payload_size = sizeof(T) == 1 ? 0 : sizeof(T);
        header.size = sizeof(lifx::NetworkHeader) - sizeof(lifx::NetworkHeader::payload) + payload_size;
        if (payload_size > 0)
            memcpy(header.payload, ((uint8_t*)(&message)), payload_size);
    }

    void connect();
    void discover();
    void update();
    void getLabel(target_type& target);
    void getGroup(target_type& target);
    void getLabels();

    void color(float H, float S, float B, int K, int T);
};

#endif // OFXTALIFXCLIENT_H
