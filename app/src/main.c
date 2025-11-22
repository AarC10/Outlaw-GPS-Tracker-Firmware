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
#include <zephyr/logging/log_ctrl.h>
#include <core/lora.h>
#include <core/types.h>


#define TRANSMITTER_LOGIC_LEVEL 1
#define TRANSMITTER_LED_LEVEL 0

#define RECEIVER_LOGIC_LEVEL 0
#define RECEIVER_LED_LEVEL 1
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

LOG_MODULE_REGISTER(main);


static void tx_timer_handler(struct k_timer* timer_id);
K_TIMER_DEFINE(tx_timer, tx_timer_handler, NULL);

// ******************************************** //
// *                GNSS                      * //
// ******************************************** //
// GNSS data storage for transmission
static struct gnss_data latest_gnss_data;

#pragma GCC diagnostic ignored "-Wunused-function"
static void gnss_data_callback(const struct device* dev, const struct gnss_data* data) {
#pragma GCC diagnostic pop
    if (!lora_is_tx()) return;

    memcpy(&latest_gnss_data, data, sizeof(latest_gnss_data));
    if (data->info.fix_status != GNSS_FIX_STATUS_NO_FIX) {
        LOG_INF("Fix acquired!");
    } else {
        LOG_INF("No fix acquired!");
        gpio_pin_toggle_dt(&led);
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
    lora_set_tx();
    gpio_pin_set_dt(&led, TRANSMITTER_LED_LEVEL);
    k_timer_start(&tx_timer, K_SECONDS(5), K_SECONDS(5));
}

static void receiver_entry(void*) {
    lora_set_rx();
    gpio_pin_set_dt(&led, RECEIVER_LED_LEVEL);
    k_timer_stop(&tx_timer);
}

static enum smf_state_result receiver_run(void*) {
    lora_await_rx_packet();

    return SMF_EVENT_HANDLED;
}

static const struct smf_state states[] = {
    // [transmitter] = SMF_CREATE_STATE(transmitter_entry, check_for_transition, NULL, NULL, NULL),
    // TODO: V2 currently has a floating pin, making it impossible to detect the state of the pin properly
    [transmitter] = SMF_CREATE_STATE(transmitter_entry, NULL, NULL, NULL, NULL),
    [receiver] = SMF_CREATE_STATE(receiver_entry, receiver_run, NULL, NULL, NULL),
};




static void tx_timer_handler(struct k_timer* timer_id) {
    if (latest_gnss_data.info.fix_status != GNSS_FIX_STATUS_NO_FIX) {
        LOG_INF("Fix acquired! (Timer)");
        lora_tx((uint8_t*)&latest_gnss_data, sizeof(latest_gnss_data));
    } else {
        LOG_INF("No fix acquired! (Timer)");
        lora_tx(NOFIX, strlen(NOFIX));
        gpio_pin_toggle_dt(&led);
    }
}

// ******************************************** //
// *                  Main                    * //
// ******************************************** //
int main(void) {
    if (!device_is_ready(led.port)) {
        LOG_ERR("LED GPIO device not ready\n");
    }

#ifdef CONFIG_DEFAULT_RECEIVE_MODE
    smf_set_initial(SMF_CTX(&smf_obj), &states[receiver]);
#else
    lora_set_tx();
    smf_set_initial(SMF_CTX(&smf_obj), &states[transmitter]);
#endif

    while (true) {
        const int32_t ret = smf_run_state(SMF_CTX(&smf_obj));
        if (ret) {
            LOG_INF("SMF returned non-zero status: %d", ret);
        }
        k_msleep(1000);
    }

    return 0;
}
