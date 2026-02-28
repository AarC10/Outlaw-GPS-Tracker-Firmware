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
#include <core/time.h>
#include <core/defs.h>
#include <core/tdma.h>
#include <core/LoraTransceiver.h>
#include <core/HamCallsign.h>
#include "state_machine.h"
#include <core/GnssReceiver.h>
#include <core/OutlawSettings.h>

LOG_MODULE_REGISTER(main);
GNSS_DATA_CALLBACK_DEFINE(DEVICE_DT_GET(DT_ALIAS(gnss)), gnssCallback);

int main() {
    static const gpio_dt_spec pps_spec = GPIO_DT_SPEC_GET(DT_ALIAS(pps), gpios);
    static const gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
    if (!device_is_ready(led.port)) {
        LOG_ERR("LED GPIO device not ready\n");
    }

    uint8_t nodeId = 0;
    OutlawSettings::load();
    StateMachine sm(nodeId, OutlawSettings::getFrequency());
    time_setup_pps(&pps_spec);

    while (true) {
        const int ret = sm.run();
        if (ret != 0) {
            LOG_ERR("state_machine_run returned %d", ret);
            k_sleep(K_SECONDS(1));
        }

        k_sleep(K_MSEC(100));
    }

    return 0;
}
