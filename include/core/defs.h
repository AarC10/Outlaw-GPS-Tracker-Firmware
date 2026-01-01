#ifndef TYPES_H
#define TYPES_H

#define NOFIX "NOFIX"
// +1 for node id
#define NOFIX_PACKET_SIZE 6

typedef struct __attribute__((__packed__)) {
    float latitude;
    float longitude;
    uint8_t satellites_cnt;
    uint8_t fix_status;
} lora_payload_t;

// TODO: Increases payload size which means we go past 7 byte limit for
// less than 1 second TX time. need to determine if this is worth
typedef struct __attribute__((__packed__)) {
    uint8_t  version;        // 0x01
    uint8_t  origin_id;      // who created the GPS info (node_id)
    uint8_t  hop_count;      // times relayed
    // max hops?
    lora_payload_t payload;  // 7 bytes
} lora_frame_t;

#endif //TYPES_H