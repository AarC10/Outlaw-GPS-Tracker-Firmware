#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "core/defs.h"

struct device;
struct gnss_data;

void gnss_data_callback(const struct device* dev, const struct gnss_data* data);

bool gnss_fix_acquired();

void gnss_get_latest_data(struct gnss_data* out_data);

void gnss_populate_lora_payload(lora_payload_t* payload);
