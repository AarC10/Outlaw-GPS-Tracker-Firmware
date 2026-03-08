#include <core/TdmaClock.h>

#include <zephyr/drivers/counter.h>
#include <zephyr/logging/log.h>

#include <utility>

LOG_MODULE_REGISTER(tdma_clock, LOG_LEVEL_INF);

namespace {
constexpr uint32_t frameLenMs = 1000;
constexpr uint32_t gpsDemoteMs = 5000;
constexpr uint32_t hunterStaleMs = 30000;
}

TdmaClock& TdmaClock::instance() {
    static TdmaClock clock;
    return clock;
}

void TdmaClock::init(const gpio_dt_spec* pps, const device* tim2Dev) {

}

TdmaClock::Source TdmaClock::source() const {
    return static_cast<Source>(atomic_get(&currentSource));
}

uint32_t TdmaClock::epochTicks() const {
    return static_cast<uint32_t>(atomic_get(&epochTicksValue));
}

uint32_t TdmaClock::frameNumber() const {
    return static_cast<uint32_t>(atomic_get(&frameNumberValue));
}

void TdmaClock::onHunterBeacon(uint32_t beaconFrameNumber, uint32_t timestamp) {

}

void TdmaClock::ppsIsr(const device* dev, gpio_callback* cb, uint32_t pins) {

}

void TdmaClock::freerunExpiry(k_timer* timer) {

}

void TdmaClock::demoteHandler(k_work* work) {

}

void TdmaClock::startFreerun() {
    k_timer_start(&freerunTimer, K_NO_WAIT, K_MSEC(frameLenMs));
}

void TdmaClock::stopFreerun() {
    k_timer_stop(&freerunTimer);
}

uint32_t TdmaClock::readTim2Ticks() const {
    if (tim2 == nullptr || !device_is_ready(tim2)) {
        return 0;
    }

    uint32_t ticks = 0;
    if (counter_get_value(tim2, &ticks) != 0) {
        return 0;
    }

    return ticks;
}

void TdmaClock::scheduleDemote(k_timeout_t delay) {
    (void)k_work_reschedule(&demoteWork, delay);
}

