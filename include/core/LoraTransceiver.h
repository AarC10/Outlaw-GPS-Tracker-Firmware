#pragma once

#include <zephyr/drivers/lora.h>
#include <stdint.h>

class LoraTransceiver {
public:
    LoraTransceiver(const uint8_t nodeId);
    LoraTransceiver(const uint8_t nodeId, const lora_modem_config &config);

    bool txNoFixPayload();

    bool txGnssPayload();

    int awaitRxPacket();

    int awaitCancel();

    void receiveCallback(uint8_t *data, uint16_t size, int16_t rssi, int8_t snr);

    bool isTx() const { return config.tx; };

    bool setTx();

    bool setRx();

private:
    bool init();

    lora_modem_config config {
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

    const struct device* dev = DEVICE_DT_GET(DT_ALIAS(lora));
    const uint8_t nodeId;

    bool tx(uint8_t* data, uint32_t data_len);
};
