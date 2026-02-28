/*
 * Copyright (c) 2026 Aaron Chan
 * SPDX-License-Identifier: Apache-2.0
 */

#include "core/OutlawSettings.h"

#include <cstdlib>
#include <cstring>
#include <zephyr/settings/settings.h>
#include <zephyr/logging/log.h>

#ifdef CONFIG_SHELL
#include <zephyr/shell/shell.h>
#endif

LOG_MODULE_REGISTER(OutlawSettings);

static uint32_t CONFIGURED_FREQUENCY = OutlawSettings::DEFAULT_FREQUENCY;
static char CONFIGURED_CALLSIGN[OutlawSettings::CALLSIGN_LEN] = {};
static uint8_t CONFIGURED_NODE_ID = OutlawSettings::DEFAULT_NODE_ID;

static int settings_set_handler(const char *name, size_t len,
                                settings_read_cb readCallback, void *callbackArgs) {
    if (strcmp(name, "freq") == 0) {
        if (len != sizeof(uint32_t)) {
            return -EINVAL;
        }
        readCallback(callbackArgs, &CONFIGURED_FREQUENCY, sizeof(uint32_t));
        return 0;
    }
    if (strcmp(name, "cs") == 0) {
        const size_t to_read = len < static_cast<size_t>(OutlawSettings::CALLSIGN_LEN)
                                   ? len
                                   : static_cast<size_t>(OutlawSettings::CALLSIGN_LEN);
        readCallback(callbackArgs, CONFIGURED_CALLSIGN, to_read);
        return 0;
    }
    if (strcmp(name, "nid") == 0) {
        if (len != sizeof(uint8_t)) return -EINVAL;
        readCallback(callbackArgs, &CONFIGURED_NODE_ID, sizeof(uint8_t));
        return 0;
    }
    return -ENOENT;
}

SETTINGS_STATIC_HANDLER_DEFINE(outlaw, "outlaw", nullptr, settings_set_handler, nullptr, nullptr);

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
    LOG_INF("Loaded frequency: %u Hz", CONFIGURED_FREQUENCY);
    return CONFIGURED_FREQUENCY;
}

void getCallsign(char out[CALLSIGN_LEN]) {
    memcpy(out, CONFIGURED_CALLSIGN, CALLSIGN_LEN);
}

int saveFrequency(uint32_t frequency) {
    CONFIGURED_FREQUENCY = frequency;
    const int ret = settings_save_one("outlaw/freq", &frequency, sizeof(frequency));
    if (ret != 0) {
        LOG_ERR("settings_save_one(outlaw/freq) failed: %d", ret);
    }
    return ret;
}

int saveCallsign(const char callsign[CALLSIGN_LEN]) {
    memcpy(CONFIGURED_CALLSIGN, callsign, CALLSIGN_LEN);
    const int ret = settings_save_one("outlaw/cs", callsign, CALLSIGN_LEN);
    if (ret != 0) {
        LOG_ERR("settings_save_one(outlaw/cs) failed: %d", ret);
    }
    return ret;
}

uint8_t getNodeId() {
    return CONFIGURED_NODE_ID;
}

int saveNodeId(uint8_t nodeId) {
    if (nodeId < 1 || nodeId > 10) return -EINVAL;
    CONFIGURED_NODE_ID = nodeId;
    const int ret = settings_save_one("outlaw/nid", &nodeId, sizeof(nodeId));
    if (ret != 0) {
        LOG_ERR("settings_save_one(outlaw/nid) failed: %d", ret);
    }
    return ret;
}

} // namespace OutlawSettings

#ifdef CONFIG_SHELL

static int cmd_freq(const struct shell *sh, size_t argc, char **argv) {
    char *end;
    const unsigned long freq = strtoul(argv[1], &end, 10);
    if (*end != '\0' || freq < 100000000UL || freq > 1100000000UL) {
        shell_error(sh, "Invalid frequency '%s' (expected Hz, e.g. 903000000)", argv[1]);
        return -EINVAL;
    }
    const int ret = OutlawSettings::saveFrequency(static_cast<uint32_t>(freq));

    if (ret == 0) {
        shell_print(sh, "Frequency saved: %lu Hz (reboot to apply)", freq);
    } else {
        shell_error(sh, "Save failed: %d", ret);
    }
    return ret;
}

static int cmd_callsign(const struct shell *sh, size_t argc, char **argv) {
    const size_t len = strlen(argv[1]);
    if (len == 0 || len > (size_t)OutlawSettings::CALLSIGN_LEN) {
        shell_error(sh, "Callsign must be 1-%d characters", OutlawSettings::CALLSIGN_LEN);
        return -EINVAL;
    }
    char cs[OutlawSettings::CALLSIGN_LEN] = {};
    memcpy(cs, argv[1], len);
    const int ret = OutlawSettings::saveCallsign(cs);
    if (ret == 0) {
        shell_print(sh, "Callsign saved: %.*s (reboot to apply)", OutlawSettings::CALLSIGN_LEN, cs);
    } else {
        shell_error(sh, "Save failed: %d", ret);
    }
    return ret;
}

static int cmd_node_id(const struct shell *sh, size_t argc, char **argv) {
    char *end;
    const unsigned long id = strtoul(argv[1], &end, 10);
    if (*end != '\0' || id < 1 || id > 10) {
        shell_error(sh, "Invalid node ID '%s' (expected 1-10)", argv[1]);
        return -EINVAL;
    }
    const int ret = OutlawSettings::saveNodeId(static_cast<uint8_t>(id));
    if (ret == 0) {
        shell_print(sh, "Node ID saved: %lu (reboot to apply)", id);
    } else {
        shell_error(sh, "Save failed: %d", ret);
    }
    return ret;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_outlaw,
    SHELL_CMD_ARG(freq, NULL, "Set LoRa frequency in Hz (e.g. 903000000)", cmd_freq, 2, 0),
    SHELL_CMD_ARG(callsign, NULL, "Set callsign, max 6 chars (e.g. W1ABC)", cmd_callsign, 2, 0),
    SHELL_CMD_ARG(node_id, NULL, "Set node ID (1-10)", cmd_node_id, 2, 0),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(outlaw, &sub_outlaw, "Outlaw GPS tracker settings", NULL);

#endif // CONFIG_SHELL