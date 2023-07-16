//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/udpapp/UdpGCSCmdApp.h"

#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace inet {

Define_Module(UdpGCSCmdApp);

UdpGCSCmdApp::~UdpGCSCmdApp()
{
    cancelAndDelete(selfMsg);
}

void UdpGCSCmdApp::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);

        localPort = par("localPort");
        destPort = par("destPort");
        startTime = par("startTime");
        stopTime = par("stopTime");
        packetName = par("packetName");
        dontFragment = par("dontFragment");
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
        // Write header for Tx CSV
        std::ofstream out(m_CSVFullPath);
        out << "TxTime," << "Packet_Seq," << "Bytes," << "Dest_Addr," << "Packet_Name," << std::endl;
        out.close();
        // Write header for packet drop CSV
        std::string fileName = m_CSVFullPath.string();
        std::ofstream out_pd(fileName.substr(0,fileName.length()-6) + "PacketDrop.csv");
        // out_pd << "RxTime," << "TxTime," << "Packet_Name,"<< "Bytes," << "RSSI," << "U2G_SINR," << "U2U_SINR," << "U2G_BER," << "U2U_BER,"
        //        << "Delay," << "Queueing_Time," << "Backoff_Time," << "U2G_Distance," << "Has_Bit_Error," << "Packet_Drop_Reason" << std::endl;
        // MINIMISE DATA MODE (to revert, search for the term: MINIMISE DATA MODE)
        out_pd << "RxTime," << "TxTime," << "Packet_Name,"<< "Bytes," << "Packet_Drop_Reason" << std::endl;
        out_pd.close();
        if (stopTime >= CLOCKTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        selfMsg = new ClockEvent("sendTimer");
    }
}

void UdpGCSCmdApp::finish()
{
    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    ApplicationBase::finish();
}

void UdpGCSCmdApp::setSocketOptions()
{
    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int dscp = par("dscp");
    if (dscp != -1)
        socket.setDscp(dscp);

    int tos = par("tos");
    if (tos != -1)
        socket.setTos(tos);

    const char *multicastInterface = par("multicastInterface");
    if (multicastInterface[0]) {
        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        NetworkInterface *ie = ift->findInterfaceByName(multicastInterface);
        if (!ie)
            throw cRuntimeError("Wrong multicastInterface setting: no interface named \"%s\"", multicastInterface);
        socket.setMulticastOutputInterface(ie->getInterfaceId());
    }

    bool receiveBroadcast = par("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);

    bool joinLocalMulticastGroups = par("joinLocalMulticastGroups");
    if (joinLocalMulticastGroups) {
        MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
        socket.joinLocalMulticastGroups(mgl);
    }
    socket.setCallback(this);
}

L3Address UdpGCSCmdApp::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    if (destAddresses[k].isUnspecified() || destAddresses[k].isLinkLocal()) {
        L3AddressResolver().tryResolve(destAddressStr[k].c_str(), destAddresses[k]);
    }
    return destAddresses[k];
}

void UdpGCSCmdApp::sendPacket()
{
    // Send a packet to every address
    int numAddr = destAddresses.size();
    for (int i = 0; i < numAddr; i++) {
        if (destAddresses[i].isUnspecified() || destAddresses[i].isLinkLocal()) {
            L3AddressResolver().tryResolve(destAddressStr[i].c_str(), destAddresses[i]);
        }
        std::ostringstream str;
        str << packetName << "-" << numSent;
        Packet *packet = new Packet(str.str().c_str());
        if (dontFragment)
            packet->addTag<FragmentationReq>()->setDontFragment(true);
        const auto& payload = makeShared<ApplicationPacket>();
        simtime_t now = simTime();
        payload->setChunkLength(B(par("messageLength")));
        payload->setSequenceNumber(numSent);
        payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
        packet->insertAtBack(payload);
        // L3Address destAddr = chooseDestAddr();
        L3Address destAddr = destAddresses[i];

        // Added by Reuben on 5/9/2022
        // Store the module CSV file name to the packet tag
        std::string fileName = m_CSVFullPath.string();
        packet->addTagIfAbsent<SnirInd>()->setFileName(fileName.substr(0,fileName.length()-6)); // Update the CSV file path to this current module's (leave out App[0]-Tx.csv)

        int numPacketRecord = par("numPacketRecord");
        if ((numPacketRecord == -1) | (numSent < numPacketRecord)){
            packet->addTagIfAbsent<SnirInd>()->setRecordPacket(true); // Set flag to record packet in CSV
            // Let's log the packet sending application
            std::stringstream os;
            // If this is the first packet being recorded,then include packetName, else exclude. This is to save space
            if (numSent == 0){
                os << now << "," << numSent << "," << packet->getByteLength() << "," << destAddr.str() << "," << packetName;
            }
            else{
                os << now << "," << numSent << "," << packet->getByteLength() << "," << destAddr.str();
            }
            std::ofstream out(m_CSVFullPath, std::ios::app);
            out << os.str() << endl;
            out.close();
        }
        else{
            packet->addTagIfAbsent<SnirInd>()->setRecordPacket(false); // Set flag to NOT record packet in CSV
        }

        emit(packetSentSignal, packet);
        socket.sendTo(packet, destAddr, destPort);

        numSent++;
    }
}

