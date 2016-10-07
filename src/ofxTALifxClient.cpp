#include "ofMain.h"
#include "ofxTALifxClient.h"

ofxTALifxClient::ofxTALifxClient() {
    last_discovered = 0;
    status = statuses::init;
}

void ofxTALifxClient::connect() {
    udpCnx.Create();
    if (!udpCnx.SetEnableBroadcast(true)) {
        ofLogError("Cannot SetEnableBroadcast");
    }
    if (!udpCnx.SetReuseAddress(true)) {
        ofLogError("Cannot SetReuseAddress");
    }
    udpCnx.SetNonBlocking(true);
    if (!udpCnx.Bind(56700)) {
        ofLogError("Cannot bind");
    }
    if (!udpCnx.Connect(BROADCAST_ADDRESS, 56700)) {
        ofLogError("Cannot connect");
    }
}

void ofxTALifxClient::discover() {
    lifx::message::device::GetService message;
    lifx::NetworkHeader header;

    uint8_t target[8] = {0};
    set_header(header, target, true, message);
    udpCnx.Send((char*)&header, header.size);
    last_discovered = ofGetElapsedTimeMillis();
    status = statuses::get_labels;
}

void ofxTALifxClient::getLabels() {
    lifx::message::device::GetLabel message;
    lifx::NetworkHeader header;
    target_type target;

    ofLog() << bulbs.size();
    for (auto bulb : bulbs) {
        set_header(header, bulb.second.target, true, message, true);
        udpCnx.Send((char*)&header, header.size);
        ofLog() << "SEND GetLabel for target " << ofxTALifxBulb::target_to_hex(target);
    }
}

void ofxTALifxClient::getLabel(target_type& target) {
    lifx::message::device::GetLabel message;
    lifx::NetworkHeader header;

    set_header(header, target, false, message, true);
    udpCnx.Send((char*)&header, header.size);
    ofLog() << "SEND GetLabel for target " << ofxTALifxBulb::target_to_hex(target);
}

void ofxTALifxClient::getGroup(target_type& target) {
    lifx::message::device::GetGroup message;
    lifx::NetworkHeader header;

    set_header(header, target, false, message, true);
    udpCnx.Send((char*)&header, header.size);
    ofLog() << "SEND GetGroup for target " << ofxTALifxBulb::target_to_hex(target);
}

void ofxTALifxClient::refresh_online() {
    for (int i=0; i<bulbs.size(); i++) {
        if (ofGetElapsedTimeMillis() - bulbs[i].last_seen_at > 5000) {
            bulbs[i].online = false;
        }
    }
}

void ofxTALifxClient::update() {
    uint64_t target;

    if (status == statuses::init)
        return;

    lifx::NetworkHeader header;
    int size = udpCnx.PeekReceive();
    if (size > 0) {
        memset((void*) &header, 0, sizeof(lifx::NetworkHeader));
        udpCnx.Receive((char*) &header, size);
        switch (header.type) {
            case 3: { // STATE_SERVICE
                    ofxTALifxBulb bulb;
                    target = ofxTALifxBulb::target_to_64(header.target);
                    bulb.last_seen_at = ofGetElapsedTimeMillis();
                    bulb.online = true;
                    std::memcpy(bulb.target, header.target, 8);
                    auto it = bulbs.find(target);
                    ofLog() << ofxTALifxBulb::target_to_hex(header.target);
                    if (it == bulbs.end()) {
                        bulbs.insert(pair<uint64_t, ofxTALifxBulb>(target, bulb));
                        getLabel(header.target);
                        getGroup(header.target);
                    }
                }

                break;

            case 25: { // STATE_LABEL
                    lifx::message::device::StateLabel message;
                    strncpy((char*)&message, (char*)&header.payload[0], sizeof(message));
                    ofLog() << string(message.label);
                    bulbs[target].label = string(message.label);
                }
                break;

            case 53: { // STATE_GROUP
                    lifx::message::device::StateGroup message;
                    strncpy((char*)&message, (char*)&header.payload[0], sizeof(message));
                    ofLog() << string(message.label);
                    // bulbs[target].label = string(message.label);
                }

            case 45: { // ACKNOLEDGMENT
                }
                break;
            default:
                ofLogNotice("Unhandled header.type message " + ofToString(header.type));
                break;
        }
    }
    //if (ofGetElapsedTimeMillis() - last_discovered > DISCOVER_DELAY) {
    //    discover();
    //}
    //refresh_online();
}

void ofxTALifxClient::color(float H, float S, float B, int K, int T) {

}
