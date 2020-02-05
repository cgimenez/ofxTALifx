#include "ofxTALifxBulb.h"

using namespace ofxtalifx;

ofxTALifxBulb::ofxTALifxBulb(ofxTALifxUdpManager& _udpMan) : udp_man(_udpMan) {
}

const string ofxTALifxBulb::toString() {
    string s = label + " " + ip_address;
    if (online)
        s+= " [online]";
    else
        s+= " [offline since " + ofToString((ofGetElapsedTimeMillis() - OFFLINE_DELAY) / 1000) + "]";
    return s;
}

string ofxTALifxBulb::targetToHex(const uint8_t t[]) {
    string s;
    for (int i=0; i<8; i++) {
        Poco::NumberFormatter::appendHex(s, t[i], 2);
    }
    return s;
}

void ofxTALifxBulb::targetFrom64(target_t* t1, const uint64_t* t2) {
    strncpy(reinterpret_cast<char*>(t1[0]), (char*)t2, 8);
}

uint64_t ofxTALifxBulb::targetTo64(const target_t t) {
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

    DLOG("SEND GetLabel for target " << target_to_hex(this->target));
    udp_man.buildHeader(header, false, message);
    udp_man.sendUnicast(*this, header);
}

void ofxTALifxBulb::StateLabel(lifx::NetworkHeader& header) {
    lifx::message::device::StateLabel message;

    memcpy(reinterpret_cast<char*>(&message), reinterpret_cast<char*>(&header.payload[0]), sizeof(message));
    label = string(message.label);
    label_received = true;
}

void ofxTALifxBulb::GetGroup() {
    lifx::message::device::GetGroup message;
    lifx::NetworkHeader header;

    DLOG("SEND GetGroup for target " << target_to_hex(this->target));
    udp_man.buildHeader(header, false, message);
    udp_man.sendUnicast(*this, header);
}

void ofxTALifxBulb::StateGroup(lifx::NetworkHeader& header) {
    lifx::message::device::StateGroup message;

    memcpy(reinterpret_cast<char*>(&message), reinterpret_cast<char*>(&header.payload[0]), sizeof(message));
    group_label = string(message.label);
    group_received = true;
}

void ofxTALifxBulb::SetColor(const float H, const float S, const float B, const uint16_t T) {
    lifx::message::light::SetColor message;
    lifx::NetworkHeader header;

    message.color = {uint16_t(H * 65535), uint16_t(S * 65535), uint16_t(B * 65535), 2500};
    message.duration = T;
    udp_man.buildHeader(header, false, message, true);
    udp_man.sendUnicast(*this, header);
    DLOG("SEND SetColor for target " << target_to_hex(this->target) << " " << this->ip_address);
}

void ofxTALifxBulb::SetWhite(uint16_t K, const float B, const uint16_t T) {
    lifx::message::light::SetColor message;
    lifx::NetworkHeader header;

    DLOG("SEND SetColor for target " << target_to_hex(this->target) << " " << this->ip_address);
    K = K < 2500 ? 2500 : K;
    K = K > 9600 ? 9600 : K;
    message.color = {0, 0, uint16_t(B * 65535), K};
    message.duration = T;
    udp_man.buildHeader(header, false, message, true);
    udp_man.sendUnicast(*this, header);
}

void ofxTALifxBulb::SetPower(const uint16_t level) {
    lifx::message::device::SetPower message;
    lifx::NetworkHeader header;

    DLOG("SEND SetPower for target " << target_to_hex(this->target) << " level " << level << " " << this->ip_address);
    message.level = level;
    udp_man.buildHeader(header, false, message, true);
    udp_man.sendUnicast(*this, header);
}
