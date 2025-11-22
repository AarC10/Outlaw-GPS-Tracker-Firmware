/*
 * Copyright (c) 2024 Aaron Chan
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gnss.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log_ctrl.h>
#include <core/lora.h>
#include <core/time.h>
#include <core/defs.h>
#include <core/tdma.h>
#include <core/gnss.h>

#include "state_machine.h"

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);


LOG_MODULE_REGISTER(main);


GNSS_DATA_CALLBACK_DEFINE(DEVICE_DT_GET(DT_ALIAS(gnss)), gnss_data_callback);


// ******************************************** //
// *                  Main                    * //
// ******************************************** //
int main(void) {
    static const struct gpio_dt_spec pps_spec = GPIO_DT_SPEC_GET(DT_ALIAS(pps), gpios);
    if (!device_is_ready(led.port)) {
        LOG_ERR("LED GPIO device not ready\n");
    }

    state_machine_init();
    lora_init();
    time_setup_pps(&pps_spec);


    while (true) {
        state_machine_run();
        k_msleep(1000);
    }

    return 0;
}

