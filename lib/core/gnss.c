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
        LOG_INF("Fix acquired!");
        fix_acquired = true;
    } else if (data->info.fix_status == GNSS_FIX_STATUS_NO_FIX && fix_acquired) {
        LOG_INF("No fix acquired!");
        fix_acquired = false;
    }
}

bool gnss_fix_acquired() {
    return fix_acquired;
}

void gnss_get_latest_data(struct gnss_data* out_data) {
    memcpy(out_data, &latest_gnss_data, sizeof(latest_gnss_data));
}