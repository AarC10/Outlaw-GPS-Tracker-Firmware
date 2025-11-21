#ifndef TYPES_H
#define TYPES_H

#define NOFIX "NOFIX"

typedef struct __attribute__((__packed__)) {
    int16_t latitude_scaled;
    int16_t longitude_scaled;
    uint16_t altitude;
    uint16_t speed;
    uint8_t satellites_cnt;
    uint8_t fix_status;
    uint8_t node_id;
} lora_payload_t;



#endif //TYPES_H