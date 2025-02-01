/*
 * Copyright (c) 2024 Aaron Chan
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gnss.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/smf.h>

LOG_MODULE_REGISTER(main);

GNSS_DATA_CALLBACK_DEFINE(DEVICE_DT_GET(DT_ALIAS(gnss)), gnss_data_callback);

typedef enum {
    TRANSMITTER,
    RECEIVER
} OutlawMode;

static OutlawMode mode = TRANSMITTER;
static struct device *lora_dev = DEVICE_DT_GET(DT_ALIAS(lora));

static void gnss_data_callback(const struct device *dev, const struct gnss_data *data) {
    if (mode == RECEIVER) {
        return;
    }

    if (data->info.fix_status != GNSS_FIX_STATUS_NO_FIX) {
        LOG_INF("Fix acquired!");
        lora_send_async(lora_dev, (uint8_t *) data, sizeof(*data), NULL);
    } else {
        LOG_INF("No fix acquired!");
        lora_send_async(lora_dev, "NOFIX", 5, NULL);
    }
}

static void lora_receive_callback(const struct device *dev, uint8_t *data, uint16_t size,
                         int16_t rssi, int8_t snr, void *user_data) {
    if (mode == TRANSMITTER) {
        return;
    }

    if (size == sizeof(struct gnss_data)) {
        struct gnss_data *gnss_data = (struct gnss_data *) data;
        LOG_INF("Received GNSS data:");
        LOG_INF("Latitude: %lld", gnss_data->nav_data.latitude);
        LOG_INF("Longitude: %lld", gnss_data->nav_data.longitude);
        LOG_INF("Bearing: %u", gnss_data->nav_data.bearing);
        LOG_INF("Speed: %u", gnss_data->nav_data.speed);
        LOG_INF("Altitude: %d", gnss_data->nav_data.altitude);
    } else {
        LOG_INF("Received data: %s", data);
    }
}


static const struct smf_state states[];

enum demo_state { transmitter, receiver};

struct s_object {
    struct smf_ctx ctx;
} s_obj;

/* State transmitter */
static void transmitter_entry(void *o) {

}

static void receiver_entry(void *o) {
}


static const struct smf_state states[] = {
    [transmitter] = SMF_CREATE_STATE(transmitter_entry, NULL, NULL, NULL, NULL),
    [receiver] = SMF_CREATE_STATE(receiver_entry, NULL, NULL, NULL, NULL),
};

int main(void)
{
    smf_set_initial(SMF_CTX(&s_obj), &states[transmitter]);

    while (true) {
        int32_t ret = smf_run_state(SMF_CTX(&s_obj));
        if (ret) {
            LOG_WRN("SMF returned non-zero status: %d", ret);
        }
        k_msleep(1000);
    }

    return 0;
}
