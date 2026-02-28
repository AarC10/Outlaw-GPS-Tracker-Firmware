/*
 * Copyright (c) 2024 Aaron Chan
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>

#include "core/LoraTransceiver.h"
#include "core/OutlawSettings.h"


LOG_MODULE_REGISTER(main);

static LoraTransceiver *loraPtr = nullptr;

int main(void) {
    OutlawSettings::load();

    static LoraTransceiver lora(0, OutlawSettings::getFrequency());
    loraPtr = &lora;

    OutlawSettings::setFrequencyChangedCallback([](uint32_t f) {
        if (loraPtr) {
            loraPtr->awaitCancel();
            loraPtr->setFrequency(f);
            loraPtr->awaitRxPacket();
        }
    });

    lora.awaitRxPacket();

    while (true) {
        k_sleep(K_FOREVER);
    }

    return 0;
}
