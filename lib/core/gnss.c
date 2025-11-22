#include "core/defs.h"
#include "core/gnss.h"
#include "core/lora.h"

#include <stdbool.h>
#include <zephyr/drivers/gnss.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(gnss);

static struct gnss_data latest_gnss_data;
static bool fix_acquired = false;

void gnss_data_callback(const struct device* dev, const struct gnss_data* data) {
    memcpy(&latest_gnss_data, data, sizeof(latest_gnss_data));
    if (data->info.fix_status != GNSS_FIX_STATUS_NO_FIX && !fix_acquired) {
        fix_acquired = true;
    } else if (data->info.fix_status == GNSS_FIX_STATUS_NO_FIX && fix_acquired) {
        fix_acquired = false;
    }
}

bool gnss_fix_acquired() {
    return fix_acquired;
}

void gnss_get_latest_data(struct gnss_data* out_data) {
    memcpy(out_data, &latest_gnss_data, sizeof(latest_gnss_data));
}

static float nanodeg_to_deg(int64_t nanodeg) {
    return nanodeg / 10000000.0f;
}

int16_t gnss_get_latitude_scaled() {
    return (int16_t)(nanodeg_to_deg(latest_gnss_data.nav_data.latitude) / LAT_LON_SCALING_FACTOR);
}

int16_t gnss_get_longitude_scaled() {
    return (int16_t)(nanodeg_to_deg(latest_gnss_data.nav_data.longitude) / LAT_LON_SCALING_FACTOR);
}

uint8_t gnss_get_satellites_cnt() {
    return latest_gnss_data.info.satellites_cnt;
}

uint8_t gnss_get_fix_status() {
    return latest_gnss_data.info.fix_status;
}
