/*
 * Copyright (c) 2024 Aaron Chan
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/logging/log_ctrl.h>

#include <core/lora.h>


LOG_MODULE_REGISTER(main);

int main(void) {
    lora_init();
    lora_set_rx();

    while (true) {
        int ret = lora_await_rx_packet();
        if (ret != 0) {
            LOG_INF("Error while awaiting LoRa packet: %d", ret);
        }
    }

    return 0;
}
