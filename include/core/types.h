#ifndef TYPES_H
#define TYPES_H

#define NOFIX "NOFIX"

typedef struct {
    uint8_t node_id;
    float latitude;
    float longitude;
    uint16_t altitude;
    uint16_t satellites_cnt;
    uint32_t speed;
} lora_payload_t;


#endif //TYPES_H