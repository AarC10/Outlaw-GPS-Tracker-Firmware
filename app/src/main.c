/*
 * Copyright (c) 2024 Aaron Chan
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gnss.h>
#include <zephyr/drivers/lora.h>


LOG_MODULE_REGISTER(main);

GNSS_DATA_CALLBACK_DEFINE(DEVICE_DT_GET(DT_ALIAS(gnss)), gnss_data_cb);
// GNSS_SATELLITES_CALLBACK_DEFINE(DEVICE_DT_GET(DT_ALIAS(gnss)), gnss_satellites_cb);

static struct device *lora_dev = DEVICE_DT_GET(DT_ALIAS(lora));

void gnss_data_cb(const struct device *dev, const struct gnss_data *data) {
    if (data->info.fix_status != GNSS_FIX_STATUS_NO_FIX) {
        LOG_INF("Fix acquired!");
        lora_send_async(lora_dev, (uint8_t *) data, sizeof(*data), NULL);
    } else {
        LOG_INF("No fix acquired!");
        lora_send_async(lora_dev, "NOFIX", 5, NULL);
    }
}


void gnss_satellites_cb(const struct device *dev, const struct gnss_satellite *satellites, uint16_t size) {
    LOG_INF("Locked with %u satellites!", dev->name, size);
}


int main(void) {

}

