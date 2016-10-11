#include "ofMain.h"
#include "ofxTALifxClient.h"

ofxTALifxClient::ofxTALifxClient() {
    source_id = uint32_t(ofRandom(1,10000));
    last_discovered = 0;
    status = statuses::init;
    bulbs.clear();
    ofLog() << "Source ID is " << source_id;
}

void ofxTALifxClient::connect() {
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
    if (!udpCnx.Bind(LIFX_PORT)) {
        ofLogError("Cannot bind");
    }
    if (!udpCnx.Connect(LIFX_BROADCAST_ADDRESS, LIFX_PORT)) {
        ofLogError("Cannot connect");
    }
}

void ofxTALifxClient::send(lifx::NetworkHeader& header) {
    udpCnx.Send((char*)&header, header.size);
}

bool ofxTALifxClient::set_target(string starget, target_type& target) {
    if (starget == "*") {
        memset(target, 0, sizeof(target_type));
        return true;
    } else {
        ofxTALifxBulb bulb;
        if (find_bulb(starget, bulb)) {
            memcpy(target, bulb.target, sizeof(target_type));
            return true;
        } else return false;
    }
}

void ofxTALifxClient::discover() {
    lifx::message::device::GetService message;
    lifx::NetworkHeader header;

    uint8_t target[8] = {0};
    set_header(header, target, true, message);
    send(header);
    last_discovered = ofGetElapsedTimeMillis();
    status = statuses::get_labels;
    ofLog(OF_LOG_VERBOSE) << "GET_SERVICE broadcast sent";
}

bool ofxTALifxClient::find_bulb(string label, ofxTALifxBulb& _bulb) {
    for (auto bulb : bulbs) {
        if (bulb.second.label == label) {
            _bulb = bulb.second;
            return true;
        }
    }
    return false;
}

void ofxTALifxClient::getLabel(target_type& target) {
    lifx::message::device::GetLabel message;
    lifx::NetworkHeader header;

    set_header(header, target, false, message);
    send(header);
    ofLog(OF_LOG_VERBOSE) << "SEND GetLabel for target " << ofxTALifxBulb::target_to_hex(target);
}

void ofxTALifxClient::getGroup(target_type& target) {
    lifx::message::device::GetGroup message;
    lifx::NetworkHeader header;

    set_header(header, target, false, message);
    send(header);
    ofLog(OF_LOG_VERBOSE) << "SEND GetGroup for target " << ofxTALifxBulb::target_to_hex(target);
}

void ofxTALifxClient::check_online() {
    for (int i=0; i<bulbs.size(); i++) {
        if (ofGetElapsedTimeMillis() - bulbs[i].last_seen_at > 5000) {
            bulbs[i].online = false;
        }
    }
}

void ofxTALifxClient::dump_bulbs() {
    string s = ofToString(bulbs.size()) + " bulbs found - BULBS : ";
    for (auto bulb: bulbs) {
        s += "[" + bulb.second.label + "#" + ofxTALifxBulb::target_to_hex(bulb.second.target) + "#" + bulb.second.ip_address + "] ";
    }
    if (bulbs.size() > 0)
        ofLog() << s;
    last_bulbs_dump = ofGetElapsedTimeMillis();
}

