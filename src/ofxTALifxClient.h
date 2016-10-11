#ifndef OFXTALIFXCLIENT_H
#define OFXTALIFXCLIENT_H

#include "ofxNetwork.h"
#include "Poco/NumberFormatter.h"
#include "protocol.h"

#define LIFX_BROADCAST_ADDRESS "192.168.1.255"
#define LIFX_PORT 56700
#define DISCOVER_DELAY 5000

typedef uint8_t target_type[8];

class ofxTALifxBulb {
  public:
    string label;
    string group_label;
    target_type target;
    string ip_address;
    uint64_t last_seen_at = 0;
    bool online = true;
    bool label_received = false;
    bool group_received = false;

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
    uint32_t source_id;
    uint64_t last_discovered;
    uint64_t last_bulbs_dump = 0;
    bulb_map bulbs;
    string target_to_hex(uint8_t t[]);
    enum statuses { idle, init, get_labels };
    int status;

    void dump_bulbs();
    void check_online();
    void getLabel(target_type& target);
    void getGroup(target_type& target);
    bool find_bulb(string label, ofxTALifxBulb& _bulb);
    bool set_target(string starget, target_type& target);
    void send(lifx::NetworkHeader& header);

  public:
    ofxTALifxClient();

    template<typename T>
    void set_header(lifx::NetworkHeader& header, target_type target, bool tagged, T message, bool _res_required = false) {
        header.origin = 0;
        header.tagged = tagged;
        header.addressable = true;
        header.protocol = 1024;
        header.source = source_id;

        memcpy(header.target, target, sizeof(lifx::NetworkHeader::target));
        memset(header.site, 0, sizeof(lifx::NetworkHeader::site));
        header.ack_required = 0;
        header.res_required = _res_required;
        header.sequence = 2; // A générer ?

        header.type = message.type;
        unsigned long payload_size = sizeof(T) == 1 ? 0 : sizeof(T);
        header.size = sizeof(lifx::NetworkHeader) - sizeof(lifx::NetworkHeader::payload) + payload_size;
        if (payload_size > 0)
            memcpy(header.payload, &message, payload_size);
    }

    void connect();
    void discover();
    void update();
    void color(string target, float H, float S, float B, uint16_t T = 0);
    void power_off(string target);
    void power_on(string target);
    void test();
};

#endif // OFXTALIFXCLIENT_H
