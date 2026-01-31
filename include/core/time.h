#pragma once

#include <stdint.h>

struct gpio_dt_spec;
int time_setup_pps(const gpio_dt_spec* pps_pin);

uint32_t time_get_gps_seconds();
