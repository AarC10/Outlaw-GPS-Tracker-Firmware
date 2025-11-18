#include "core/lora.h"

#include <stdbool.h>

LOG_MODULE_REGISTER(lora);

static const struct device* lora_dev = DEVICE_DT_GET(DT_ALIAS(lora));

static struct lora_modem_config lora_configuration = {
    .frequency = 903000000,
    .bandwidth = BW_125_KHZ,
    .datarate = SF_12,
    .coding_rate = CR_4_5,
    .preamble_len = 8,
    .tx_power = 20,
    .tx = false,
    .iq_inverted = false,
    .public_network = false,
};


void lora_receive_callback(const struct device* dev, uint8_t* data, uint16_t size, int16_t rssi, int8_t snr,
                                  void* user_data) {
    if (lora_configuration.tx) return;

    LOG_INF("Packet received (%d bytes | %d dBm | %d dB:", size, rssi, snr);

    switch (size) {
    case sizeof(struct gnss_data): {
        struct gnss_data gnss_data_local;
        memcpy(&gnss_data_local, data, sizeof(gnss_data_local));
        LOG_INF("\tLatitude: %lld", gnss_data_local.nav_data.latitude);
        LOG_INF("\tLongitude: %lld", gnss_data_local.nav_data.longitude);
        LOG_INF("\tBearing: %u", gnss_data_local.nav_data.bearing);
        LOG_INF("\tSpeed: %u", gnss_data_local.nav_data.speed);
        LOG_INF("\tAltitude: %d", gnss_data_local.nav_data.altitude);
        break;
    }
    case strlen(NOFIX):
        LOG_INF("\tNo fix acquired!");
        break;
    default:
        LOG_INF("\tReceived data: %s", data);
        break;
    }
}

bool lora_is_tx() {
    return lora_configuration.tx;
}