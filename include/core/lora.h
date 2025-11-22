#pragma once

#include <stdint.h>
#include <zephyr/device.h>

// Forward Declarations
struct gnss_data;

bool lora_init();

void lora_receive_callback(const struct device* dev, uint8_t* data, uint16_t size, int16_t rssi, int8_t snr,
                                  void* user_data);
bool lora_is_tx();

bool lora_set_tx();

bool lora_set_rx();

bool lora_tx(uint8_t* data, uint32_t data_len);

bool lora_send_no_fix_payload(uint8_t node_id);

bool lora_send_gnss_payload(uint8_t node_id, const struct gnss_data* gnss_data);

int lora_await_rx_packet();