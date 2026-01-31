#ifndef CORE_GNSS_H
#define CORE_GNSS_H

#include <stdbool.h>
#include <stdint.h>

#include "core/defs.h"

struct device;
struct gnss_data;

void gnss_data_callback(const device* dev, const gnss_data* data);

bool gnss_fix_acquired();

void gnss_get_latest_data(gnss_data* out_data);

void gnss_populate_lora_payload(lora_payload_t* payload);


#endif //CORE_GNSS_H
