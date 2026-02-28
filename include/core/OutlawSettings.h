#pragma once

#include <stdint.h>

namespace OutlawSettings {

constexpr uint32_t DEFAULT_FREQUENCY = 903000000;
constexpr int CALLSIGN_LEN = 6;

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

/**
 * Persist a new frequency to NVS and update the in-memory value.
 * @return 0 on success, negative errno on failure
 */
int saveFrequency(uint32_t frequency);

/**
 * Persist a new callsign to NVS and update the in-memory value.
 * Stores exactly CALLSIGN_LEN bytes from callsign.
 * @return 0 on success, negative errno on failure
 */
int saveCallsign(const char callsign[CALLSIGN_LEN]);

} // namespace OutlawSettings