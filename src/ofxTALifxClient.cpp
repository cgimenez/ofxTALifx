#include "ofMain.h"
#include "ofxTALifxBulb.h"
#include "ofxTALifxClient.h"

ofxTALifxClient::ofxTALifxClient() {
    last_discovered = 0;
    bulbs.clear();
    udpMan.connect();
}

/*
 * Return a vector of bulb instances corresponding to the target
 * * all bulbs
 * #XX bulbs of group XX
 * XX bulb named XX
 *
 */
bulb_vector ofxTALifxClient::targeted_bulbs(string target) {
    bulb_vector v;
    if (target == "*") {
        for (auto& bulb: bulbs) {
            v.push_back(bulb.second);
        }
    } else if (target[0] == '#') {
        string group_name = target.substr(1, target.size() - 1);
        for (auto& bulb: bulbs) {
            if (bulb.second.group_name == group_name) {
                v.push_back(bulb.second);
            }
        }
    } else {
        v.push_back(find_bulb(target)->second);
    }
    return v;
}

/*
 * Add bulbs to a group
 *
 */
void ofxTALifxClient::Group(string groupName, vector<string> bulbs_labels) {
    for (auto label: bulbs_labels) {
        bulb_map_iterator it = find_bulb(label);
        if (it != bulbs.end()) {
            it->second.group_name = groupName;
        }
    }
}

/*
 * Remove bulb from group
 *
 */
void ofxTALifxClient::unGroup(string groupName) {
    for (auto bulb: bulbs) {
        if(bulb.second.group_name == groupName)
            bulb.second.group_name = "";
    }
}

void ofxTALifxClient::unGroupAll() {
    for (auto& bulb: bulbs) {
        bulb.second.group_name = "";
    }
}

void ofxTALifxClient::discover() {
    lifx::message::device::GetService message;
    lifx::NetworkHeader header;

    udpMan.buildHeader(header, true, message);
    udpMan.sendBroadcast(header);
    last_discovered = ofGetElapsedTimeMillis();
    ofLog(OF_LOG_VERBOSE) << "DISCOVER : GET_SERVICE broadcast sent";
}

bulb_map_iterator ofxTALifxClient::find_bulb(string label) {
    for (bulb_map_iterator it = bulbs.begin(); it != bulbs.end(); it++) {
        if (it->second.label == label) {
            return it;
        }
    }
    return bulbs.end();
}

void ofxTALifxClient::check_online() {
    /*for (int i=0; i<bulbs.size(); i++) {
        if (ofGetElapsedTimeMillis() - bulbs[i].last_seen_at > 5000) {
            bulbs[i].online = false;
        }
    }
    */
}

void ofxTALifxClient::dump_bulbs() {
    string s = ofToString(bulbs.size()) + " bulbs found - BULBS : ";
    for (auto& bulb: bulbs) {
        s += "[" + bulb.second.label + "#" + ofxTALifxBulb::target_to_hex(bulb.second.target) + "#" + bulb.second.ip_address + "#" + bulb.second.group_name + "] ";
    }
    if (bulbs.size() > 0)
        ofLog() << s;
    last_bulbs_dump = ofGetElapsedTimeMillis();
}

