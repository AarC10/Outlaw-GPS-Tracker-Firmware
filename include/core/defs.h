#pragma once

#include <stdint.h>
#include <stddef.h>

namespace core {

constexpr char NOFIX[] = "NOFIX";
constexpr size_t NOFIX_PACKET_SIZE = 6; // includes node_id prefix

#pragma pack(push, 1)
struct LoraPayload {

    float latitude {0.0f};
    float longitude {0.0f};
    uint8_t satellites_cnt {0};
    uint8_t fix_status {0};
};

struct LoraFrame {
    uint8_t version {0x01};
    uint8_t origin_id {0};
    uint8_t hop_count {0};
    LoraPayload payload {};
};
#pragma pack(pop)

} // namespace core

// Backwards-compatible aliases for existing C-style code
inline constexpr auto NOFIX = core::NOFIX;
inline constexpr size_t NOFIX_PACKET_SIZE = core::NOFIX_PACKET_SIZE;
using lora_payload_t = core::LoraPayload;
using lora_frame_t = core::LoraFrame;
