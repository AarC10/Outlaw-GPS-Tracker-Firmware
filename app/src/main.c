/*
 * Copyright (c) 2024 Aaron Chan
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gnss.h>


LOG_MODULE_REGISTER(main);

GNSS_DATA_CALLBACK_DEFINE(DEVICE_DT_GET(DT_ALIAS(gnss)), gnss_data_cb);
// GNSS_SATELLITES_CALLBACK_DEFINE(DEVICE_DT_GET(DT_ALIAS(gnss)), gnss_satellites_cb);

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


int main(void) {

}

