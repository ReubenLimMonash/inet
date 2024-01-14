//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/ReceiverBase.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandNoiseBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionDecision.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/ReceptionResult.h"
#include "inet/physicallayer/wireless/common/signal/Interference.h"

namespace inet {
namespace physicallayer {

bool ReceiverBase::computeIsReceptionPossible(const IListening *listening, const ITransmission *transmission) const
{
    return true;
}

bool ReceiverBase::computeIsReceptionPossible(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part) const
{
    return true;
}

bool ReceiverBase::computeIsReceptionAttempted(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference) const
{
    if (!computeIsReceptionPossible(listening, reception, part))
        return false;
    else if (simTime() == reception->getStartTime(part)) {
        // TODO isn't there a better way for this optimization? see also in RadioMedium::isReceptionAttempted
        auto transmission = reception->getReceiver()->getReceptionInProgress();
        return transmission == nullptr || transmission == reception->getTransmission();
    }
    else {
        // determining whether the reception is attempted or not for the future
        auto radio = reception->getReceiver();
        auto radioMedium = radio->getMedium();
        auto interferingReceptions = interference->getInterferingReceptions();
        for (auto interferingReception : *interferingReceptions) {
            auto isPrecedingReception = interferingReception->getStartTime() < reception->getStartTime() ||
                (interferingReception->getStartTime() == reception->getStartTime() &&
                 interferingReception->getTransmission()->getId() < reception->getTransmission()->getId());
            if (isPrecedingReception) {
                auto interferingTransmission = interferingReception->getTransmission();
                if (interferingReception->getStartTime() <= simTime()) {
                    if (radio->getReceptionInProgress() == interferingTransmission)
                        return false;
                }
                else if (radioMedium->isReceptionAttempted(radio, interferingTransmission, part))
                    return false;
            }
        }
        return true;
    }
}

const IReceptionDecision *ReceiverBase::computeReceptionDecision(const IListening *listening, const IReception *reception, IRadioSignal::SignalPart part, const IInterference *interference, const ISnir *snir) const
{
    auto isReceptionPossible = computeIsReceptionPossible(listening, reception, part);
    auto isReceptionAttempted = isReceptionPossible && computeIsReceptionAttempted(listening, reception, part, interference);
    auto isReceptionSuccessful = isReceptionAttempted && computeIsReceptionSuccessful(listening, reception, part, interference, snir);
    return new ReceptionDecision(reception, part, isReceptionPossible, isReceptionAttempted, isReceptionSuccessful);
}

const IReceptionResult *ReceiverBase::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions, const double distance) const
{
    bool isReceptionSuccessful = true;
    for (auto decision : *decisions)
        isReceptionSuccessful &= decision->isReceptionSuccessful();
    auto packet = computeReceivedPacket(snir, isReceptionSuccessful);
    auto signalPower = computeSignalPower(listening, snir, interference);
    // 16/04/2023 Changed to only record RSSI for G2U / U2G links
    // if (!std::isnan(signalPower.get())) {
    //     auto signalPowerInd = packet->addTagIfAbsent<SignalPowerInd>();
    //     signalPowerInd->setPower(signalPower);
    // }
    // Using the FileName tag of packets to get the sender name (get previous node)
    // ASSUMING AT THIS POINT, THE CURRENT NODE HAS NOT UPDATED THE fileName TAG YET
    auto snirInd = packet->addTagIfAbsent<SnirInd>();
    std::string filename = snirInd->getFileName();
    std::string packetName = packet->getName();
    // std::cout << packetName << std::endl;
    if (filename.compare(0,3,"NaN") != 0) { // If not NaN
        // If previous node is GCS, and the packet name is CNC..., the SINR calculated is for G2U link
        if ((filename.compare(filename.length()-4, 3, "GCS") == 0) && (packetName.compare(0, 3, "CNC") == 0)){
            snirInd->setU2GSnir(snir->getMin());
            snirInd->setU2GDistance(distance);
            if (!std::isnan(signalPower.get())) {
                auto signalPowerInd = packet->addTagIfAbsent<SignalPowerInd>();
                signalPowerInd->setPower(signalPower);
            }
        }
        // If previous node is gateway UAV, and the packet name is not CNC..., the SINR calculated is for U2G link
        else if ((filename.compare(filename.length()-3, 2, "GW") == 0) && (packetName.compare(0, 3, "CNC") != 0)){
            snirInd->setU2GSnir(snir->getMin());
            snirInd->setU2GDistance(distance);
            if (!std::isnan(signalPower.get())) {
                auto signalPowerInd = packet->addTagIfAbsent<SignalPowerInd>();
                signalPowerInd->setPower(signalPower);
            }
        }
        // If packet is GatewayVideo, the link is U2G
        else if (packetName.compare(0, 12, "GatewayVideo") == 0){
            snirInd->setU2GSnir(snir->getMin());
            snirInd->setU2GDistance(distance);
            if (!std::isnan(signalPower.get())) {
                auto signalPowerInd = packet->addTagIfAbsent<SignalPowerInd>();
                signalPowerInd->setPower(signalPower);
            }
        }
        else {
            snirInd->setU2USnir(snir->getMin());
        }
    }
    snirInd->setMinimumSnir(snir->getMin());
    snirInd->setMaximumSnir(snir->getMax());
    snirInd->setAverageSnir(snir->getMean());
    snirInd->setTxDistance(distance);
    auto signalTimeInd = packet->addTagIfAbsent<SignalTimeInd>();
    signalTimeInd->setStartTime(reception->getStartTime());
    signalTimeInd->setEndTime(reception->getEndTime());
    return new ReceptionResult(reception, decisions, packet);
}

W ReceiverBase::computeSignalPower(const IListening *listening, const ISnir *snir, const IInterference *interference) const
{
    if (!dynamic_cast<const NarrowbandNoiseBase *>(snir->getNoise()))
        return W(0);
    else {
        auto analogModel = snir->getReception()->getTransmission()->getMedium()->getAnalogModel();
        auto signalPlusNoise = check_and_cast<const NarrowbandNoiseBase *>(analogModel->computeNoise(snir->getReception(), snir->getNoise()));
        auto signalPower = signalPlusNoise == nullptr ? W(NaN) : signalPlusNoise->computeMinPower(listening->getStartTime(), listening->getEndTime());
        delete signalPlusNoise;
        return signalPower;
    }
}

Packet *ReceiverBase::computeReceivedPacket(const ISnir *snir, bool isReceptionSuccessful) const
{
    auto transmittedPacket = snir->getReception()->getTransmission()->getPacket();
    auto receivedPacket = transmittedPacket->dup();
    std::string packetName = receivedPacket->getName();
    double queueingTime = -1.0;
    double backoffTime = -1.0;
    double txDistance = NaN;
    double sinr = NaN;
    double U2GSinr = NaN;
    double U2USinr = NaN;
    double U2GDistance = NaN;
    W rssi = W(NaN);
    double ber = NaN;
    double U2GBer = NaN;
    double U2UBer = NaN;
    int retryCount = 0;
    bool recordPacket = true;
    std::string fileName = "NaN";
    // Checking for queueing time and other stats
    if ((packetName.compare(0, 4, "Wlan") != 0) && (packetName.compare(0, 3, "arp") != 0)) {
        if (receivedPacket->findTag<SnirInd>()){
            auto snirInd = receivedPacket->getTag<SnirInd>();
            queueingTime = snirInd->getQueueingTime();
            backoffTime = snirInd->getBackoffPeriod();
            txDistance = snirInd->getTxDistance();
            sinr = snirInd->getMinimumSnir();
            U2GSinr = snirInd->getU2GSnir();
            U2USinr = snirInd->getU2USnir();
            U2GDistance = snirInd->getU2GDistance();
            fileName = snirInd->getFileName();
            retryCount = snirInd->getRetryCount();
            recordPacket = snirInd->getRecordPacket();
        }
        else{
            std::cout << "No SnirInd!" << std::endl;
            std::cout << packetName << std::endl;
        }
        if (receivedPacket->findTag<SignalPowerInd>()){
            rssi = receivedPacket->getTag<SignalPowerInd>()->getPower();
        }
        if (receivedPacket->findTag<ErrorRateInd>()){
            auto errorRateInd = receivedPacket->getTag<ErrorRateInd>();
            ber = errorRateInd->getBitErrorRate();
            U2GBer = errorRateInd->getU2GBer();
            U2UBer = errorRateInd->getU2UBer();
        }
    }

    receivedPacket->clearTags(); // Need to add back the tags cleared
    receivedPacket->addTag<PacketProtocolTag>()->setProtocol(transmittedPacket->getTag<PacketProtocolTag>()->getProtocol());
    
    // Added for statistics collection
    if ((packetName.compare(0, 4, "Wlan") != 0) && (packetName.compare(0, 3, "arp") != 0)) {
        auto snirInd = receivedPacket->addTagIfAbsent<SnirInd>();
        snirInd->setQueueingTime(queueingTime);
        snirInd->setBackoffPeriod(backoffTime);
        snirInd->setTxDistance(txDistance);
        snirInd->setMinimumSnir(sinr);
        snirInd->setU2GSnir(U2GSinr);
        snirInd->setU2USnir(U2USinr);
        snirInd->setU2GDistance(U2GDistance);
        snirInd->setRetryCount(retryCount);
        snirInd->setRecordPacket(recordPacket);
        auto signalPowerInd = receivedPacket->addTagIfAbsent<SignalPowerInd>();
        signalPowerInd->setPower(rssi);
        auto errorRateInd = receivedPacket->addTagIfAbsent<ErrorRateInd>();
        errorRateInd->setBitErrorRate(ber);
        errorRateInd->setU2GBer(U2GBer);
        errorRateInd->setU2UBer(U2UBer);
        if (fileName.compare(0,fileName.length(),"NaN") != 0){
            snirInd->setFileName(fileName);
        }
    }
    
    if (!isReceptionSuccessful)
        receivedPacket->setBitError(true);
    return receivedPacket;
}

} // namespace physicallayer
} // namespace inet

