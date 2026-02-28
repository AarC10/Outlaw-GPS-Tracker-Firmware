#pragma once

#include <stdint.h>

namespace OutlawSettings {

#ifdef CONFIG_LICENSED_FREQUENCY
constexpr uint32_t DEFAULT_FREQUENCY = 435000000;
#else
constexpr uint32_t DEFAULT_FREQUENCY = 903000000;
#endif
constexpr int CALLSIGN_LEN = 6;
constexpr uint8_t DEFAULT_NODE_ID = 1;

/**
 * Initialize the settings subsystem and load persisted values from NVS.
 * Call once at startup before reading any settings.
 * @return 0 on success, negative errno on failure
 */
int load();

/**
 * Get the persisted LoRa frequency, or DEFAULT_FREQUENCY if not yet saved.
 */
uint32_t getFrequency();

/**
 * Copy the persisted callsign into out (exactly CALLSIGN_LEN bytes, zero-padded).
 */
void getCallsign(char out[CALLSIGN_LEN]);
uint8_t getNodeId();

int saveFrequency(uint32_t frequency);
int saveCallsign(const char callsign[CALLSIGN_LEN]);
int saveNodeId(uint8_t nodeId);

} // namespace OutlawSettings