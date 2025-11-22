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

int main(void) {
    lora_init();
    lora_set_rx();

    while (true) {
        lora_await_rx_packet();
    }

    return 0;
}
