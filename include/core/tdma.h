#ifndef CORE_TDMA_H
#define CORE_TDMA_H

#include <stdint.h>

#define TDMA_FRAME_LEN_MS 10000
#define TDMA_SLOT_LEN_MS 1100
#define TDMA_MAX_SLOTS (TDMA_FRAME_LEN_MS / TDMA_SLOT_LEN_MS)
#define TDMA_MAX_NODES 5



void tdma_init(const uint8_t node_id);



#endif //CORE_TDMA_H