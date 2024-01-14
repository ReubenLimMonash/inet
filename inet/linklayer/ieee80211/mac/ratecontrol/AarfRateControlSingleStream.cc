//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Date: 27/09/2023
// Author: Reuben Lim
// Desc: Modified AarfRateControl to use only MCS in single spatial stream
//

#include "inet/linklayer/ieee80211/mac/ratecontrol/AarfRateControlSingleStream.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

Define_Module(AarfRateControlSingleStream);

void AarfRateControlSingleStream::initialize(int stage)
{
    RateControlBaseSingleStream::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        factor = par("increaseThresholdFactor");
        increaseThreshold = par("increaseThreshold");
        maxIncreaseThreshold = par("maxIncreaseThreshold");
        decreaseThreshold = par("decreaseThreshold");
        interval = par("interval");
        WATCH(factor);
        WATCH(increaseThreshold);
        WATCH(maxIncreaseThreshold);
        WATCH(decreaseThreshold);
        WATCH(interval);
        WATCH(probing);
        WATCH(numberOfConsSuccTransmissions);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        updateDisplayString();
    }
}

void AarfRateControlSingleStream::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't handle self messages");
}

void AarfRateControlSingleStream::updateDisplayString() const
{
    getDisplayString().setTagArg("t", 0, currentMode->getName());
}

void AarfRateControlSingleStream::frameTransmitted(Packet *frame, int retryCount, bool isSuccessful, bool isGivenUp)
{
    increaseRateIfTimerIsExpired();

    if (!isSuccessful && probing) { // probing packet failed
        numberOfConsSuccTransmissions = 0;
        // std::cout << "The current mode is " << *currentMode << " the net bitrate is " << currentMode->getDataMode()->getNetBitrate() << std::endl;
        currentMode = decreaseRateIfPossible(currentMode);
        emitDatarateChangedSignal();
        updateDisplayString();
        EV_DETAIL << "Decreased rate to " << *currentMode << endl;
        multiplyIncreaseThreshold(factor);
        resetTimer();
    }
    else if (!isSuccessful && retryCount >= decreaseThreshold - 1) { // decreaseThreshold consecutive failed transmissions
        numberOfConsSuccTransmissions = 0;
        // std::cout << "The current mode is " << *currentMode << " the net bitrate is " << currentMode->getDataMode()->getNetBitrate() << std::endl;
        currentMode = decreaseRateIfPossible(currentMode);
        emitDatarateChangedSignal();
        updateDisplayString();
        EV_DETAIL << "Decreased rate to " << *currentMode << endl;
        resetIncreaseThreshdold();
        resetTimer();
    }
    else if (isSuccessful && retryCount == 0)
        numberOfConsSuccTransmissions++;

    if (numberOfConsSuccTransmissions == increaseThreshold) {
        numberOfConsSuccTransmissions = 0;
        // std::cout << "The current mode is " << *currentMode << " the net bitrate is " << currentMode->getDataMode()->getNetBitrate() << std::endl;
        currentMode = increaseRateIfPossible(currentMode);
        emitDatarateChangedSignal();
        updateDisplayString();
        EV_DETAIL << "Increased rate to " << *currentMode << endl;
        resetTimer();
        probing = true;
    }
    else
        probing = false;

}

void AarfRateControlSingleStream::multiplyIncreaseThreshold(double factor)
{
    if (increaseThreshold * factor <= maxIncreaseThreshold)
        increaseThreshold *= factor;
}

void AarfRateControlSingleStream::resetIncreaseThreshdold()
{
    increaseThreshold = par("increaseThreshold");
}

void AarfRateControlSingleStream::resetTimer()
{
    timer = simTime();
}

void AarfRateControlSingleStream::increaseRateIfTimerIsExpired()
{
    if (simTime() - timer >= interval) {
        currentMode = increaseRateIfPossible(currentMode);
        emitDatarateChangedSignal();
        updateDisplayString();
        EV_DETAIL << "Increased rate to " << *currentMode << endl;
        resetTimer();
    }
}

void AarfRateControlSingleStream::frameReceived(Packet *frame)
{
}

const IIeee80211Mode *AarfRateControlSingleStream::getRate()
{
    Enter_Method("getRate");
    increaseRateIfTimerIsExpired();
    EV_INFO << "The current mode is " << currentMode << " the net bitrate is " << currentMode->getDataMode()->getNetBitrate() << std::endl;
    return currentMode;
}

} /* namespace ieee80211 */
} /* namespace inet */

