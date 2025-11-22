#ifndef CORE_GNSS_H
#define CORE_GNSS_H

#include <stdbool.h>
#include <stdint.h>

// Forward Declares
struct device;
struct gnss_data;

void gnss_data_callback(const struct device* dev, const struct gnss_data* data);

bool gnss_fix_acquired();

void gnss_get_latest_data(struct gnss_data* out_data);

int16_t gnss_get_latitude_scaled();

int16_t gnss_get_longitude_scaled();

uint8_t gnss_get_satellites_cnt();

uint8_t gnss_get_fix_status();

#endif //CORE_GNSS_H
