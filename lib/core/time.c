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

    gpio_init_callback(&button_cb_data, time_pps_callback, BIT(pps->pin));
    return gpio_add_callback(pps->port, &button_cb_data);
}