void ofxTALifxClient::update() {
    uint64_t target;
    lifx::NetworkHeader header;
    ofxTALifxBulb new_bulb;
    int dummy_port;

    //if (status == statuses::init)
    //    return;

    int size = udpCnx.PeekReceive();
    if (size > 0) {
        memset((void*) &header, 0, sizeof(header));
        udpCnx.Receive((char*) &header, size);
        //if (header.type != 2) {
        target = ofxTALifxBulb::target_to_64(header.target);
        if (target != 0x0000000000000000) { // No loopback
            auto it = bulbs.find(target);
            if (it == bulbs.end()) {
                std::memcpy(new_bulb.target, header.target, 8);
                bulbs.insert(pair<uint64_t, ofxTALifxBulb>(target, new_bulb));
            }
            ofxTALifxBulb& bulb = bulbs.find(target)->second;
            bulb.last_seen_at = ofGetElapsedTimeMillis();
            bulb.online = true;
            udpCnx.GetRemoteAddr(bulb.ip_address, dummy_port);
            // ofLog(OF_LOG_VERBOSE) << ofxTALifxBulb::target_to_hex(bulb.target) << " " << ofToHex(target) << " " << header.type;
            switch (header.type) {
                case 3:  { // STATE_SERVICE
                        lifx::message::device::StateService message;
                        memcpy((char*)&message, (char*)&header.payload[0], sizeof(message));
                        ofLog(OF_LOG_VERBOSE) << "STATE_SERVICE rcv for target " << ofToHex(target) << " " << message.port;
                        if (!bulb.label_received) {
                            getLabel(bulb.target);
                        }
                        if (!bulb.group_received) {
                            getGroup(bulb.target);
                        }
                    }
                    break;

                case 25: { // STATE_LABEL
                        lifx::message::device::StateLabel message;
                        memcpy((char*)&message, (char*)&header.payload[0], sizeof(message));
                        bulb.label = string(message.label);
                        bulb.label_received = true;
                    }
                    break;

                case 53: { // STATE_GROUP
                        lifx::message::device::StateGroup message;
                        memcpy((char*)&message, (char*)&header.payload[0], sizeof(message));
                        bulb.group_label = string(message.label);
                        bulb.group_received = true;
                    }

                case 45:  // ACKNOLEDGMENT
                    break;
                default:
                    ofLog() << "Unhandled header.type message " << header.type;
                    break;
            }
            //}
        }
    }

    if (ofGetElapsedTimeMillis() - last_bulbs_dump > 5000)
        dump_bulbs();
    if (ofGetElapsedTimeMillis() - last_discovered > DISCOVER_DELAY) {
        discover();
    }
    //refresh_online();
}

void ofxTALifxClient::color(string target, float H, float S, float B, uint16_t T) {
    lifx::message::light::SetColor message;
    lifx::NetworkHeader header;
    target_type _target;

    if (set_target(target, _target)) {
        message.color = {uint16_t(H * 65535), uint16_t(S * 65535), uint16_t(B * 65535), 2500};
        message.duration = T;
        set_header(header, _target, true, message);
        send(header);
        ofLog() << "SEND Color for target " << ofxTALifxBulb::target_to_hex(_target) << " (" << target << ")";
    } else {
        ofLog() << "Target " << target << " not found";
    }
}

void ofxTALifxClient::power_off(string target) {
    lifx::message::light::SetPower message;
    lifx::NetworkHeader header;
    target_type _target;

    if (set_target(target, _target)) {
        message.duration = 0;
        message.level = 0;
        set_header(header, _target, false, message);
        send(header);
        ofLog() << "SEND PowerOff for target " << ofxTALifxBulb::target_to_hex(_target) << " (" << target << ")";
    } else {
        ofLog() << "Target " << target << " not found";
    }
}

void ofxTALifxClient::power_on(string target) {
    lifx::message::light::SetPower message;
    lifx::NetworkHeader header;
    target_type _target;

    if (set_target(target, _target)) {
        message.duration = 0;
        message.level = 65535;
        set_header(header, _target, false, message);
        send(header);
        ofLog() << "SEND PowerOn for target " << ofxTALifxBulb::target_to_hex(_target) << " (" << target << ")";
    } else {
        ofLog() << "Target " << target << " not found";
    }
}

void ofxTALifxClient::test() {
//    for (auto bulb : bulbs) {
//        udpCnx.Connect(bulb.second.ip_address.c_str(), LIFX_PORT);
//        power_off(bulb.second.label);
//        //color(bulb.second.label, ofRandom(0,1), ofRandom(0,1), 1);
//    }
//    sleep(1);
//    for (auto bulb : bulbs) {
//        udpCnx.Connect(bulb.second.ip_address.c_str(), LIFX_PORT);
//        power_on(bulb.second.label);
//        //color(bulb.second.label, ofRandom(0,1), ofRandom(0,1), 1);
//    }
    for (auto bulb : bulbs) {
        udpCnx.Connect(bulb.second.ip_address.c_str(), LIFX_PORT);
        color(bulb.second.label, ofRandom(0,1), ofRandom(0,1), 1);
    }
    udpCnx.Connect(LIFX_BROADCAST_ADDRESS, LIFX_PORT);
    //color("*", ofRandom(0,1), ofRandom(0,1), 1);
}
