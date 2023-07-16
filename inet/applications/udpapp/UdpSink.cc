//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/applications/udpapp/UdpSink.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace inet {

Define_Module(UdpSink);

UdpSink::~UdpSink()
{
    cancelAndDelete(selfMsg);
}

void UdpSink::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numReceived = 0;
        WATCH(numReceived);

        localPort = par("localPort");
        startTime = par("startTime");
        stopTime = par("stopTime");
        m_CSVFilePath = par("csvFilePath").stdstringValue(); // Folder to store CSV file
        m_CSVFileName = par("csvFileName").stdstringValue(); // File name to store CSV file
        // First, let's try to create the directory
        if (!std::filesystem::exists(m_CSVFilePath)){
            std::error_code err;
            int dirExist = std::filesystem::create_directory(m_CSVFilePath, err);
        }
        // if (!dirExist){throw cRuntimeError("CSV File Directory Failed To Be Created, Check Parent Path.");}
        std::filesystem::path dir (m_CSVFilePath.c_str());
        std::filesystem::path file (m_CSVFileName.c_str());
        m_CSVFullPath = dir / file ;
        std::ofstream out(m_CSVFullPath);
        // out << "RxTime," << "TxTime," << "Packet_Name,"<< "Bytes," << "RSSI," << "U2G_SINR," << "U2U_SINR," << "U2G_BER," << "U2U_BER," <<
        //         "Src_Addr," << "Dest_Addr," << "Hop_Count," << "Delay," << "Queueing_Time," << "Backoff_Time," << 
        //         "U2G_Distance," << "Retry_Count" << std::endl;
        // MINIMISE DATA MODE (to revert, search for the term: MINIMISE DATA MODE)
        out << "RxTime," << "TxTime," << "Packet_Name,"<< "Bytes," << "RSSI," << "U2G_SINR," << "U2G_BER," <<
                "Retry_Count," << "Dest_Addr," << "U2G_Distance" << std::endl;
        out.close();
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        selfMsg = new cMessage("UDPSinkTimer");
    }
}

void UdpSink::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case STOP:
                processStop();
                break;

            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else if (msg->arrivedOn("socketIn"))
        socket.processMessage(msg);
    else
        throw cRuntimeError("Unknown incoming gate: '%s'", msg->getArrivalGate()->getFullName());
}

void UdpSink::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processPacket(packet);
}

void UdpSink::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void UdpSink::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void UdpSink::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[50];
    sprintf(buf, "rcvd: %d pks", numReceived);
    getDisplayString().setTagArg("t", 0, buf);
}

void UdpSink::finish()
{
    ApplicationBase::finish();
    EV_INFO << getFullPath() << ": received " << numReceived << " packets\n";
}

void UdpSink::setSocketOptions()
{
    bool receiveBroadcast = par("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);

    MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
    socket.joinLocalMulticastGroups(mgl);

    // join multicastGroup
    const char *groupAddr = par("multicastGroup");
    multicastGroup = L3AddressResolver().resolve(groupAddr);
    if (!multicastGroup.isUnspecified()) {
        if (!multicastGroup.isMulticast())
            throw cRuntimeError("Wrong multicastGroup setting: not a multicast address: %s", groupAddr);
        socket.joinMulticastGroup(multicastGroup);
    }
    socket.setCallback(this);
}

void UdpSink::processStart()
{
    socket.setOutputGate(gate("socketOut"));
    socket.bind(localPort);
    setSocketOptions();

    if (stopTime >= SIMTIME_ZERO) {
        selfMsg->setKind(STOP);
        scheduleAt(stopTime, selfMsg);
    }
}

void UdpSink::processStop()
{
    if (!multicastGroup.isUnspecified())
        socket.leaveMulticastGroup(multicastGroup); // FIXME should be done by socket.close()
    socket.close();
}

void UdpSink::processPacket(Packet *pk)
{
    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    emit(packetReceivedSignal, pk);

    // Only record the packet in CSV file if the record flag is set
    auto snirInd = pk->getTag<SnirInd>();
    bool recordPacket = snirInd->getRecordPacket();
    if (recordPacket) {
        std::ofstream out(m_CSVFullPath, std::ios::app);
        out << UdpSocket::getReceivedPacketInfoCSV(pk, numReceived) << endl;
        out.close();
    }
    delete pk;

    numReceived++;
}

void UdpSink::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t start = std::max(startTime, simTime());
    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
}

void UdpSink::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    if (!multicastGroup.isUnspecified())
        socket.leaveMulticastGroup(multicastGroup); // FIXME should be done by socket.close()
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void UdpSink::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    if (operation->getRootModule() != getContainingNode(this)) { // closes socket when the application crashed only
        if (!multicastGroup.isUnspecified())
            socket.leaveMulticastGroup(multicastGroup); // FIXME should be done by socket.close()
        socket.destroy(); // TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
    }
}

} // namespace inet

