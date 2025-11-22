#include "core/time.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(time);

static void time_pps_callback(const struct device* dev, struct gpio_callback* cb, uint32_t pins) {
    LOG_INF("PPS");
}

int time_setup_pps(const struct gpio_dt_spec* pps) {
    static struct gpio_callback button_cb_data;

    int ret = gpio_pin_interrupt_configure_dt(pps, GPIO_INT_EDGE_RISING);
    if (ret != 0) {
        LOG_ERR("Failed to configure interrupt on pin %d", pps->pin);
    }

    gpio_init_callback(&button_cb_data, time_pps_callback, BIT(pps->pin));
    ret = gpio_add_callback(pps->port, &button_cb_data);
    if (ret != 0) {
        LOG_ERR("Failed to add callback, rc=%d", ret);
        return ret;
    }

    LOG_INF("PPS callback added on pin %d", pps->pin);

    return 0;
}
