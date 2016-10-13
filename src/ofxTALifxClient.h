#ifndef OFXTALIFXCLIENT_H
#define OFXTALIFXCLIENT_H

#include "ofMain.h"
#include "Poco/NumberFormatter.h"

#include "defines.h"
#include "ofxTALifxBulb.h"
#include "ofxTALifxUdpManager.h"

typedef std::unordered_map<uint64_t, ofxTALifxBulb> bulb_map;
typedef bulb_map::iterator bulb_map_iterator;
typedef vector<ofxTALifxBulb> bulb_vector;

struct color_state {
    float H;
    float S;
    float B;
};

typedef vector<string> vstring_t;

class ofxTALifxClient : public ofThread {
  private:
    ofxTALifxUdpManager udpMan;
    uint64_t    last_discovered = 0;
    uint64_t    last_bulbs_dump = 0;
    bulb_map    bulbs;
    std::unordered_map<string, color_state> color_states;
    bool        fire_and_forget_mode = false;

    void dump_bulbs();
    void check_online();
    bulb_map_iterator find_bulb(string label);
    bulb_vector targeted_bulbs(string target);

    void discover();
    void off(bulb_vector bulbs);
    void on(bulb_vector bulbs);
    void color(bulb_vector bulbs, float H, float S, float B, uint16_t T);
    void white(bulb_vector bulbs, uint16_t K, float B, uint16_t T);
    void boff(bulb_vector bulbs);
    void bon(bulb_vector bulbs, float H, float S, float B);

  public:
    ofxTALifxClient();
    void threadedFunction();

    void enableFireForget() { fire_and_forget_mode = true; }
    void disableFireForget() { fire_and_forget_mode = false; }

    void nextColor(string target, float H, float S, float B);

    void Group(string groupName, vstring_t bulbs_labels);
    void unGroup(string groupName);
    void unGroupAll();

    void Color(string target, float H, float S, float B, uint16_t T = 0);

    void Off(string target);
    void On(string target);

    void White(string target, uint16_t K, float B = 1.0, uint16_t T = 0);

    void Boff(string target);
    void Bon(string target);
};

#endif // OFXTALIFXCLIENT_H
