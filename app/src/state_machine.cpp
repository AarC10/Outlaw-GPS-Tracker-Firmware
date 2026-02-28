#include "state_machine.h"

#include <cstring>
#include <zephyr/drivers/gnss.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <core/OutlawSettings.h>

static constexpr int TRANSMITTER_LOGIC_LEVEL = 0;
static constexpr int TRANSMITTER_LED_LEVEL = 0;

static constexpr int RECEIVER_LOGIC_LEVEL = 1;
static constexpr int RECEIVER_LED_LEVEL = 1;

LOG_MODULE_REGISTER(state_machine);

static const gpio_dt_spec dip0 = GPIO_DT_SPEC_GET(DT_ALIAS(dip0), gpios);
static const gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static void txTimerCallback(struct k_timer* timer) {
    StateMachine* sm = static_cast<StateMachine*>(k_timer_user_data_get(timer));
    if (sm) k_work_submit(&sm->txWork);
}

void StateMachine::txWorkHandler(struct k_work* work) {
    CONTAINER_OF(work, StateMachine, txWork)->handleTxTimer();
}

StateMachine::StateMachine(const uint8_t nodeId, const uint32_t frequencyHz)
    : lora(nodeId, frequencyHz), nodeId(nodeId) {
    k_timer_init(&txTimer, txTimerCallback, nullptr);
    k_timer_user_data_set(&txTimer, this);
    k_work_init(&txWork, txWorkHandler);

#ifdef CONFIG_LICENSED_FREQUENCY
    char cs[OutlawSettings::CALLSIGN_LEN] = {};
    OutlawSettings::getCallsign(cs);
    const size_t csLen = strnlen(cs, OutlawSettings::CALLSIGN_LEN);
    lora.setCallsign(HamCallsign(std::string_view(cs, csLen)));
#endif

#ifdef CONFIG_DEFAULT_RECEIVE_MODE
    currentState = State::Receiver;
    enterReceiver();
#else
    currentState = State::Transmitter;
    enterTransmitter();
#endif

    setGnssReciever(&gnssReceiver);
}

void StateMachine::handleTxTimer() {
    if (gnssReceiver.isFixAcquired()) {
        lora.txGnssPayload(gnssReceiver.getLatestData());
    } else {
        lora.txNoFixPayload();
    }
}

int StateMachine::run() {
    return checkForTransition();
}

void StateMachine::enterTransmitter() {
    LOG_INF("Entering transmitter state");
    lora.setTx();
    gpio_pin_set_dt(&led, TRANSMITTER_LED_LEVEL);
    k_timer_start(&txTimer, K_SECONDS(5), K_SECONDS(5));
}

void StateMachine::enterReceiver() {
    LOG_INF("Entering receiver state");
    lora.setRx();
    gpio_pin_set_dt(&led, RECEIVER_LED_LEVEL);
    k_timer_stop(&txTimer);
    lora.awaitRxPacket();
}

void StateMachine::exitReceiver() {
    lora.awaitCancel();
}

void StateMachine::transitionTo(State target) {
    if (target == currentState) return;
    if (currentState == State::Receiver) exitReceiver();
    currentState = target;
    if (currentState == State::Transmitter) {
        enterTransmitter();
    } else {
        enterReceiver();
    }
}

int StateMachine::checkForTransition() {
    const int currentPinState = gpio_pin_get_dt(&dip0);
    LOG_DBG("Pin state: %d", currentPinState);

    if (currentPinState < 0) {
        LOG_DBG("dip0 gpio read error (%d); ignoring transition check", currentPinState);
        return 0;
    }

    if (lastPinSate != currentPinState) {
        if (currentPinState == TRANSMITTER_LOGIC_LEVEL) {
            transitionTo(State::Transmitter);
        } else if (currentPinState == RECEIVER_LOGIC_LEVEL) {
            transitionTo(State::Receiver);
        }
    }

    lastPinSate = currentPinState;
    return 0;
}

void StateMachine::applyFrequency(uint32_t frequencyHz) {
    lora.setFrequency(frequencyHz);
#ifdef CONFIG_LICENSED_FREQUENCY
    char cs[OutlawSettings::CALLSIGN_LEN] = {};
    OutlawSettings::getCallsign(cs);
    const size_t csLen = strnlen(cs, OutlawSettings::CALLSIGN_LEN);
    lora.setCallsign(HamCallsign(std::string_view(cs, csLen)));
#endif
}

void StateMachine::applyCallsign(const char* cs, int len) {
#ifdef CONFIG_LICENSED_FREQUENCY
    const size_t csLen = strnlen(cs, len);
    lora.setCallsign(HamCallsign(std::string_view(cs, csLen)));
#endif
}

void StateMachine::applyNodeId(uint8_t id) {
    nodeId = id;
    lora.setNodeId(id);
}