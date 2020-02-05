#ifndef OFXTALIFXUDPMANAGER_H
#define OFXTALIFXUDPMANAGER_H

#include "ofMain.h"
#include "ofxNetwork.h"
#include "defines.h"
#include "protocol.h"

namespace ofxtalifx {

class ofxTALifxBulb;

struct ack_waiting_entry {
    lifx::NetworkHeader header;
    uint64_t sent_at;
    uint16_t send_count = 0;
};

typedef std::unordered_map<uint8_t, ack_waiting_entry> headers_waiting_ack_map;

class ofxTALifxUdpManager {
  private:
    ofxUDPManager   udpCnx;
    uint32_t        source_id;
    uint8_t         sequence_id = 0;
    uint64_t        last_send_time = 0;
    std::unordered_map<uint64_t, uint64_t> last_send_times;
    headers_waiting_ack_map headers_waiting_ack;
    bool            ack_mode_enabled = false;

  public:
    ofxTALifxUdpManager();
    virtual ~ofxTALifxUdpManager();

    template<typename T>
    void buildHeader(lifx::NetworkHeader& header, const bool tagged, const T message, const bool _ack_required = false) {
        header.origin = 0;
        header.tagged = tagged;
        header.addressable = true;
        header.protocol = 1024;
        header.source = source_id;

        // target is set by sendUnicast or sendBroadcast
        memset(header.site, 0, sizeof(header.site));
        if (ack_mode_enabled)
            header.ack_required = _ack_required;
        else
            header.ack_required = false;

        header.res_required = 0;
        header.sequence = ++sequence_id;

        header.at_time = 0;
        header.type = message.type;
        unsigned long payload_size = sizeof(T) == 1 ? 0 : sizeof(T);
        header.size = sizeof(lifx::NetworkHeader) - sizeof(lifx::NetworkHeader::payload) + payload_size;
        if (payload_size > 0)
            memcpy(header.payload, &message, payload_size);
    }

    void setup();
    bool has_pending_data();
    void receive(lifx::NetworkHeader& header);
    void sendBroadcast(lifx::NetworkHeader& header);
    void sendUnicast(const ofxTALifxBulb& bulb, lifx::NetworkHeader& header);
    const string getIpAddress();
    void enableAck() {
        ack_mode_enabled = true;
    }
    void disableAck() {
        ack_mode_enabled = false;
    }
    void ackReceived(const uint8_t sequence);
    void ackCheck();
};

}

#endif // OFXTALIFXUDPMANAGER_H
