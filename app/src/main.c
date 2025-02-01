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

#define NOFIX "NOFIX"

LOG_MODULE_REGISTER(main);
GNSS_DATA_CALLBACK_DEFINE(DEVICE_DT_GET(DT_ALIAS(gnss)), gnss_data_callback);

static const struct device* lora_dev = DEVICE_DT_GET(DT_ALIAS(lora));

static struct lora_modem_config lora_configuration = {
    .frequency = 915000000,
    .bandwidth = BW_125_KHZ,
    .datarate = SF_12,
    .coding_rate = CR_4_5,
    .preamble_len = 8,
    .tx_power = 13,
    .tx = false,
    .iq_inverted = false,
    .public_network = false,
};

static const struct smf_state states[];

enum demo_state { transmitter, receiver };

struct s_object {
    struct smf_ctx ctx;
} smf_obj;

static void gnss_data_callback(const struct device* dev, const struct gnss_data* data) {
    if (!lora_configuration.tx) return;

    if (data->info.fix_status != GNSS_FIX_STATUS_NO_FIX) {
        LOG_INF("Fix acquired!");
        lora_send_async(lora_dev, (uint8_t*)data, sizeof(*data), NULL);
    } else {
        LOG_INF("No fix acquired!");
        lora_send_async(lora_dev, NOFIX, strlen(NOFIX), NULL);
    }
}

static void lora_receive_callback(const struct device* dev, uint8_t* data, uint16_t size, int16_t rssi, int8_t snr,
                                  void* user_data) {
    if (lora_configuration.tx) return;

    LOG_INF("Packet received (%d bytes | %d dBm | %d dB:", size, rssi, snr);

    switch (size) {
    case sizeof(struct gnss_data): {
        struct gnss_data* gnss_data = (struct gnss_data*)data;
        LOG_INF("\tLatitude: %lld", gnss_data->nav_data.latitude);
        LOG_INF("\tLongitude: %lld", gnss_data->nav_data.longitude);
        LOG_INF("\tBearing: %u", gnss_data->nav_data.bearing);
        LOG_INF("\tSpeed: %u", gnss_data->nav_data.speed);
        LOG_INF("\tAltitude: %d", gnss_data->nav_data.altitude);
        break;
    }
    case strlen(NOFIX):
        LOG_INF("No fix acquired!");
        break;
    default:
        LOG_INF("Received data: %s", data);
        break;
    }
}


static void transmitter_entry(void* o) {
    lora_configuration.tx = true;
    lora_config(lora_dev, &lora_configuration);
}

static void receiver_entry(void* o) {
    lora_configuration.tx = false;
    lora_config(lora_dev, &lora_configuration);
}

static void receiver_run(void* o) {
    lora_recv_async(lora_dev, lora_receive_callback, NULL);
}


static const struct smf_state states[] = {
    [transmitter] = SMF_CREATE_STATE(transmitter_entry, NULL, NULL, NULL, NULL),
    [receiver] = SMF_CREATE_STATE(receiver_entry, receiver_run, NULL, NULL, NULL),
};

int main(void) {
    smf_set_initial(SMF_CTX(&smf_obj), &states[transmitter]);

    while (true) {
        const int32_t ret = smf_run_state(SMF_CTX(&smf_obj));
        if (ret) {
            LOG_WRN("SMF returned non-zero status: %d", ret);
        }
        k_msleep(1000);
    }

    return 0;
}
