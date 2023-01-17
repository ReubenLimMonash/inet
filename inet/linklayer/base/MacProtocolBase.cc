//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/base/MacProtocolBase.h"

#include "inet/common/IInterfaceRegistrationListener.h"
#include "inet/common/ModuleAccess.h"

// Included the following to save results to CSV
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/common/TimeTag.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace inet {

MacProtocolBase::MacProtocolBase()
{
}

MacProtocolBase::~MacProtocolBase()
{
    delete currentTxFrame;
}

MacAddress MacProtocolBase::parseMacAddressParameter(const char *addrstr)
{
    MacAddress address;

    if (!strcmp(addrstr, "auto"))
        // assign automatic address
        address = MacAddress::generateAutoAddress();
    else
        address.setAddress(addrstr);

    return address;
}

void MacProtocolBase::initialize(int stage)
{
    LayeredProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        upperLayerInGateId = findGate("upperLayerIn");
        upperLayerOutGateId = findGate("upperLayerOut");
        lowerLayerInGateId = findGate("lowerLayerIn");
        lowerLayerOutGateId = findGate("lowerLayerOut");
        currentTxFrame = nullptr;
        hostModule = findContainingNode(this);
        // // Added by Reuben Lim on 04/09/2022
        // // For recording packet drop stats
        // m_CSVFilePath = par("csvFilePath").stdstringValue(); // Folder to store CSV file
        // m_CSVFileName = par("csvFileName").stdstringValue(); // File name to store CSV file
        // // First, let's try to create the directory
        // if (!std::filesystem::exists(m_CSVFilePath)){
        //     std::error_code err;
        //     int dirExist = std::filesystem::create_directory(m_CSVFilePath, err);
        // }
        // std::filesystem::path dir (m_CSVFilePath.c_str());
        // std::filesystem::path file (m_CSVFileName.c_str());
        // m_CSVFullPath = dir / file ;
        // std::ofstream out(m_CSVFullPath);
        // out << "RxTime," << "PkCreationTime," << "Packet_Name," << "Bytes," << "RSSI," << "SINR," <<
        //         "Delay," << "Queueing_Time," << "Backoff_Time" << "Distance," << "PacketDropReason" << std::endl;
        // out.close();
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION)
        registerInterface();
}

void MacProtocolBase::registerInterface()
{
    ASSERT(networkInterface == nullptr);
    networkInterface = getContainingNicModule(this);
    configureNetworkInterface();
}

void MacProtocolBase::sendUp(cMessage *message)
{
    if (message->isPacket())
        emit(packetSentToUpperSignal, message);
    send(message, upperLayerOutGateId);
}

void MacProtocolBase::sendDown(cMessage *message)
{
    if (message->isPacket())
        emit(packetSentToLowerSignal, message);
    send(message, lowerLayerOutGateId);
}

bool MacProtocolBase::isUpperMessage(cMessage *message) const
{
    return message->getArrivalGateId() == upperLayerInGateId;
}

bool MacProtocolBase::isLowerMessage(cMessage *message) const
{
    return message->getArrivalGateId() == lowerLayerInGateId;
}

void MacProtocolBase::handleMessageWhenDown(cMessage *msg)
{
    if (!msg->isSelfMessage() && msg->getArrivalGateId() == lowerLayerInGateId) {
        EV << "Interface is turned off, dropping packet\n";
        delete msg;
    }
    else
        LayeredProtocolBase::handleMessageWhenDown(msg);
}

void MacProtocolBase::deleteCurrentTxFrame()
{
    delete currentTxFrame;
    currentTxFrame = nullptr;
}

void MacProtocolBase::dropCurrentTxFrame(PacketDropDetails& details)
{
    emit(packetDroppedSignal, currentTxFrame, &details);
    delete currentTxFrame;
    currentTxFrame = nullptr;
}

void MacProtocolBase::flushQueue(PacketDropDetails& details)
{
    // code would look slightly nicer with a pop() function that returns nullptr if empty
    if (txQueue)
        while (canDequeuePacket()) {
            auto packet = dequeuePacket();
            emit(packetDroppedSignal, packet, &details); // FIXME this signal lumps together packets from the network and packets from higher layers! separate them
            delete packet;
        }
}

void MacProtocolBase::clearQueue()
{
    if (txQueue)
        while (canDequeuePacket())
            delete dequeuePacket();
}

void MacProtocolBase::handleStartOperation(LifecycleOperation *operation)
{
    networkInterface->setState(NetworkInterface::State::UP);
    networkInterface->setCarrier(true);
}

