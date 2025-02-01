#ifndef GNSS_HELPER_H
#define GNSS_HELPER_H

#include <stdint.h>
#include <zephyr/drivers/gnss.h> // Might as well include here since users of this header need to use the macro for setting up callbacks

void gnss_data_cb(const struct device *dev, const struct gnss_data *data);

void gnss_satellites_cb(const struct device *dev, const struct gnss_satellite *satellites, uint16_t size);

#endif //GNSS_HELPER_H