void ofxTALifxClient::threadedFunction() {
    uint64_t target;
    lifx::NetworkHeader header;

    while (isThreadRunning()) {
        if (udpMan.has_pending_data()) {
            udpMan.receive(header);
            target = ofxTALifxBulb::target_to_64(header.target);
            if (target != 0x0000000000000000) { // No loopback
                this->sleep(5);
                lock();
                auto it = bulbs.find(target);
                if (it == bulbs.end()) {
                    ofxTALifxBulb new_bulb(udpMan);
                    std::memcpy(new_bulb.target, header.target, sizeof(header.target));
                    new_bulb.setManager(udpMan);
                    new_bulb.ip_address = udpMan.getIpAddress();
                    new_bulb.target64 = target;
                    bulbs.insert(pair<uint64_t, ofxTALifxBulb>(target, new_bulb));
                }
                ofxTALifxBulb& bulb = bulbs.find(target)->second;
                bulb.last_seen_at = ofGetElapsedTimeMillis();
                bulb.online = true;
                // ofLog(OF_LOG_VERBOSE) << ofxTALifxBulb::target_to_hex(bulb.target) << " " << ofToHex(target) << " " << header.type;
                switch (header.type) {
                    case lifx::message::device::StateService::type: // STATE_SERVICE
                        lifx::message::device::StateService message;
                        memcpy((char*)&message, (char*)&header.payload[0], sizeof(message));
                        ofLog(OF_LOG_VERBOSE) << "STATE_SERVICE rcv for target " << ofxTALifxBulb::target_to_hex(bulb.target) << " " << message.port;
                        if (!bulb.label_received) {
                            bulb.GetLabel();

                        }
                        if (!bulb.group_received) {
                            bulb.GetGroup();
                        }
                        break;

                    case lifx::message::device::StateLabel::type: // STATE_LABEL
                        ofLog(OF_LOG_VERBOSE) << "STATE_LABEL rcv for target " << ofxTALifxBulb::target_to_hex(bulb.target);
                        bulb.StateLabel(header);
                        break;

                    case lifx::message::device::StateGroup::type: // STATE_GROUP
                        ofLog(OF_LOG_VERBOSE) << "STATE_GROUP rcv for target " << ofxTALifxBulb::target_to_hex(bulb.target);
                        bulb.StateGroup(header);

                    case lifx::message::device::Acknowledgement::type:  // ACKNOWLEDGMENT
                        ofLog() << "ACK rcv SEQ = " << int(header.sequence) << " " << ofxTALifxBulb::target_to_hex(bulb.target);
                        udpMan.ackReceived(header.sequence);
                        break;
                    default:
                        ofLog() << "Unhandled header.type message " << header.type;
                        break;
                }
                //}
            } // Target 0
        }
        if (ofGetElapsedTimeMillis() - last_bulbs_dump > DUMP_DELAY)
            dump_bulbs();
        if (ofGetElapsedTimeMillis() - last_discovered > DISCOVER_DELAY || last_discovered == 0) {
            discover();
        }
        unlock();
        //refresh_online();
    }
}

void ofxTALifxClient::nextColor(string target, float H, float S, float B) {
    color_states[target] = {H,S,B};
}

void ofxTALifxClient::Color(string target, float H, float S, float B, uint16_t T) {
    lock();
    color_states[target] = {H,S,B};
    color(targeted_bulbs(target),H,S,B,T);
    unlock();
}

void ofxTALifxClient::color(bulb_vector bulbs, float H, float S, float B, uint16_t T) {
    for (auto& b : bulbs) {
        b.SetColor(H,S,B,T);
    }
}

void ofxTALifxClient::Off(string target) {
    lock();
    off(targeted_bulbs(target));
    unlock();
}

void ofxTALifxClient::off(bulb_vector bulbs) {
    for (auto& b : bulbs) {
        b.SetPower(0);
    }
}

void ofxTALifxClient::On(string target) {
    lock();
    on(targeted_bulbs(target));
    unlock();
}

void ofxTALifxClient::on(bulb_vector bulbs) {
    for (auto& b : bulbs) {
        b.SetPower(65535);
    }
}

void ofxTALifxClient::White(string target, uint16_t K, float B, uint16_t T) {
    lock();
    white(targeted_bulbs(target),B,K,T);
    unlock();
}

void ofxTALifxClient::white(bulb_vector bulbs, uint16_t K, float B, uint16_t T) {
    for (auto& b : bulbs) {
        b.SetWhite(K,B,T);
    }
}

void ofxTALifxClient::Boff(string target) {
    lock();
    boff(targeted_bulbs(target));
    unlock();
}

void ofxTALifxClient::boff(bulb_vector bulbs) {
    for (auto& b : bulbs) {
        b.SetColor(0,0,0);
    }
}

void ofxTALifxClient::Bon(string target) {
    lock();
    color_state state = color_states[target];
    bon(targeted_bulbs(target), state.H, state.S, state.B);
    unlock();
}

void ofxTALifxClient::bon(bulb_vector bulbs, float H, float S, float B) {
    for (auto& b : bulbs) {
        b.SetColor(H,S,B);
    }
}
