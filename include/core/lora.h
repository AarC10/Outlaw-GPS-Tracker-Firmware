#pragma once

#include <stdint.h>
#include <zephyr/device.h>

void lora_receive_callback(const struct device* dev, uint8_t* data, uint16_t size, int16_t rssi, int8_t snr,
                                  void* user_data);

bool lora_is_tx();

bool lora_set_tx();

bool lora_set_rx();

bool lora_tx(uint8_t* data, uint32_t data_len);

int lora_await_rx_packet();