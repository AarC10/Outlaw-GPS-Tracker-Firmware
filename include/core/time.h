
#ifndef CORE_TIME_H
#define CORE_TIME_H

struct gpio_dt_spec;
int time_setup_pps(const struct gpio_dt_spec* pps_pin);




#endif //CORE_TIME_H