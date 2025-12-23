#include "state_machine.h"

#include <core/lora.h>
#include <core/gnss.h>

#include <zephyr/drivers/gnss.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/smf.h>

#define TRANSMITTER_LOGIC_LEVEL 0
#define TRANSMITTER_LED_LEVEL 0

#define RECEIVER_LOGIC_LEVEL 1
#define RECEIVER_LED_LEVEL 1

LOG_MODULE_REGISTER(state_machine);


static const struct gpio_dt_spec dip0 = GPIO_DT_SPEC_GET(DT_ALIAS(dip0), gpios);
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static void tx_timer_handler(struct k_timer* timer_id) {
    uint8_t node_id = POINTER_TO_UINT(k_timer_user_data_get(timer_id));

    static lora_payload_t payload = {0};
    payload.node_id = node_id;

    if (gnss_fix_acquired()) {
        gnss_populate_lora_payload(&payload);
        lora_tx((uint8_t*)&payload, sizeof(lora_payload_t));
    } else {
        LOG_INF("Sending NOFIX");
        lora_send_no_fix_payload(node_id);
        gpio_pin_toggle_dt(&led);
    }
}

K_TIMER_DEFINE(tx_timer, tx_timer_handler, NULL);

static const struct smf_state states[];

enum demo_state { transmitter, receiver };

struct s_object {
    struct smf_ctx ctx;
} smf_obj;

static enum smf_state_result check_for_transition(void*) {
    static int last_pin_state = -1;
    const int current_pin_state = gpio_pin_get_dt(&dip0);
    LOG_DBG("Pin state: %d", current_pin_state);

    if (current_pin_state < 0) {
        LOG_DBG("dip0 gpio read error (%d); ignoring transition check", current_pin_state);
        return SMF_EVENT_HANDLED;
    }

    if (last_pin_state != current_pin_state) {
        if (current_pin_state == TRANSMITTER_LOGIC_LEVEL) {
            smf_set_state(SMF_CTX(&smf_obj), &states[transmitter]);
        } else if (current_pin_state == RECEIVER_LOGIC_LEVEL) {
            smf_set_state(SMF_CTX(&smf_obj), &states[receiver]);
        }
    }

    last_pin_state = current_pin_state;

    return SMF_EVENT_HANDLED;
}

static void transmitter_entry(void*) {
    LOG_INF("Entering transmitter state");
    lora_set_tx();
    gpio_pin_set_dt(&led, TRANSMITTER_LED_LEVEL);
    k_timer_start(&tx_timer, K_SECONDS(5), K_SECONDS(5));
}

static void receiver_entry(void*) {
    LOG_INF("Entering receiver state");
    lora_set_rx();
    gpio_pin_set_dt(&led, RECEIVER_LED_LEVEL);
    k_timer_stop(&tx_timer);
    lora_await_rx_packet();
}

static void receiver_exit(void*) {
    lora_await_cancel();
}

static const struct smf_state states[] = {
    [transmitter] = SMF_CREATE_STATE(transmitter_entry, check_for_transition, NULL, NULL, NULL),
    [receiver] = SMF_CREATE_STATE(receiver_entry, check_for_transition, receiver_exit, NULL, NULL),
};

void state_machine_init(const uint8_t node_id) {
    k_timer_user_data_set(&tx_timer, UINT_TO_POINTER(node_id));

#ifdef CONFIG_DEFAULT_RECEIVE_MODE
    smf_set_initial(SMF_CTX(&smf_obj), &states[receiver]);
#else
    smf_set_initial(SMF_CTX(&smf_obj), &states[transmitter]);
#endif
}

int state_machine_run() {
    const int32_t ret = smf_run_state(SMF_CTX(&smf_obj));
    if (ret != 0) {
        LOG_ERR("State machine terminated with value: %d", ret);
    }
    return ret;
}
