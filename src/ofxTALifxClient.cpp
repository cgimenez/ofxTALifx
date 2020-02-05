#include "ofxTALifxClient.h"
#include "ofMain.h"
#include "ofxTALifxBulb.h"

using namespace ofxtalifx;

ofxTALifxClient::ofxTALifxClient() {
    last_discovered = 0;
    bulbs.clear();
    udp_man.setup();
    disableAck();
}

ofxTALifxClient::~ofxTALifxClient() {
    ofLogVerbose() << "ofxTALifxClient dtor";
}

/*
 * Find a bulb given its label
 * Return a bulb iterator, instead of the bulb itself as we can't return references
 *
 */
bulb_map_iterator ofxTALifxClient::findBulb(const string label) {
    for (bulb_map_iterator it = bulbs.begin(); it != bulbs.end(); it++) {
        if (it->second.label == label) {
            return it;
        }
    }
    return bulbs.end();
}

/*
 * Return a vector of bulb instances corresponding to the target
 * * all bulbs
 * #XX bulbs of group XX
 * XX bulb named XX
 *
 */
const bulb_vector ofxTALifxClient::targetedBulbs(const string target) {
    bulb_vector v;

    if (target == "*") {
        for (auto &bulb : bulbs) {
            v.push_back(&bulb.second);
        }
    } else if (target[0] == '#') {
        string group_name = target.substr(1, target.size() - 1);
        v = groups[group_name];
    } else {
        if (findBulb(target) != bulbs.end()) {
            v.push_back(&(findBulb(target)->second));
        }
    }
    return v;
}

const group_map ofxTALifxClient::getGroups() {
    return groups;
}

bulb_map ofxTALifxClient::getBulbs() {
    return bulbs;
}

/*
 * Overwrite group with bulbs
 *
 */
void ofxTALifxClient::setGroup(const string groupName, const vector<string> bulbs_labels) {
    bulb_vector v;
    for (auto &label : bulbs_labels) {
        bulb_map_iterator it = findBulb(label);
        if (it != bulbs.end()) {
            v.push_back(&(it->second));
        }
    }
    groups[groupName] = v;
}

/*
 * Add bulb to group
 *
 */
void ofxTALifxClient::addToGroup(const string groupName, const string label) {
    group_map_iterator git = groups.find(groupName);
    if (git != groups.end()) {
        bulb_map_iterator it = findBulb(label);
        if (it != bulbs.end()) {
            git->second.push_back(&(it->second));
        }
    }
}

/*
 * Remove bulb from group
 *
 */
void ofxTALifxClient::removeFromGroup(const string groupName, const string label) {
    group_map_iterator git;
    if (git != groups.end()) {
    }
}

/*
 * Delete group
 *
 */
void ofxTALifxClient::removeGroup(const string groupName) {
    group_map_iterator it = groups.find(groupName);
    if (it != groups.end()) {
        groups.erase(it);
    }
}

/*
 * Delete all groups
 *
 */
void ofxTALifxClient::removeAllGroups() {
    groups.clear();
}

/*
 * Discover bulbs on network
 *
 */

void ofxTALifxClient::Discover() {
    lifx::message::device::GetService message;
    lifx::NetworkHeader header;

    udp_man.buildHeader(header, true, message);
    udp_man.sendBroadcast(header);
    last_discovered = ofGetElapsedTimeMillis();
    DLOG("DISCOVER : GET_SERVICE broadcast sent");
}

/*
 * Set the online flag of bulbs
 *
 */
void ofxTALifxClient::checkOnline() {
    for (auto &bulb : bulbs) {
        if (ofGetElapsedTimeMillis() - bulb.second.last_seen_at > OFFLINE_DELAY) {
            bulb.second.online = false;
        }
    }
}

/*
 * Weird dump of discovered bulbs
 *
 */
void ofxTALifxClient::dumpBulbs() {
    string s;

    DLOG(ofToString(bulbs.size()) << " bulbs found");
    for (auto &bulb : bulbs) {
        s += "[" + bulb.second.label + "#" + ofxTALifxBulb::targetToHex(bulb.second.target) + "#" + bulb.second.ip_address + "] ";
    }
    if (bulbs.size() > 0)
        DLOG(s);
    last_bulbs_dump = ofGetElapsedTimeMillis();
}

/*
 * Heartbeat
 *
 */
