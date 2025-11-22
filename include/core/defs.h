#ifndef TYPES_H
#define TYPES_H

#define NOFIX "NOFIX"

#define LAT_LON_SCALING_FACTOR 10000.0f

typedef struct __attribute__((__packed__)) {
    int16_t latitude_scaled;
    int16_t longitude_scaled;
    uint8_t satellites_cnt;
    uint8_t fix_status;
    uint8_t node_id;
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