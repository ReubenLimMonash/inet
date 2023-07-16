//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ReceiverBase.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211TransmissionBase.h"
// The following were added by Reuben on 16/7/2022 for monitor mode
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/DscpTag_m.h"
#include "inet/networklayer/common/HopLimitTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/common/TosTag_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace inet {

namespace physicallayer {

Ieee80211ReceiverBase::Ieee80211ReceiverBase() :
    modeSet(nullptr),
    band(nullptr),
    channel(nullptr)
{
}

Ieee80211ReceiverBase::~Ieee80211ReceiverBase()
{
    delete channel;
}

void Ieee80211ReceiverBase::initialize(int stage)
{
    FlatReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *opMode = par("opMode");
        setModeSet(*opMode ? Ieee80211ModeSet::getModeSet(opMode) : nullptr);
        const char *bandName = par("bandName");
        setBand(*bandName != '\0' ? Ieee80211CompliantBands::getBand(bandName) : nullptr);
        int channelNumber = par("channelNumber");
        if (channelNumber != -1)
            setChannelNumber(channelNumber);
        // Added by Reuben Lim on 16/7/2022
        // For monitor mode
        enableMonitorMode = par("monitorMode");
        m_CSVFilePath = par("csvFilePath").stdstringValue(); // Folder to store CSV file
        m_CSVFileName = par("csvFileName").stdstringValue(); // File name to store CSV file
        // First, let's try to create the directory
        if (!std::filesystem::exists(m_CSVFilePath)){
            std::error_code err;
            int dirExist = std::filesystem::create_directory(m_CSVFilePath, err);
        }
        std::filesystem::path dir (m_CSVFilePath.c_str());
        std::filesystem::path file (m_CSVFileName.c_str());
        m_CSVFullPath = dir / file ;
        if (enableMonitorMode){
            std::ofstream out(m_CSVFullPath);
            out << "RxTime," << "PkCreationTime," << "Packet_Name," << "Bytes," << "RSSI," << "U2G_SINR," << "U2U_SINR," << "U2G_BER," << "U2U_BER," <<
                    "Delay," << "Queueing_Time," << "Backoff_Time," << "U2G_Distance," << "HasError" << std::endl;
            out.close();
        }
    }
}

std::ostream& Ieee80211ReceiverBase::printToStream(std::ostream& stream, int level, int evFlags) const
{
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(modeSet, printFieldToString(modeSet, level + 1, evFlags))
               << EV_FIELD(band, printFieldToString(band, level + 1, evFlags));
    if (level <= PRINT_LEVEL_INFO)
        stream << EV_FIELD(channel, printFieldToString(channel, level + 1, evFlags));
    return FlatReceiverBase::printToStream(stream, level);
}

void Ieee80211ReceiverBase::setModeSet(const Ieee80211ModeSet *modeSet)
{
    this->modeSet = modeSet;
}

void Ieee80211ReceiverBase::setBand(const IIeee80211Band *band)
{
    if (this->band != band) {
        this->band = band;
        if (channel != nullptr)
            setChannel(new Ieee80211Channel(band, channel->getChannelNumber()));
    }
}

void Ieee80211ReceiverBase::setChannel(const Ieee80211Channel *channel)
{
    if (this->channel != channel) {
        delete this->channel;
        this->channel = channel;
        setCenterFrequency(channel->getCenterFrequency());
    }
}

void Ieee80211ReceiverBase::setChannelNumber(int channelNumber)
{
    if (channel == nullptr || channelNumber != channel->getChannelNumber())
        setChannel(new Ieee80211Channel(band, channelNumber));
}

const IReceptionResult *Ieee80211ReceiverBase::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions, const double distance) const
{
    auto transmission = check_and_cast<const Ieee80211TransmissionBase *>(reception->getTransmission());
    auto receptionResult = FlatReceiverBase::computeReceptionResult(listening, reception, interference, snir, decisions, distance);
    auto packet = const_cast<Packet *>(receptionResult->getPacket());
    packet->addTagIfAbsent<Ieee80211ModeInd>()->setMode(transmission->getMode());
    packet->addTagIfAbsent<Ieee80211ChannelInd>()->setChannel(transmission->getChannel());
    std::string fileName = m_CSVFullPath.string();
    packet->addTagIfAbsent<SnirInd>()->setFileName(fileName.substr(0,fileName.length()-8)); // Update the CSV file path to this current module's (leave out Wlan.csv)
    // Check if the receiver is the 
    if (enableMonitorMode){
        std::ofstream out(m_CSVFullPath, std::ios::app);
        out << getReceivedPacketInfoCSV(packet) << endl;
        out.close();
    }
    return receptionResult;
}

std::string Ieee80211ReceiverBase::getReceivedPacketInfoCSV(Packet *pk)
{
    simtime_t now = simTime();
    simtime_t packetCreationTime;
    simtime_t delay = 0;
    W rssi = pk->getTag<SignalPowerInd>()->getPower();
    auto errorRateInd = pk->getTag<ErrorRateInd>();
    double ber = errorRateInd->getBitErrorRate();
    double U2GBer = errorRateInd->getU2GBer();
    double U2UBer = errorRateInd->getU2UBer();
    std::string packetName = pk->getName();
    auto snirInd = pk->getTag<SnirInd>();
    double snir = snirInd->getMinimumSnir();
    double U2GSnir = snirInd->getU2GSnir();
    double U2USnir = snirInd->getU2USnir();
    double txDistance = snirInd->getTxDistance();
    double U2GDistance = snirInd->getU2GDistance();
    double backoffTime = snirInd->getBackoffPeriod();
    double queueingTime = snirInd->getQueueingTime(); 
    int retryCount = snirInd->getRetryCount();
    std::stringstream os;
    // std::cout << packetName << std::endl;
    // std::cout << retryCount << std::endl;
    // if ((packetName[-3] == "A") && (packetName[-2] == "c") && (packetName[-1] == "k")) {
    if ((packetName.compare(0, 4, "Wlan")) == 0) {
        // For Ack packets
        os << now << "," << "Null" << ",";
    }
    else if ((packetName.compare(0, 3, "arp")) == 0) {
        // For ARP packets
        os << now << "," << "Null" << ",";
    }
    else {
        packetCreationTime = pk->peekData()->getAllTags<CreationTimeTag>()[0].getTag()->getCreationTime();
        delay = now - packetCreationTime;
        os << now << "," << packetCreationTime << ",";
    }
    os << packetName << "," << pk->getByteLength() << "," << rssi << "," << U2GSnir << "," << U2USnir << "," << U2GBer << "," << U2UBer << ",";
    os << delay << "," << queueingTime << "," << backoffTime << "," << U2GDistance << "," << pk->hasBitError();
    return os.str();
}

} // namespace physicallayer

} // namespace inet

