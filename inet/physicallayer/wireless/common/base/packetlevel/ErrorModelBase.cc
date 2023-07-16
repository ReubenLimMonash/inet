//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/base/packetlevel/ErrorModelBase.h"

#include "inet/common/ProtocolTag_m.h"

#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"

namespace inet {

namespace physicallayer {

void ErrorModelBase::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        const char *corruptionModeString = par("corruptionMode");
        if (!strcmp("packet", corruptionModeString))
            corruptionMode = CorruptionMode::CM_PACKET;
        else if (!strcmp("chunk", corruptionModeString))
            corruptionMode = CorruptionMode::CM_CHUNK;
        else if (!strcmp("byte", corruptionModeString))
            corruptionMode = CorruptionMode::CM_BYTE;
        else if (!strcmp("bit", corruptionModeString))
            corruptionMode = CorruptionMode::CM_BIT;
        else
            throw cRuntimeError("Unknown corruption mode");
        const char *snirModeString = par("snirMode");
        if (!strcmp("min", snirModeString))
            snirMode = SnirMode::SM_MIN;
        else if (!strcmp("mean", snirModeString))
            snirMode = SnirMode::SM_MEAN;
        else
            throw cRuntimeError("Unknown SNIR mode: '%s'", snirModeString);
        snirOffset = inet::math::dB2fraction(par("snirOffset"));
    }
}

double ErrorModelBase::getScalarSnir(const ISnir *snir) const
{
    double scalarSnir;
    if (snirMode == SnirMode::SM_MIN)
        scalarSnir = snir->getMin();
    else if (snirMode == SnirMode::SM_MEAN)
        scalarSnir = snir->getMean();
    else
        throw cRuntimeError("Unknown SNIR mode");
    return scalarSnir * snirOffset;
}

bool ErrorModelBase::hasProbabilisticError(b length, double ber) const
{
    ASSERT(0.0 < ber && ber <= 1.0);
    return dblrand() < 1 - std::pow((1 - ber), length.get());
}

Packet *ErrorModelBase::corruptBits(const Packet *packet, double ber, bool& isCorrupted) const
{
    std::vector<bool> corruptedBits;
    const auto& all = packet->peekAllAsBits();
    for (bool bit : all->getBits()) {
        if (hasProbabilisticError(b(1), ber)) {
            isCorrupted = true;
            bit = !bit;
        }
        corruptedBits.push_back(bit);
    }
    return new Packet(packet->getName(), makeShared<BitsChunk>(corruptedBits));
}

Packet *ErrorModelBase::corruptBytes(const Packet *packet, double ber, bool& isCorrupted) const
{
    std::vector<uint8_t> corruptedBytes;
    const auto& all = packet->peekAllAsBytes();
    for (uint8_t byte : all->getBytes()) {
        if (hasProbabilisticError(B(1), ber)) {
            isCorrupted = true;
            byte = ~byte;
        }
        corruptedBytes.push_back(byte);
    }
    return new Packet(packet->getName(), makeShared<BytesChunk>(corruptedBytes));
}

Packet *ErrorModelBase::corruptChunks(const Packet *packet, double ber, bool& isCorrupted) const
{
    b offset = b(0);
    auto corruptedPacket = new Packet(packet->getName());
    while (auto chunk = packet->peekAt(offset, b(-1), Chunk::PF_ALLOW_NULLPTR)) {
        if (hasProbabilisticError(chunk->getChunkLength(), ber)) {
            isCorrupted = true;
            auto corruptedChunk = chunk->dupShared();
            corruptedChunk->markIncorrect();
            corruptedPacket->insertAtBack(corruptedChunk);
        }
        else
            corruptedPacket->insertAtBack(chunk);
        offset += chunk->getChunkLength();
    }
    return corruptedPacket;
}

Packet *ErrorModelBase::corruptPacket(const Packet *packet, bool& isCorrupted) const
{
    isCorrupted = true;
    auto corruptedPacket = packet->dup();
    corruptedPacket->setBitError(true);
    return corruptedPacket;
}

Packet *ErrorModelBase::computeCorruptedPacket(const Packet *packet, double ber) const
{
    bool isCorrupted = false;
    Packet *corruptedPacket = nullptr;
    // TODO this while loop looks bad, but we don't have any other chance now, because the decision whether the reception is successful or not has been already made
    while (!isCorrupted) {
        switch (corruptionMode) {
            case CorruptionMode::CM_PACKET:
                corruptedPacket = corruptPacket(packet, isCorrupted);
                break;
            case CorruptionMode::CM_CHUNK:
                corruptedPacket = corruptChunks(packet, ber, isCorrupted);
                break;
            case CorruptionMode::CM_BYTE:
                corruptedPacket = corruptBytes(packet, ber, isCorrupted);
                break;
            case CorruptionMode::CM_BIT:
                corruptedPacket = corruptBits(packet, ber, isCorrupted);
                break;
            default:
                throw cRuntimeError("Unknown corruption mode");
        }
        if (!isCorrupted)
            delete corruptedPacket;
    }
    return corruptedPacket;
}

Packet *ErrorModelBase::computeCorruptedPacket(const ISnir *snir) const
{
    auto transmittedPacket = snir->getReception()->getTransmission()->getPacket();
    auto ber = computeBitErrorRate(snir, IRadioSignal::SIGNAL_PART_WHOLE);
    auto receivedPacket = computeCorruptedPacket(transmittedPacket, ber);
    std::string packetName = receivedPacket->getName();
    // Added for statistics collection (02/11/2022)
    double queueingTime = -1.0;
    double backoffTime = -1.0;
    double txDistance = NaN;
    double sinr = NaN;
    double U2GSinr = NaN;
    double U2USinr = NaN;
    double U2GDistance = NaN;
    double U2GBer = NaN;
    double U2UBer = NaN;
    int retryCount = 0;
    bool recordPacket = true;
    W rssi = W(NaN);
    std::string fileName = "NaN";
    if ((packetName.compare(0, 4, "Wlan") != 0) && (packetName.compare(0, 3, "arp") != 0)) {
        // Checking for queueing time and other stats
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
            U2GBer = errorRateInd->getU2GBer();
            U2UBer = errorRateInd->getU2UBer();
        }
    }
    // -------------------------

    receivedPacket->clearTags();
    receivedPacket->addTag<PacketProtocolTag>()->setProtocol(transmittedPacket->getTag<PacketProtocolTag>()->getProtocol());

    // Added for statistics collection (02/11/2022)
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
    receivedPacket->setBitError(true);
    // -------------------------

    return receivedPacket;
}

} // namespace physicallayer

} // namespace inet

