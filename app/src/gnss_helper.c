#include "gnss_helper.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gnss.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(gnss_helper);

void gnss_data_cb(const struct device *dev, const struct gnss_data *data) {
    if (data->info.fix_status != GNSS_FIX_STATUS_NO_FIX) {
        LOG_INF("Fix acquired!");
    } else {
        LOG_INF("No fix acquired!");
    }
}


void gnss_satellites_cb(const struct device *dev, const struct gnss_satellite *satellites, uint16_t size) {
    LOG_INF("Locked with %u satellites!", dev->name, size);
}
