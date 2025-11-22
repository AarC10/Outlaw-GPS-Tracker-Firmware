
#ifndef CORE_TIME_H
#define CORE_TIME_H

#include <stdint.h>

struct gpio_dt_spec;
int time_setup_pps(const struct gpio_dt_spec* pps_pin);

uint32_t time_get_gps_seconds();


#endif //CORE_TIME_H
