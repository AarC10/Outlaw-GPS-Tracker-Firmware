#include "core/lora.h"
#include "core/types.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/drivers/gnss.h>
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

bool lora_set_tx() {
    lora_configuration.tx = true;
    if (lora_config(lora_dev, &lora_configuration) != 0) {
        LOG_ERR("LoRa configuration failed");
        return false;
    }
    return true;
}

bool lora_set_rx() {
    lora_configuration.tx = false;
    if (lora_config(lora_dev, &lora_configuration) != 0) {
        LOG_ERR("LoRa configuration failed");
        return false;
    }
    return true;
}

bool lora_init() {
    if (!device_is_ready(lora_dev)) {
        LOG_ERR("LoRa device not ready");
        return false;
    }

    if (lora_config(lora_dev, &lora_configuration) != 0) {
        LOG_ERR("LoRa configuration failed");
        return false;
    }

    LOG_INF("LoRa initialized successfully");
    return true;
}

bool lora_tx(uint8_t* data, uint32_t data_len) {
    if (lora_send_async(lora_dev, data, data_len, NULL) != 0) {
        LOG_ERR("LoRa send failed");
        return false;
    }
    return true;
}

bool lora_send_no_fix_payload(uint8_t node_id) {
    return lora_tx((uint8_t*)NOFIX, strlen(NOFIX));
}

bool lora_send_gnss_payload(uint8_t node_id, const struct gnss_data* gnss_data) {
    lora_payload_t payload;

    payload.latitude_scaled = (int16_t)(gnss_data->nav_data.latitude / LAT_LON_SCALING_FACTOR);
    payload.longitude_scaled = (int16_t)(gnss_data->nav_data.longitude / LAT_LON_SCALING_FACTOR);
    payload.altitude = (uint16_t)(gnss_data->nav_data.altitude);
    payload.speed = (uint16_t)(gnss_data->nav_data.speed);
    payload.satellites_cnt = gnss_data->info.satellites_cnt;
    payload.fix_status = gnss_data->info.fix_status;
    payload.node_id = node_id;

    return lora_tx((uint8_t*)&payload, sizeof(payload));
}

int lora_await_rx_packet() {
    return lora_recv_async(lora_dev, lora_receive_callback, NULL);
}