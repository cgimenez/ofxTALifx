//
// From https://raw.githubusercontent.com/codemaster/lib-lifx/master/include/lib-lifx/lifx_messages.h
//

#ifndef OFX_TA_LIFX_PROTOCOL_H
#define OFX_TA_LIFX_PROTOCOL_H

#include <stdint.h>

namespace lifx {
#pragma pack(push, 1)
typedef struct {
    // Frame - 64
    uint16_t  size;
    uint16_t  protocol: 12;
    uint8_t   addressable: 1;
    uint8_t   tagged: 1;
    uint8_t   origin: 2;
    uint32_t  source;
    // Frame Address - 128
    uint8_t   target[8];
    uint8_t   site[6];
    uint8_t   ack_required: 1;
    uint8_t   res_required: 1;
    uint8_t   :6;
    uint8_t   sequence;
    // Protocol Header - 96
    uint64_t  at_time;
    uint16_t  type;
    uint16_t  :16;
    uint8_t   payload[4096];
} Header;

typedef Header NetworkHeader;

typedef struct {
    uint16_t  hue;
    uint16_t  saturation;
    uint16_t  brightness;
    uint16_t  kelvin;
} HSBK;
#pragma pack(pop)


namespace message {

enum types {
    state_service = 3,
    state_host_info = 15,
    state_power = 20,
    state_label = 25,
    acknowlegment = 45,
    state_group = 51
};

#pragma pack(push, 1)

namespace device {
struct GetService {
    static constexpr uint16_t type = 2;
};

struct StateService {
    static constexpr uint16_t type = 3;
    uint8_t service;
    uint32_t port;
};

struct GetHostInfo {
    static constexpr uint16_t type = 12;
};

struct StateHostInfo {
    static constexpr uint16_t type = 13;
    float signal;
    uint32_t tx;
    uint32_t rx;
    int16_t:16;
};

struct GetHostFirmware {
    static constexpr uint16_t type = 14;
};

struct StateHostFirmware {
    static constexpr uint16_t type = 15;
    uint64_t build;
    uint64_t:64;
    uint32_t version;
};

struct GetWifiInfo {
    static constexpr uint16_t type = 16;
};

struct StateWifiInfo {
    static constexpr uint16_t type = 17;
    float signal;
    uint32_t tx;
    uint32_t rx;
    int16_t:16;
};

struct GetWifiFirmware {
    static constexpr uint16_t type = 18;
};

struct StateWifiFirmware {
    static constexpr uint16_t type = 19;
    uint64_t build;
    uint64_t:64;
    uint32_t version;
};

struct GetPower {
    static constexpr uint16_t type = 20;
};

struct SetPower {
    static constexpr uint16_t type = 21;
    uint16_t level;
};

struct StatePower {
    static constexpr uint16_t type = 22;
    uint16_t level;
};

struct GetLabel {
    static constexpr uint16_t type = 23;
};

struct SetLabel {
    static constexpr uint16_t type = 24;
    char label[32];
};

struct StateLabel {
    static constexpr uint16_t type = 25;
    char label[32];
};

struct GetVersion {
    static constexpr uint16_t type = 32;
};

struct StateVersion {
    static constexpr uint16_t type = 33;
    uint32_t vendor;
    uint32_t product;
    uint32_t version;
};

struct GetInfo {
    static constexpr uint16_t type = 34;
};

struct StateInfo {
    static constexpr uint16_t type = 35;
    uint64_t time;
    uint64_t uptime;
    uint64_t downtime;
};

struct Acknowledgement {
    static constexpr uint16_t type = 45;
};

struct GetLocation {
    static constexpr uint16_t type = 48;
};

struct StateLocation {
    static constexpr uint16_t type = 50;
    uint8_t location[16];
    char label[32];
    uint64_t updated_at;
};

struct GetGroup {
    static constexpr uint16_t type = 51;
};

struct StateGroup {
    static constexpr uint16_t type = 53;
    uint8_t group[16];
    char label[32];
    uint64_t updated_at;
};

struct EchoRequest {
    static constexpr uint16_t type = 58;
    uint64_t payload;
};

struct EchoResponse {
    static constexpr uint16_t type = 59;
    uint64_t payload;
};
} // namespace device

namespace light {
struct Get {
    static constexpr uint16_t type = 101;
};

struct SetColor {
    static constexpr uint16_t type = 102;
    uint8_t:8;
    HSBK color;
    uint32_t duration;
};

struct State {
    static constexpr uint16_t type = 107;
    HSBK color;
    int16_t:16;
    uint16_t power;
    char label[32];
    uint64_t:64;
};

struct GetPower {
    static constexpr uint16_t type = 116;
};

struct SetPower {
    static constexpr uint16_t type = 117;
    uint16_t level;
    uint32_t duration;
};

struct StatePower {
    static constexpr uint16_t type = 118;
    uint16_t level;
};
} // namespace light

#pragma pack(pop)
} // namespace message
} // namespace lifx

#endif // OFX_TA_LIFX_PROTOCOL_H