void MacProtocolBase::handleStopOperation(LifecycleOperation *operation)
{
    PacketDropDetails details;
    details.setReason(INTERFACE_DOWN);
    if (currentTxFrame) {
        // Save packet drop detail to file
        if (currentTxFrame->findTag<SnirInd>()){
            std::string packetDropCSV = currentTxFrame->getTag<SnirInd>()->getFileName() + "PacketDrop.csv"; // For saving to CSV file
            std::filesystem::path csvFilePath (packetDropCSV.c_str()); // For saving to CSV file
            std::string packetInfo = MacProtocolBase::getPacketInfoCSV(currentTxFrame, "INTERFACE_DOWN");
            std::ofstream out(csvFilePath, std::ios::app);
            out << packetInfo << endl;
            out.close();
        }
        dropCurrentTxFrame(details);
    }
    flushQueue(details);

    networkInterface->setCarrier(false);
    networkInterface->setState(NetworkInterface::State::DOWN);
}

void MacProtocolBase::handleCrashOperation(LifecycleOperation *operation)
{
    deleteCurrentTxFrame();
    clearQueue();

    networkInterface->setCarrier(false);
    networkInterface->setState(NetworkInterface::State::DOWN);
}

void MacProtocolBase::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));
}


queueing::IPacketQueue *MacProtocolBase::getQueue(cGate *gate) const
{
    // TODO use findConnectedModule() when the function is updated
    for (auto g = gate->getPreviousGate(); g != nullptr; g = g->getPreviousGate()) {
        if (g->getType() == cGate::OUTPUT) {
            auto m = dynamic_cast<queueing::IPacketQueue *>(g->getOwnerModule());
            if (m)
                return m;
        }
    }
    throw cRuntimeError("Gate %s is not connected to a module of type queueing::IPacketQueue", gate->getFullPath().c_str());
}

bool MacProtocolBase::canDequeuePacket() const
{
    return txQueue && txQueue->canPullSomePacket(gate(upperLayerInGateId)->getPathStartGate());
}

Packet *MacProtocolBase::dequeuePacket()
{
    Packet *packet = txQueue->dequeuePacket();
    take(packet);
    packet->setArrival(getId(), upperLayerInGateId, simTime());
    return packet;
}

// Added by Reuben on 4/9/2022 to record packet drop stats
std::string MacProtocolBase::getPacketInfoCSV(Packet *pk, std::string reason)
{
    simtime_t now = simTime();
    simtime_t packetCreationTime;
    // ------------------
    simtime_t delay = 0;
    W rssi = W(NaN);
    double ber = NaN;
    double U2GBer = NaN;
    double U2UBer = NaN;
    double snir = NaN;
    double U2GSnir = NaN;
    double U2USnir = NaN;
    double txDistance = NaN;
    double U2GDistance = NaN;
    double backoffTime = NaN;
    double queueingTime = NaN;  
    // -------------------
    std::string packetName = pk->getName();
    std::stringstream os;
    if ((packetName.compare(0, 4, "Wlan")) == 0) {
        // For Ack packets
        os << now << "," << "Null" << ",";
    }
    else if ((packetName.compare(0, 3, "arp")) == 0) {
        // For ARP packets
        os << now << "," << "Null" << ",";
    }
    else {
        if (pk->findTag<SignalPowerInd>()){
            rssi = pk->getTag<SignalPowerInd>()->getPower();
        }
        if (pk->findTag<ErrorRateInd>()){
            auto errorRateInd = pk->getTag<ErrorRateInd>();
            ber = errorRateInd->getBitErrorRate();
            U2GBer = errorRateInd->getU2GBer();
            U2UBer = errorRateInd->getU2UBer();
        }
        packetCreationTime = pk->peekData()->getAllTags<CreationTimeTag>()[0].getTag()->getCreationTime();
        delay = now - packetCreationTime;
        auto snirInd = pk->getTag<SnirInd>();
        snir = snirInd->getMinimumSnir();
        U2GSnir = snirInd->getU2GSnir();
        U2USnir = snirInd->getU2USnir();
        txDistance = snirInd->getTxDistance();
        U2GDistance = snirInd->getU2GDistance();
        backoffTime = snirInd->getBackoffPeriod();
        queueingTime = snirInd->getQueueingTime(); 
        os << now << "," << packetCreationTime << ",";
    }
    os << packetName << "," << pk->getByteLength() << "," << rssi << "," << U2GSnir << "," << U2USnir << "," << U2GBer << "," << U2UBer << ",";
    os << delay << "," << queueingTime << "," << backoffTime << "," << U2GDistance << "," << pk->hasBitError() << ",";
    os << reason;

    // Uncomment for debugging
    // std::cout << os.str() << std::endl;

    return os.str();
}

// // Added by Reuben on 5/9/2022 to record packet drop stats
// std::filesystem::path MacProtocolBase::getCSVFilePath()
// {
//     return m_CSVFullPath;
// }

} // namespace inet

