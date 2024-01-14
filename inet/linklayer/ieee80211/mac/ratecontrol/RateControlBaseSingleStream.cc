//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Date: 27/09/2023
// Author: Reuben Lim
// Desc: Modified RateControlBase to use only MCS in single spatial stream with 20MHz bandwidth
//

#include "inet/linklayer/ieee80211/mac/ratecontrol/RateControlBaseSingleStream.h"

#include "inet/common/Simsignals.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

simsignal_t RateControlBaseSingleStream::datarateChangedSignal = cComponent::registerSignal("datarateChanged");

void RateControlBaseSingleStream::initialize(int stage)
{
    ModeSetListener::initialize(stage);
}

const IIeee80211Mode *RateControlBaseSingleStream::increaseRateIfPossible(const IIeee80211Mode *currentMode)
{
    if (MCSIndex < 7){
        ++MCSIndex;
    }
    const IIeee80211Mode *newMode = modeSet->getMode(MCSBitrates[MCSIndex], bandwidth, numSpatialStream);
    return newMode == nullptr ? currentMode : newMode;
}

const IIeee80211Mode *RateControlBaseSingleStream::decreaseRateIfPossible(const IIeee80211Mode *currentMode)
{
    if (MCSIndex > 0){
        --MCSIndex;
    }
    const IIeee80211Mode *newMode = modeSet->getMode(MCSBitrates[MCSIndex], bandwidth, numSpatialStream);
    return newMode == nullptr ? currentMode : newMode;
}

const IIeee80211Mode *RateControlBaseSingleStream::setMode(bps bitrate, Hz bandwidth, int numSpatialStream)
{
    const IIeee80211Mode *newMode = modeSet->getMode(bitrate, bandwidth, numSpatialStream);
    return newMode == nullptr ? currentMode : newMode;
}

void RateControlBaseSingleStream::emitDatarateChangedSignal()
{
    bps rate = currentMode->getDataMode()->getNetBitrate();
    emit(datarateChangedSignal, rate.get());
}

void RateControlBaseSingleStream::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    // This function should only be called once when the Ieee80211Mac initializes
    // Ref: \inet\linklayer\ieee80211\mac\Ieee80211Mac.cc
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == modesetChangedSignal) {
        modeSet = check_and_cast<Ieee80211ModeSet *>(obj);
        double initRate = par("initialRate");
        // Search for the MCS index based on init rate
        for (int index = 0; index < 8; index++) {
            if (bps(initRate) == MCSBitrates[index]){
                MCSIndex = index;
            }
        }
        if (MCSIndex < 0 || MCSIndex > 7) {
            throw cRuntimeError("Initial bitrate does not match single spatial stream bitrates");
        }
        currentMode = initRate == -1 ? modeSet->getMode(Mbps(65), bandwidth, numSpatialStream) : modeSet->getMode(bps(initRate), bandwidth, numSpatialStream);
        emitDatarateChangedSignal();
    }
}

} /* namespace ieee80211 */
} /* namespace inet */

