#ifndef CORE_GNSS_H
#define CORE_GNSS_H

#include <stdbool.h>
#include <stdint.h>

#include "core/defs.h"

// Forward Declares
struct device;
struct gnss_data;
struct lora_payload_t;

void gnss_data_callback(const struct device* dev, const struct gnss_data* data);

bool gnss_fix_acquired();

void gnss_get_latest_data(struct gnss_data* out_data);

int16_t gnss_get_latitude_scaled();

int16_t gnss_get_longitude_scaled();

void gnss_populate_lora_payload(lora_payload_t* payload);


#endif //CORE_GNSS_H