void ofxTALifxClient::threadedFunction() {
    uint64_t target;
    lifx::NetworkHeader header;

    while (isThreadRunning()) {
        sleep(5);
        lock();
        if (udp_man.has_pending_data()) {
            udp_man.receive(header);
            target = ofxTALifxBulb::targetTo64(header.target);
            if (target != 0x0000000000000000) { // No loopback
                /*
                 * Problème ci dessous : si une lampe n'est pas correctement détecté la première fois (mauvais label, mauvaise IP)
                 * un nouveau discover ne prend pas en compte les nouvelles valeurs
                 *
                 */
                auto it = bulbs.find(target);
                if (it == bulbs.end()) {
                    ofxTALifxBulb new_bulb(udp_man);
                    std::memcpy(new_bulb.target, header.target, sizeof(header.target));
                    new_bulb.ip_address = udp_man.getIpAddress();
                    new_bulb.target64 = target;
                    bulbs.insert(pair<uint64_t, ofxTALifxBulb>(target, new_bulb));
                } else {
                    // it->second.ip_address = udp_man.getIpAddress();
                }

                ofxTALifxBulb &bulb = bulbs.find(target)->second;
                bulb.last_seen_at = ofGetElapsedTimeMillis();
                bulb.online = true;
                bulb.ip_address = udp_man.getIpAddress();
                // ofLog(OF_LOG_VERBOSE) << ofxTALifxBulb::target_to_hex(bulb.target) << " " << ofToHex(target) << " " << header.type;
                switch (header.type) {
                    case lifx::message::device::StateService::type: // STATE_SERVICE
                        lifx::message::device::StateService message;
                        memcpy(reinterpret_cast<char *>(&message), reinterpret_cast<char *>(&header.payload[0]), sizeof(message));
                        DLOG("STATE_SERVICE rcv for target " << ofxTALifxBulb::target_to_hex(bulb.target) << " " << message.port);
                        if (!bulb.label_received) {
                            bulb.GetLabel();
                        }
                        if (!bulb.group_received) {
                            bulb.GetGroup();
                        }
                        break;

                    case lifx::message::device::StateLabel::type: // STATE_LABEL
                        DLOG("STATE_LABEL rcv for target " << ofxTALifxBulb::target_to_hex(bulb.target));
                        bulb.StateLabel(header);
                        break;

                    case lifx::message::device::StateGroup::type: // STATE_GROUP
                        DLOG("STATE_GROUP rcv for target " << ofxTALifxBulb::target_to_hex(bulb.target));
                        bulb.StateGroup(header);
                        break;

                    case lifx::message::device::Acknowledgement::type: // ACKNOWLEDGMENT
                        DLOG("ACK rcv SEQ = " << int(header.sequence) << " " << ofxTALifxBulb::target_to_hex(bulb.target));
                        udp_man.ackReceived(header.sequence);
                        break;

                    default:
                        DLOG("Unhandled header.type message " << header.type);
                        break;
                }
                //}
            } // Target 0
        }
        udp_man.ackCheck();
        if (ofGetElapsedTimeMillis() - last_bulbs_dump > DUMP_DELAY)
            dumpBulbs();
        if (ofGetElapsedTimeMillis() - last_discovered > DISCOVER_DELAY || last_discovered == 0) {
            Discover();
        }
        checkOnline();
        unlock();
    }
}

// ================================================================================
// API
// ================================================================================

void ofxTALifxClient::nextColor(const string target, const float H, const float S, const float B) {
    color_states[target] = {H, S, B};
}

/*
 * Change bulb color, color is also saved for later usage by Off() On()
 *
 */

void ofxTALifxClient::Color(const string target, const float H, const float S, const float B, const uint16_t T) {
    lock();
    color_states[target] = {H, S, B};
    Color(targetedBulbs(target), H, S, B, T);
    unlock();
}

void ofxTALifxClient::Color(const bulb_vector bulbs, const float H, const float S, const float B, const uint16_t T) {
    for (auto &b : bulbs) {
        b->SetColor(H, S, B, T);
    }
}

/*
 * Change bulb's white level
 */

void ofxTALifxClient::White(const string target, const uint16_t K, const float B, const uint16_t T) {
    lock();
    White(targetedBulbs(target), K, B, T);
    unlock();
}

void ofxTALifxClient::White(const bulb_vector bulbs, const uint16_t K, const float B, const uint16_t T) {
    for (auto &b : bulbs) {
        b->SetWhite(K, B, T);
    }
}

/*
 * Set bulbs to black
 */

void ofxTALifxClient::Off(const string target) {
    lock();
    Off(targetedBulbs(target));
    unlock();
}

void ofxTALifxClient::Off(const bulb_vector bulbs) {
    for (auto &b : bulbs) {
        b->SetColor(0, 0, 0);
    }
}

/*
 * Restore color state
 */

void ofxTALifxClient::On(const string target) {
    lock();
    color_state state = color_states[target];
    On(targetedBulbs(target), state.H, state.S, state.B);
    unlock();
}

void ofxTALifxClient::On(const bulb_vector bulbs, const float H, const float S, const float B) {
    for (auto &b : bulbs) {
        b->SetColor(H, S, B);
    }
}

/*
 * Power bulbs off or on
 */

void ofxTALifxClient::powerOff(const string target) {
    lock();
    powerOff(targetedBulbs(target));
    unlock();
}

void ofxTALifxClient::powerOff(const bulb_vector bulbs) {
    for (auto &b : bulbs) {
        b->SetPower(0);
    }
}

void ofxTALifxClient::powerOn(const string target) {
    lock();
    powerOn(targetedBulbs(target));
    unlock();
}

void ofxTALifxClient::powerOn(const bulb_vector bulbs) {
    for (auto &b : bulbs) {
        b->SetPower(65535);
    }
}
