#include "core/LoraTransceiver.h"

#include <array>
#include <cstring>

#include "core/defs.h"
#include "zephyr/drivers/gnss.h"
#include "zephyr/logging/log.h"

LOG_MODULE_REGISTER(LoraTransceiver);

static void loraReceiveCallback(const device* dev, uint8_t* data, uint16_t size, int16_t rssi, int8_t snr,
                           void* userData) {
    if (auto* transceiver = static_cast<LoraTransceiver*>(userData)) {
        transceiver->receiveCallback(data, size, rssi, snr);
    }
}

LoraTransceiver::LoraTransceiver(const uint8_t nodeId) : nodeId(nodeId) {
    init();
};

LoraTransceiver::LoraTransceiver(const uint8_t nodeId, const lora_modem_config& config) : nodeId(nodeId), config(config) {
    init();
}

bool LoraTransceiver::txNoFixPayload() {
    return tx(const_cast<uint8_t*>(NOFIX), NOFIX_PACKET_SIZE);
}

bool LoraTransceiver::txGnssPayload() {
    // TODO: Update with GNSS class usage
    // if (!gnss_data) {
    //     LOG_ERR("GNSS data null, cannot send");
    //     return false;
    // }
    //
    // std::array<uint8_t, sizeof(lora_payload_t) + 1> packet{};
    // packet[0] = node_id;
    //
    // auto* payload = reinterpret_cast<lora_payload_t*>(&packet[1]);
    // payload->latitude = static_cast<float>(gnss_data->nav_data.latitude) / 1E9f;
    // payload->longitude = static_cast<float>(gnss_data->nav_data.longitude) / 1E9f;
    // payload->satellites_cnt = gnss_data->info.satellites_cnt;
    // payload->fix_status = gnss_data->info.fix_status;
    //
    // return lora_tx(packet.data(), static_cast<uint32_t>(packet.size()));
    return false;
}

int LoraTransceiver::awaitRxPacket() {
    if (config.tx) {
        LOG_WRN("LoRa is in TX mode, cannot receive");
        return -1;
    }

    if (lora_recv_async(dev, loraReceiveCallback, this) != 0) {
        LOG_ERR("LoRa async receive setup failed");
        return -1;
    }

    return 0;
}

int LoraTransceiver::awaitCancel() {
    if (lora_recv_async(dev, nullptr, nullptr) != 0) {
        LOG_ERR("LoRa async cancel setup failed");
        return -1;
    }

    return 0;
}

void LoraTransceiver::receiveCallback(uint8_t *data, uint16_t size, int16_t rssi, int8_t snr) {
    if (config.tx || !data || size == 0) return;

    const uint8_t node_id = data[0];
    data++;
    size--;

    LOG_INF("Node %u (%d bytes | %d dBm | %d dB):", node_id, size, rssi, snr);
    switch (size) {
    case sizeof(lora_payload_t): {
        lora_payload_t payload{};
        std::memcpy(&payload, data, sizeof(payload));
        LOG_INF("\tNode ID: %u", node_id);
        LOG_INF("\tLatitude: %f", static_cast<double>(payload.latitude));
        LOG_INF("\tLongitude: %f", static_cast<double>(payload.longitude));
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
    case NOFIX_PACKET_SIZE:
        LOG_INF("\tNo fix acquired!");
        break;
    default:
        LOG_INF("\tReceived data: %s", data);
        break;
    }

}

bool LoraTransceiver::setTx() {
    config.tx = true;
    const int ret = lora_config(dev, &config);
    if (ret != 0) {
        LOG_ERR("LoRa configuration failed %d", ret);
        return false;
    }
    return true;
};

bool LoraTransceiver::setRx() {
    config.tx = false;
    const int ret = lora_config(dev, &config);
    if (ret != 0) {
        LOG_ERR("LoRa configuration failed %d", ret);
        return false;
    }
    return true;
};

bool LoraTransceiver::init() {
    if (!device_is_ready(dev)) {
        LOG_ERR("LoRa device not ready (dev ptr %p)", dev);
        return false;
    }

    LOG_INF("LoRa device name: %s, addr: %p", dev->name ? dev->name : "UNKNOWN", dev);

    const int ret = lora_config(dev, &config);
    if (ret != 0) {
        LOG_ERR("LoRa configuration failed, rc=%d", ret);
        return false;
    }

    LOG_INF("LoRa initialized successfully (freq=%u, sf=%d, bw=%d, cr=%d, txp=%d, public=%d)",
            config.frequency,
            config.datarate,
            config.bandwidth,
            config.coding_rate,
            config.tx_power,
            config.public_network);

    return true;
}

bool LoraTransceiver::tx(uint8_t* data, uint32_t data_len) {
    if (!data || data_len == 0) {
        LOG_ERR("LoRa send called with empty payload");
        return false;
    }

    if (lora_send_async(dev, data, data_len, nullptr) != 0) {
        LOG_ERR("LoRa send failed");
        return false;
    }
    LOG_INF("Transmitted %u bytes over LoRa", data_len);
    return true;
}

bool LoraTransceiver::setCallsign(const HamCallsign &callsign) {
    callsignPtr = const_cast<HamCallsign*>(&callsign);
    return updateTxBufferHeader();
}

bool LoraTransceiver::setNodeId(uint8_t id) {
    nodeId = id;
    return updateTxBufferHeader();
}

bool LoraTransceiver::updateTxBufferHeader() {
    txBuffPayloadStartIndex = 0;
    if (is433MHzBand()) {
        if (callsignPtr && callsignPtr->isValid()) {
            const auto& chunks = callsignPtr->encodedChunks();
            const size_t chunkCount = chunks.size();
            for (txBuffPayloadStartIndex = 0; txBuffPayloadStartIndex < chunkCount; ++txBuffPayloadStartIndex) {
                txBuffer[txBuffPayloadStartIndex] = static_cast<uint8_t>((chunks[txBuffPayloadStartIndex] >> 8) & 0xFF);
            }
        } else {
            LOG_ERR("Invalid callsign for 433MHz band transmission");
            return false;
        }
    }

    txBuffer[txBuffPayloadStartIndex++] = nodeId;

    return true;
}
