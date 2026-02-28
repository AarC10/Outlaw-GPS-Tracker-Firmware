#pragma once

#include <stdint.h>
#include <zephyr/kernel.h>

#include "core/GnssReceiver.h"
#include "core/LoraTransceiver.h"

class StateMachine {
public:
    explicit StateMachine(uint8_t nodeId, uint32_t frequencyHz = 903000000U);
    void handleTxTimer();

    int run();

    void applyFrequency(uint32_t frequencyHz);
    void applyCallsign(const char* cs, int len);
    void applyNodeId(uint8_t id);

private:
    enum class State { Transmitter, Receiver };

    static void txWorkHandler(struct k_work* work);

    void enterTransmitter();
    void enterReceiver();
    void exitReceiver();
    void transitionTo(State target);
    int checkForTransition();

    LoraTransceiver lora;
    GnssReceiver gnssReceiver;
    k_timer txTimer{};
    k_work txWork{};
    uint8_t nodeId{};
    int lastPinSate{-1};
    State currentState{State::Transmitter};
};
