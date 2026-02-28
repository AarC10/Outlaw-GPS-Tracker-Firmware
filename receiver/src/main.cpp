/*
 * Copyright (c) 2024 Aaron Chan
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>

#include "core/LoraTransceiver.h"
#include "core/Settings.h"


LOG_MODULE_REGISTER(main);

int main(void) {
    OutlawSettings::load();

    LoraTransceiver lora(0);
    lora.setFrequency(OutlawSettings::getFrequency());
    lora.awaitRxPacket();

    while (true) {

    }

    return 0;
}
