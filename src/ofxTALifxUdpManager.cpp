#include "ofxTALifxBulb.h"
#include "ofxTALifxUdpManager.h"

ofxTALifxUdpManager::ofxTALifxUdpManager() {
    source_id = uint32_t(ofRandom(1,10000));
    ofLog() << "Source ID is " << source_id;
}

void ofxTALifxUdpManager::connect() {
    udpCnx.Create();
    if (!udpCnx.SetEnableBroadcast(true)) {
        ofLogError("Cannot SetEnableBroadcast");
    }
    /*
    if (!udpCnx.SetReuseAddress(true)) {
        ofLogError("Cannot SetReuseAddress");
    }
    */
    udpCnx.SetNonBlocking(true);
    if (!udpCnx.Bind(OFX_TALIFX_PORT)) {
        ofLogError("Cannot bind");
    }
    if (!udpCnx.Connect(OFX_TALIFX_BROADCAST_ADDRESS, OFX_TALIFX_PORT)) {
        ofLogError("Cannot connect");
    }
}

bool ofxTALifxUdpManager::has_pending_data() {
    return udpCnx.PeekReceive() > 0;
}

void ofxTALifxUdpManager::receive(lifx::NetworkHeader& header) {
    int size = udpCnx.PeekReceive();
    if (size > 0) {
        memset((void*) &header, 0, sizeof(header));
        udpCnx.Receive((char*) &header, size);
    }
}

void ofxTALifxUdpManager::sendBroadcast(lifx::NetworkHeader& header) {
    memset(header.target, 0, sizeof(header.target));
    header.tagged = true;
    udpCnx.Connect(OFX_TALIFX_BROADCAST_ADDRESS, OFX_TALIFX_PORT);
    ofLog() << "BROADCAST " << ofxTALifxBulb::target_to_hex(header.target) << " TYPE " << header.type;
    udpCnx.Send((char*)&header, header.size);
}

void ofxTALifxUdpManager::sendUnicast(ofxTALifxBulb& bulb, lifx::NetworkHeader& header) {
    memcpy(header.target, bulb.target, sizeof(lifx::NetworkHeader::target));
    header.tagged = false;
    udpCnx.Connect(bulb.ip_address.c_str(), OFX_TALIFX_PORT);
    ofLog() << "UNICAST " << bulb.ip_address << " " << ofxTALifxBulb::target_to_hex(header.target)  << " TYPE " << header.type << " SEQ " << int(header.sequence);
    uint64_t last_sent = last_send_times[bulb.target64];
    while (ofGetElapsedTimeMillis() - last_sent <= 50)
        ;
    last_send_times[bulb.target64] = ofGetElapsedTimeMillis();
    udpCnx.Send((char*)&header, header.size);
    if (header.ack_required) {
        ack_waiting_entry awe;
        awe.header = header;
        awe.sent_at = ofGetElapsedTimeMillis();
        headers_waiting_ack[header.sequence] = awe;
    }
}

string ofxTALifxUdpManager::getIpAddress() {
    string ip_address;
    int port;

    udpCnx.GetRemoteAddr(ip_address, port);
    return ip_address;
}

void ofxTALifxUdpManager::ackReceived(uint8_t sequence) {
    headers_waiting_ack_map::iterator it = headers_waiting_ack.find(sequence);
    if (it != headers_waiting_ack.end()) {
        headers_waiting_ack.erase(it);
    }
}

void ofxTALifxUdpManager::ackCheck() {
    for (auto& awe : headers_waiting_ack) {
        if (ofGetElapsedTimeMillis() - awe.second.sent_at > 300) {
            // resend
            // udpCnx.Send((char*)&awe.second.header, awe.second.header);
            // awe.second.sent_at = ofGetElapsedTimeMillis();
        }
    }
}
