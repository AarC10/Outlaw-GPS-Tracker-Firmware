/*
 * Copyright (c) 2024 Aaron Chan
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/drivers/gpio.h>
#include <stdbool.h>

#define NOFIX "NOFIX"

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);


typedef struct {
    uint8_t node_id;
    float latitude;
    float longitude;
    uint16_t altitude;
    uint16_t satellites_cnt;
    uint32_t speed;
} lora_payload_t;

// ******************************************** //
// *                LoRa                      * //
// ******************************************** //

static const struct device* lora_dev = DEVICE_DT_GET(DT_ALIAS(lora));

static struct lora_modem_config lora_configuration = {
    .frequency = 903000000,
    .bandwidth = BW_125_KHZ,
    .datarate = SF_12,
    .coding_rate = CR_4_5,
    .preamble_len = 8,
    .tx_power = 20,
    .tx = false,
    .iq_inverted = false,
    .public_network = false,
};

static void lora_receive_callback(const struct device* dev, uint8_t* data, uint16_t size, int16_t rssi, int8_t snr,
                                  void* user_data) {
    if (lora_configuration.tx) return;

    printk("Packet received (%d bytes | %d dBm | %d dB:", size, rssi, snr);

    switch (size) {
    case sizeof(struct gnss_data): {
        struct gnss_data gnss_data_local;
        memcpy(&gnss_data_local, data, sizeof(gnss_data_local));
        printk("\tLatitude: %lld", gnss_data_local.nav_data.latitude);
        printk("\tLongitude: %lld", gnss_data_local.nav_data.longitude);
        printk("\tBearing: %u", gnss_data_local.nav_data.bearing);
        printk("\tSpeed: %u", gnss_data_local.nav_data.speed);
        printk("\tAltitude: %d", gnss_data_local.nav_data.altitude);
        break;
    }
    case sizeof(lora_payload_t): {
        lora_payload_t lora_payload_local;
        memcpy(&lora_payload_local, data, sizeof(lora_payload_local));
        printk("\tNode ID: %d", lora_payload_local.node_id);
        printk("\tLatitude: %f", lora_payload_local.latitude);
        printk("\tLongitude: %f", lora_payload_local.longitude);
        printk("\tAltitude: %d m", lora_payload_local.altitude);
        printk("\tSpeed: %d cm/s", lora_payload_local.speed);
        printk("\tSatellites: %d", lora_payload_local.satellites_cnt);
        break;
    }
    case strlen(NOFIX):
        printk("\tNo fix acquired!");
        break;
    default:
        printk("\tReceived data: %s", data);
        break;
    }
}

static void receiver_entry() {
    lora_configuration.tx = false;
    lora_config(lora_dev, &lora_configuration);
    gpio_pin_set_dt(&led, RECEIVER_LED_LEVEL);
}

static void receiver_run() {
    lora_recv_async(lora_dev, lora_receive_callback, NULL);
}


// ******************************************** //
// *                  PPS                     * //
// ******************************************** //

// ******************************************** //
// *                  Main                    * //
// ******************************************** //

int main(void) {
    receiver_entry();
    while (true) {
        receiver_run();
        k_msleep(100);
    }

    return 0;
}
