#include "core/lora.h"
#include "core/defs.h"

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
    uint8_t node_id = data[0];
    data++;
    size--;

    LOG_INF("Node %u (%d bytes | %d dBm | %d dB):", node_id, size, rssi, snr);
    switch (size) {
    case sizeof(lora_payload_t): {
        lora_payload_t payload;
        memcpy(&payload, data, sizeof(payload));
        LOG_INF("\tNode ID: %u", node_id);
        LOG_INF("\tLatitude: %f", (double)(payload.latitude_scaled * LAT_LON_SCALING_FACTOR));
        LOG_INF("\tLongitude: %f", (double)(payload.longitude_scaled * LAT_LON_SCALING_FACTOR));
        LOG_INF("\tSatellites count: %u", payload.satellites_cnt);
        switch (payload.fix_status) {
            case GNSS_FIX_STATUS_NO_FIX:
                LOG_INF("\tFix status: NO FIX");
                break;
            case GNSS_FIX_STATUS_GNSS_FIX:
                LOG_INF("\tFix status: FIX");
                break;
            case GNSS_FIX_STATUS_DGNSS_FIX:
                LOG_INF("\tFix status: DIFF FIX");
                break;
            case GNSS_FIX_STATUS_ESTIMATED_FIX:
                LOG_INF("\tFix status: EST FIX");
                break;
            default:
                LOG_INF("\tFIX status: UNKNOWN");
                break;
        }
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
    int ret = lora_config(lora_dev, &lora_configuration);
    if (ret != 0) {
        LOG_ERR("LoRa configuration failed %d", ret);
        return false;
    }
    return true;
}

bool lora_set_rx() {
    lora_configuration.tx = false;
    int ret = lora_config(lora_dev, &lora_configuration);
    if (ret != 0) {
        LOG_ERR("LoRa configuration failed %d", ret);
        return false;
    }
    return true;
}


bool lora_init() {
    if (!device_is_ready(lora_dev)) {
        LOG_ERR("LoRa device not ready (dev ptr %p)", lora_dev);
        return false;
    }

    LOG_INF("LoRa device name: %s, addr: %p", lora_dev->name ? lora_dev->name : "UNKNOWN", lora_dev);

    int ret = lora_config(lora_dev, &lora_configuration);
    if (ret != 0) {
        LOG_ERR("LoRa configuration failed, rc=%d", ret);
        return false;
    }

    LOG_INF("LoRa initialized successfully (freq=%u, sf=%d, bw=%d, cr=%d, txp=%d, public=%d)",
            lora_configuration.frequency,
            lora_configuration.datarate,
            lora_configuration.bandwidth,
            lora_configuration.coding_rate,
            lora_configuration.tx_power,
            lora_configuration.public_network);

    return true;
}

bool lora_tx(uint8_t* data, uint32_t data_len) {
    if (lora_send_async(lora_dev, data, data_len, NULL) != 0) {
        LOG_ERR("LoRa send failed");
        return false;
    }
    LOG_INF("Transmitted %u bytes over LoRa", data_len);
    return true;
}

bool lora_send_no_fix_payload(uint8_t node_id) {
    uint8_t packet[NOFIX_PACKET_SIZE] = {0};
    packet[0] = node_id;
    memcpy(&packet[1], NOFIX, strlen(NOFIX));
    return lora_tx(packet, NOFIX_PACKET_SIZE);
}

bool lora_send_gnss_payload(uint8_t node_id, const struct gnss_data* gnss_data) {
    uint8_t packet[sizeof(lora_payload_t) + 1] = {0};
    packet[0] = node_id;

    lora_payload_t *payload = (lora_payload_t*)&packet[1];

    payload->latitude_scaled = (int16_t)(gnss_data->nav_data.latitude / LAT_LON_SCALING_FACTOR);
    payload->longitude_scaled = (int16_t)(gnss_data->nav_data.longitude / LAT_LON_SCALING_FACTOR);
    payload->satellites_cnt = gnss_data->info.satellites_cnt;
    payload->fix_status = gnss_data->info.fix_status;

    return lora_tx((uint8_t*)&payload, sizeof(payload));
}

int lora_await_rx_packet() {
    if (lora_configuration.tx) {
        LOG_WRN("LoRa is in TX mode, cannot receive");
        return -1;
    }

    if (lora_recv_async(lora_dev, lora_receive_callback, NULL) != 0) {
        LOG_ERR("LoRa async receive setup failed");
        return -1;
    }

    return 0;
}

int lora_await_cancel() {
    if (lora_recv_async(lora_dev, NULL, NULL) != 0) {
        LOG_ERR("LoRa async cancel setup failed");
    }


    return 0;
}