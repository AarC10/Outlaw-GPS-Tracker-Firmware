#pragma once

#include <stdint.h>
#include <stddef.h>

#pragma pack(push, 1)
struct GnssInfo {
    float latitude {0.0f};
    float longitude {0.0f};
    uint8_t satellites_cnt {0};
    uint8_t fix_status {0};
};

struct LoraFrame {
    uint8_t version {0x01};
    uint8_t origin_id {0};
    uint8_t hop_count {0};
    GnssInfo payload {};
};
#pragma pack(pop)

inline constexpr uint8_t NOFIX[] = "NOFIX";
inline constexpr size_t NOFIX_PACKET_SIZE = 6;
inline constexpr size_t CALLSIGN_CHAR_COUNT = 6;
inline constexpr size_t GNSS_INFO_SIZE = sizeof(GnssInfo);
inline constexpr size_t NODE_ID_SIZE = 1;
inline constexpr size_t MAX_PAYLOAD_SIZE = NODE_ID_SIZE + GNSS_INFO_SIZE + CALLSIGN_CHAR_COUNT;