void UdpGCSCmdApp::processStart()
{
    socket.setOutputGate(gate("socketOut"));
    const char *localAddress = par("localAddress");
    socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
    setSocketOptions();

    // Added by Reuben on 18/07/2022 to allow sending to variable number of UAVs
    
    std::string destAddr;
    int numMembers = par("numMemberUAVs");
    if (numMembers > 0) {
        destAddr = "gatewayNode";
        for (int i = 0; i < numMembers; i++) {
            destAddr  = destAddr + " adhocNode[" + std::to_string(i) + "]";
        }
        // destAddrs = destAddr.c_str();
        // std::cout << destAddrs << std::endl;
    }
    else {
        destAddr = std::string(par("destAddresses"));
    }
    const char *destAddrs = destAddr.c_str();
    // std::cout << destAddrs << std::endl;

    cStringTokenizer tokenizer(destAddrs);
    const char *token;

    while ((token = tokenizer.nextToken()) != nullptr) {
        destAddressStr.push_back(token);
        L3Address result;
        L3AddressResolver().tryResolve(token, result);
        if (result.isUnspecified())
            EV_ERROR << "cannot resolve destination address: " << token << endl;
        destAddresses.push_back(result);
    }

    if (!destAddresses.empty()) {
        selfMsg->setKind(SEND);
        processSend();
    }
    else {
        if (stopTime >= CLOCKTIME_ZERO) {
            selfMsg->setKind(STOP);
            scheduleClockEventAt(stopTime, selfMsg);
        }
    }
}

void UdpGCSCmdApp::processSend()
{
    sendPacket();
    clocktime_t d = par("sendInterval");
    if (stopTime < CLOCKTIME_ZERO || getClockTime() + d < stopTime) {
        selfMsg->setKind(SEND);
        scheduleClockEventAfter(d, selfMsg);
    }
    else {
        selfMsg->setKind(STOP);
        scheduleClockEventAt(stopTime, selfMsg);
    }
}

void UdpGCSCmdApp::processStop()
{
    socket.close();
}

void UdpGCSCmdApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case SEND:
                processSend();
                break;

            case STOP:
                processStop();
                break;

            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else
        socket.processMessage(msg);
}

void UdpGCSCmdApp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processPacket(packet);
}

void UdpGCSCmdApp::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void UdpGCSCmdApp::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void UdpGCSCmdApp::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[100];
    sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void UdpGCSCmdApp::processPacket(Packet *pk)
{
    emit(packetReceivedSignal, pk);
    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    std::ofstream out(m_CSVFullPath, std::ios::app);
    out << UdpSocket::getReceivedPacketInfoCSV(pk, numReceived) << endl;
    out.close();
    delete pk;
    numReceived++;
}

void UdpGCSCmdApp::handleStartOperation(LifecycleOperation *operation)
{
    clocktime_t start = std::max(startTime, getClockTime());
    if ((stopTime < CLOCKTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleClockEventAt(start, selfMsg);
    }
}

void UdpGCSCmdApp::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void UdpGCSCmdApp::handleCrashOperation(LifecycleOperation *operation)
{
    cancelClockEvent(selfMsg);
    socket.destroy(); // TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
}

} // namespace inet

