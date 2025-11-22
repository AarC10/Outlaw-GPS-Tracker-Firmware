#ifndef CORE_GNSS_H
#define CORE_GNSS_H

#include <stdbool.h>

// Forward Declares
struct device;
struct gnss_data;

void gnss_data_callback(const struct device* dev, const struct gnss_data* data);

bool gnss_fix_acquired();

void gnss_get_latest_data(struct gnss_data* out_data);

#endif //CORE_GNSS_H
