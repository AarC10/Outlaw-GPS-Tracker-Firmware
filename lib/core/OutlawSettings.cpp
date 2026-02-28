/*
 * Copyright (c) 2026 Aaron Chan
 * SPDX-License-Identifier: Apache-2.0
 */

#include "core/OutlawSettings.h"

#include <cstring>
#include <zephyr/settings/settings.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(OutlawSettings);

static uint32_t s_frequency = OutlawSettings::DEFAULT_FREQUENCY;
static char s_callsign[OutlawSettings::CALLSIGN_LEN] = {};

static int settings_set_handler(const char *name, size_t len,
                                settings_read_cb read_cb, void *cb_arg) {
    if (strcmp(name, "freq") == 0) {
        if (len != sizeof(uint32_t)) {
            return -EINVAL;
        }
        read_cb(cb_arg, &s_frequency, sizeof(uint32_t));
        return 0;
    }
    if (strcmp(name, "cs") == 0) {
        const size_t to_read = len < (size_t)OutlawSettings::CALLSIGN_LEN
                                   ? len
                                   : (size_t)OutlawSettings::CALLSIGN_LEN;
        read_cb(cb_arg, s_callsign, to_read);
        return 0;
    }
    return -ENOENT;
}

SETTINGS_STATIC_HANDLER_DEFINE(outlaw, "outlaw", NULL, settings_set_handler, NULL, NULL);

namespace OutlawSettings {

int load() {
    int ret = settings_subsys_init();
    if (ret != 0) {
        LOG_ERR("settings_subsys_init failed: %d", ret);
        return ret;
    }
    ret = settings_load_subtree("outlaw");
    if (ret != 0) {
        LOG_ERR("settings_load_subtree failed: %d", ret);
    }
    return ret;
}

uint32_t getFrequency() {
    return s_frequency;
}

void getCallsign(char out[CALLSIGN_LEN]) {
    memcpy(out, s_callsign, CALLSIGN_LEN);
}

int saveFrequency(uint32_t frequency) {
    s_frequency = frequency;
    const int ret = settings_save_one("outlaw/freq", &frequency, sizeof(frequency));
    if (ret != 0) {
        LOG_ERR("settings_save_one(outlaw/freq) failed: %d", ret);
    }
    return ret;
}

int saveCallsign(const char callsign[CALLSIGN_LEN]) {
    memcpy(s_callsign, callsign, CALLSIGN_LEN);
    const int ret = settings_save_one("outlaw/cs", callsign, CALLSIGN_LEN);
    if (ret != 0) {
        LOG_ERR("settings_save_one(outlaw/cs) failed: %d", ret);
    }
    return ret;
}

} // namespace OutlawSettings