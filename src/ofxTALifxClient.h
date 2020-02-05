#ifndef OFXTALIFXCLIENT_H
#define OFXTALIFXCLIENT_H

#include "Poco/Condition.h"
#include "Poco/NumberFormatter.h"
#include "ofMain.h"

#include "defines.h"
#include "ofxTALifxBulb.h"
#include "ofxTALifxUdpManager.h"

namespace ofxtalifx {

    struct color_state {
        float H;
        float S;
        float B;
    };

    typedef std::unordered_map<uint64_t, ofxTALifxBulb> bulb_map;
    typedef bulb_map::iterator bulb_map_iterator;
    typedef vector<ofxTALifxBulb *> bulb_vector;
    typedef vector<string> vstring_t;
    typedef std::unordered_map<string, bulb_vector> group_map;
    typedef group_map::iterator group_map_iterator;
    typedef std::unordered_map<string, color_state> color_states_map;

    class ofxTALifxClient : public ofThread {
      private:
        ofxTALifxUdpManager udp_man;
        uint64_t last_discovered = 0;
        uint64_t last_bulbs_dump = 0;
        bulb_map bulbs;
        color_states_map color_states;
        group_map groups;

        void dumpBulbs();
        void checkOnline();
        bulb_map_iterator findBulb(const string label);
        const bulb_vector targetedBulbs(const string target);

        void powerOff(const bulb_vector bulbs);
        void powerOn(const bulb_vector bulbs);
        void Color(const bulb_vector bulbs, const float H, const float S, const float B, const uint16_t T);
        void White(const bulb_vector bulbs, const uint16_t K, const float B, const uint16_t T);
        void Off(const bulb_vector bulbs);
        void On(const bulb_vector bulbs, const float H, const float S, const float B);

      public:
        ofxTALifxClient();
        virtual ~ofxTALifxClient();
        void threadedFunction();

        void Discover();

        void enableAck() {
            udp_man.enableAck();
        }
        void disableAck() {
            udp_man.disableAck();
        }

        bulb_map getBulbs();

        void setGroup(const string groupName, const vstring_t bulbs_labels);
        void addToGroup(const string groupName, const string label);
        void removeFromGroup(const string groupName, const string label);
        void removeGroup(const string groupName);
        void removeAllGroups();
        const group_map getGroups();

        const color_states_map getColorStates() {
            return color_states;
        }
        void clearColorStates() {
            color_states.clear();
        }

        void nextColor(const string target, const float H, const float S, const float B);
        void Color(const string target, const float H, const float S, const float B, const uint16_t T = 0);
        void White(const string target, const uint16_t K, const float B = 1.0, const uint16_t T = 0);
        void Off(const string target);
        void On(const string target);
        void powerOff(const string target);
        void powerOn(const string target);
    };
}

#endif // OFXTALIFXCLIENT_H
