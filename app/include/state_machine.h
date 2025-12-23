#ifndef OUTLAW_STATE_MACHINE_H
#define OUTLAW_STATE_MACHINE_H
#include <stdint.h>

void state_machine_init(const uint8_t node_id);
int state_machine_run();

#endif //OUTLAW_STATE_MACHINE_H