#include "core/time.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(time);

static void time_pps_callback(const struct device* dev, struct gpio_callback* cb, uint32_t pins) {
    LOG_INF("PPS");
}

int time_setup_pps(const struct gpio_dt_spec* pps) {
    static struct gpio_callback pps_cb;

    if (!pps || !pps->port) {
        LOG_ERR("Invalid PPS gpio_dt_spec");
        return -EINVAL;
    }

    if (!device_is_ready(pps->port)) {
        LOG_ERR("PPS GPIO device not ready");
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(pps, GPIO_INPUT | GPIO_PULL_UP);
    if (ret) {
        LOG_ERR("Failed to configure PPS pin %d, rc=%d", pps->pin, ret);
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(pps, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret) {
        LOG_ERR("Failed to configure interrupt on pin %d, rc=%d", pps->pin, ret);
        return ret;
    }

    gpio_init_callback(&pps_cb, time_pps_callback, BIT(pps->pin));
    ret = gpio_add_callback(pps->port, &pps_cb);
    if (ret) {
        LOG_ERR("Failed to add callback, rc=%d", ret);
        return ret;
    }

    LOG_INF("PPS callback added on pin %d", pps->pin);
    return 0;
}
