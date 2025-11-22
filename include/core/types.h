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

#endif //TYPES_H