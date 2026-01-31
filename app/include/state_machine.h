#pragma once

#include <stdint.h>
#include <zephyr/kernel.h>

#include "core/LoraTransceiver.h"

class StateMachine {
public:
    explicit StateMachine(uint8_t nodeId);

    void handleTxTimer();

    int run();

private:
    enum class State { Transmitter, Receiver };

    void enterTransmitter();
    void enterReceiver();
    void exitReceiver();
    void transitionTo(State target);
    int checkForTransition();

    LoraTransceiver lora;
    k_timer txTimer{};
    uint8_t nodeId{};
    int lastPinSate{-1};
    State currentState{State::Transmitter};
};
