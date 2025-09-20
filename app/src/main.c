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
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log_ctrl.h>
#include <stdbool.h>

#define NOFIX "NOFIX"

#define TRANSMITTER_LOGIC_LEVEL 1
#define TRANSMITTER_LED_LEVEL 0

#define RECEIVER_LOGIC_LEVEL 0
#define RECEIVER_LED_LEVEL 1
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

LOG_MODULE_REGISTER(main);

typedef struct {
    uint8_t node_id;
    float latitude;
    float longitude;
    uint16_t altitude;
    uint16_t satellites_cnt;
    uint32_t speed;
} lora_payload_t;

volatile int pps_counter = 0;
volatile int no_fix_counter = 0;

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

    LOG_INF("Packet received (%d bytes | %d dBm | %d dB:", size, rssi, snr);

    switch (size) {
    case sizeof(struct gnss_data): {
        struct gnss_data gnss_data_local;
        memcpy(&gnss_data_local, data, sizeof(gnss_data_local));
        LOG_INF("\tLatitude: %lld", gnss_data_local.nav_data.latitude);
        LOG_INF("\tLongitude: %lld", gnss_data_local.nav_data.longitude);
        LOG_INF("\tBearing: %u", gnss_data_local.nav_data.bearing);
        LOG_INF("\tSpeed: %u", gnss_data_local.nav_data.speed);
        LOG_INF("\tAltitude: %d", gnss_data_local.nav_data.altitude);
        break;
    }
    case strlen(NOFIX):
        LOG_INF("\tNo fix acquired!");
        break;
    default:
        LOG_INF("\tReceived data: %s", data);
        break;
    }
}

// ******************************************** //
// *                GNSS                      * //
// ******************************************** //

#pragma GCC diagnostic ignored "-Wunused-function"
static void gnss_data_callback(const struct device* dev, const struct gnss_data* data) {
#pragma GCC diagnostic pop
    if (!lora_configuration.tx) return;

    if (data->info.fix_status != GNSS_FIX_STATUS_NO_FIX) {
        LOG_INF("Fix acquired!");

        no_fix_counter = 0;
        pps_counter = 0;

        static const int TX_INTERVAL = CONFIG_GPS_TRANSMIT_INTERVAL + CONFIG_TIMESLOT;
        const lora_payload_t payload = {
            .node_id = 0,
            .latitude = (float) (data->nav_data.latitude / 1e7),
            .longitude = (float) (data->nav_data.longitude / 1e7),
            .altitude = (uint16_t)(data->nav_data.altitude / 100),
            .speed = (uint16_t)(data->nav_data.speed / 100),
            .satellites_cnt = data->info.satellites_cnt,
        };

        if (TX_INTERVAL == pps_counter) {
            LOG_INF("Sending GPS transmission");
            lora_send_async(lora_dev, (uint8_t*)&payload, sizeof(lora_payload_t), NULL);
        } else if (pps_counter < TX_INTERVAL) {
            LOG_INF("Missed transmission window, current pps_counter: %d", pps_counter);
        }
    } else {
        LOG_INF("No fix acquired!");

        no_fix_counter++;
        gpio_pin_toggle_dt(&led);

        if (no_fix_counter == CONFIG_GPS_TRANSMIT_INTERVAL) {
            lora_send_async(lora_dev, NOFIX, strlen(NOFIX), NULL);
        }
    }
}

GNSS_DATA_CALLBACK_DEFINE(DEVICE_DT_GET(DT_ALIAS(gnss)), gnss_data_callback);

// ******************************************** //
// *             State Machine                * //
// ******************************************** //

// static const struct gpio_dt_spec pin_sw = GPIO_DT_SPEC_GET(DT_ALIAS(pin_sw), gpios);

static const struct smf_state states[];

enum demo_state { transmitter, receiver };

struct s_object {
    struct smf_ctx ctx;
} smf_obj;

// static void check_for_transition(void*) {
//     static int last_pin_state = -1;
//     const int current_pin_state = gpio_pin_get_dt(&pin_sw);
//     LOG_INF("Pin state: %d", current_pin_state);
//     if (last_pin_state != current_pin_state) {
//         last_pin_state = current_pin_state;
//
//         if (current_pin_state == TRANSMITTER_LOGIC_LEVEL) {
//             smf_set_state(SMF_CTX(&smf_obj), &states[transmitter]);
//         } else if (current_pin_state == RECEIVER_LOGIC_LEVEL) {
//             smf_set_state(SMF_CTX(&smf_obj), &states[receiver]);
//         }
//     }
// }

static void transmitter_entry(void*) {
    lora_configuration.tx = true;
    lora_config(lora_dev, &lora_configuration);
    gpio_pin_set_dt(&led, TRANSMITTER_LED_LEVEL);
}

static void receiver_entry(void*) {
    lora_configuration.tx = false;
    lora_config(lora_dev, &lora_configuration);
    gpio_pin_set_dt(&led, RECEIVER_LED_LEVEL);
}

static void receiver_run(void*) {
    lora_recv_async(lora_dev, lora_receive_callback, NULL);
}


static const struct smf_state states[] = {
    // [transmitter] = SMF_CREATE_STATE(transmitter_entry, check_for_transition, NULL, NULL, NULL),
    // TODO: V2 currently has a floating pin, making it impossible to detect the state of the pin properly
    [transmitter] = SMF_CREATE_STATE(transmitter_entry, NULL, NULL, NULL, NULL),
    [receiver] = SMF_CREATE_STATE(receiver_entry, receiver_run, NULL, NULL, NULL),
};

// ******************************************** //
// *                  PPS                     * //
// ******************************************** //

static const struct gpio_dt_spec pps = GPIO_DT_SPEC_GET(DT_ALIAS(pps), gpios);
static struct gpio_callback pps_cb_data;
static volatile bool pps_triggered = false;

static void pps_callback(const struct device* dev, struct gpio_callback* cb, uint32_t pins) {
    ++pps_counter;
    LOG_INF("PPS triggered");
}

static int pps_init(void) {
    if (!device_is_ready(pps.port)) {
        LOG_ERR("PPS GPIO device not ready");
        return -ENODEV;
    }
    int ret = gpio_pin_configure_dt(&pps, GPIO_INPUT);
    if (ret != 0) {
        LOG_ERR("Failed to configure PPS pin");
        return ret;
    }
    ret = gpio_pin_interrupt_configure_dt(&pps, GPIO_INT_EDGE_RISING);
    if (ret != 0) {
        LOG_ERR("Failed to configure PPS interrupt");
        return ret;
    }
    gpio_init_callback(&pps_cb_data, pps_callback, BIT(pps.pin));
    gpio_add_callback(pps.port, &pps_cb_data);
    LOG_INF("PPS GPIO interrupt configured");
    return 0;
}

// ******************************************** //
// *                  Main                    * //
// ******************************************** //

int main(void) {
    int pps_status = pps_init();
    if (pps_status != 0) {
        LOG_ERR("PPS initialization failed: %d", pps_status);
    }

#ifdef CONFIG_DEFAULT_RECEIVE_MODE
    smf_set_initial(SMF_CTX(&smf_obj), &states[receiver]);
#else
    smf_set_initial(SMF_CTX(&smf_obj), &states[transmitter]);
#endif

    while (true) {
        const int32_t ret = smf_run_state(SMF_CTX(&smf_obj));
        if (ret) {
            LOG_INF("SMF returned non-zero status: %d", ret);
        }
        k_msleep(100);
    }

    return 0;
}
