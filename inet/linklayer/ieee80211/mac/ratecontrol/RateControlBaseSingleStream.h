//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Date: 27/09/2023
// Author: Reuben Lim
// Desc: Modified RateControlBase to use only MCS in single spatial stream with 20MHz bandwidth
//

#ifndef __INET_RATECONTROLBASESINGLESTREAM_H
#define __INET_RATECONTROLBASESINGLESTREAM_H

#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateControl.h"

namespace inet {
namespace ieee80211 {

class INET_API RateControlBaseSingleStream : public ModeSetListener, public IRateControl
{
  public:
    static simsignal_t datarateChangedSignal;

  protected:
    const physicallayer::IIeee80211Mode *currentMode = nullptr;
    int MCSIndex; // This should range between 0 and 7 since single spatial stream
    const Hz bandwidth = MHz(20);
    const int numSpatialStream = 1;
    const std::vector<bps> MCSBitrates {Mbps(6.5), Mbps(13), Mbps(19.5), Mbps(26), Mbps(39), Mbps(52), Mbps(58.5), Mbps(65)};

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    virtual void emitDatarateChangedSignal();

    const physicallayer::IIeee80211Mode *increaseRateIfPossible(const physicallayer::IIeee80211Mode *currentMode);
    const physicallayer::IIeee80211Mode *decreaseRateIfPossible(const physicallayer::IIeee80211Mode *currentMode);
    const physicallayer::IIeee80211Mode *setMode(bps bitrate, Hz bandwidth = MHz(20), int numSpatialStream = 1);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif

