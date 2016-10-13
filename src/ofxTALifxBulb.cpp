#include "ofxTALifxBulb.h"

ofxTALifxBulb::ofxTALifxBulb(ofxTALifxUdpManager& _udpMan) : udpMan(_udpMan) {
}

void ofxTALifxBulb::setManager(ofxTALifxUdpManager& _udpMan) {
    udpMan = _udpMan;
}

string ofxTALifxBulb::target_to_hex(uint8_t t[]) {
    string s;
    for (int i=0; i<8; i++) {
        Poco::NumberFormatter::appendHex(s, t[i], 2);
    }
    return s;
}

void ofxTALifxBulb::target_from_u64(target_t* t1, uint64_t* t2) {
    strncpy((char*)t1[0], (char*)t2, 8);
}

uint64_t ofxTALifxBulb::target_to_64(target_t t) {
    uint64_t res = 0;
    int shift = 0;

    for (int i=0; i<8; i++) {
        res = res << 8 | t[i];
        shift += 8;
    }
    return res;
}

void ofxTALifxBulb::GetLabel() {
    lifx::message::device::GetLabel message;
    lifx::NetworkHeader header;

    udpMan.buildHeader(header, false, message);
    udpMan.sendUnicast(*this, header);
    ofLog(OF_LOG_VERBOSE) << "SEND GetLabel for target " << target_to_hex(this->target);
}

void ofxTALifxBulb::StateLabel(lifx::NetworkHeader& header) {
    lifx::message::device::StateLabel message;

    memcpy((char*)&message, (char*)&header.payload[0], sizeof(message));
    label = string(message.label);
    label_received = true;
}

void ofxTALifxBulb::GetGroup() {
    lifx::message::device::GetGroup message;
    lifx::NetworkHeader header;

    udpMan.buildHeader(header, false, message);
    udpMan.sendUnicast(*this, header);
    ofLog(OF_LOG_VERBOSE) << "SEND GetGroup for target " << target_to_hex(this->target);
}

void ofxTALifxBulb::StateGroup(lifx::NetworkHeader& header) {
    lifx::message::device::StateGroup message;

    memcpy((char*)&message, (char*)&header.payload[0], sizeof(message));
    group_label = string(message.label);
    group_received = true;
}

void ofxTALifxBulb::SetColor(float H, float S, float B, uint16_t T) {
    lifx::message::light::SetColor message;
    lifx::NetworkHeader header;

    message.color = {uint16_t(H * 65535), uint16_t(S * 65535), uint16_t(B * 65535), 2500};
    message.duration = T;
    udpMan.buildHeader(header, false, message, true);
    udpMan.sendUnicast(*this, header);
    ofLog(OF_LOG_VERBOSE) << "SEND SetColor for target " << target_to_hex(this->target) << " " << this->ip_address;
}

void ofxTALifxBulb::SetWhite(float B, uint16_t K, uint16_t T) {
    lifx::message::light::SetColor message;
    lifx::NetworkHeader header;

    K = K < 2500 ? 2500 : K;
    K = K > 9600 ? 9600 : K;
    message.color = {0, 0, uint16_t(B * 65535), K};
    message.duration = T;
    udpMan.buildHeader(header, false, message, true);
    udpMan.sendUnicast(*this, header);
    ofLog(OF_LOG_VERBOSE) << "SEND SetColor for target " << target_to_hex(this->target) << " " << this->ip_address;
}

void ofxTALifxBulb::SetPower(uint16_t level) {
    lifx::message::device::SetPower message;
    lifx::NetworkHeader header;

    message.level = level;
    udpMan.buildHeader(header, false, message, true);
    udpMan.sendUnicast(*this, header);
    ofLog(OF_LOG_VERBOSE) << "SEND SetPower for target " << level << " " << target_to_hex(this->target) << " " << this->ip_address;
}
