#include "core/gnss.h"

#include <atomic>
#include <cstring>

#include <zephyr/drivers/gnss.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(gnss);

static struct gnss_data latest_gnss_data;
static std::atomic_bool fix_acquired{false};

void gnss_data_callback(const struct device* dev, const struct gnss_data* data) {
    if (!data) {
        return;
    }

    std::memcpy(&latest_gnss_data, data, sizeof(latest_gnss_data));

    const bool has_fix = data->info.fix_status != GNSS_FIX_STATUS_NO_FIX;
    fix_acquired.store(has_fix, std::memory_order_relaxed);
}

bool gnss_fix_acquired() {
    return fix_acquired.load(std::memory_order_relaxed);
}

void gnss_get_latest_data(struct gnss_data* out_data) {
    if (!out_data) {
        return;
    }
    std::memcpy(out_data, &latest_gnss_data, sizeof(latest_gnss_data));
}

void gnss_populate_lora_payload(lora_payload_t* payload) {
    if (!payload) {
        return;
    }
    payload->latitude = static_cast<float>(latest_gnss_data.nav_data.latitude) / 1E9f;
    payload->longitude = static_cast<float>(latest_gnss_data.nav_data.longitude) / 1E9f;
    payload->satellites_cnt = static_cast<uint8_t>(latest_gnss_data.info.satellites_cnt);
    payload->fix_status = static_cast<uint8_t>(latest_gnss_data.info.fix_status);
}
