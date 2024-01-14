//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// Date: 30/08/2023
// Desc: To use custom CSV file to define the packet lengths and Tx time to send each packet. 
//       CSV file should have two columns, first one for Tx time, second one for packet lengths
//       The application will cycle through the packets in the CSV file until application stop time
// Author: Reuben Lim

#ifndef __INET_UDPBASICCSVAPP_H
#define __INET_UDPBASICCSVAPP_H

#include <vector>
#include <filesystem>

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

namespace inet {

/**
 * UDP application. See NED for more info.
 */
class INET_API UdpBasicCsvApp : public ClockUserModuleMixin<ApplicationBase>, public UdpSocket::ICallback
{
  protected:
    enum SelfMsgKinds { START = 1, SEND, STOP };

    // parameters
    std::vector<L3Address> destAddresses;
    std::vector<std::string> destAddressStr;
    int localPort = -1, destPort = -1;
    clocktime_t startTime;
    clocktime_t stopTime;
    bool dontFragment = false;
    const char *packetName = nullptr;
    std::string m_CSVFilePath;
    std::string m_CSVFileName;
    std::filesystem::path m_CSVFullPath;
    std::string m_PCAPCSVFullPath;
    std::vector<std::vector<std::string>> pcap_content; // To store PCAP CSV contents, 2D matrix, First column is send interval, Second column is packet size
    int numPacketRecord;
    int seed;

    // state
    UdpSocket socket;
    ClockEvent *selfMsg = nullptr;

    // statistics
    int numSent = 0;
    int numReceived = 0;
    int pcapIndex = 0;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    // chooses random destination address
    virtual L3Address chooseDestAddr();
    virtual void sendPacket();
    virtual void processPacket(Packet *msg);
    virtual void setSocketOptions();

    virtual void processStart();
    virtual void processSend();
    virtual void processStop();

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

    virtual void readPcapCsv(); // To read the CSV file containing send intervals and packet sizes from PCAP
    
  public:
    UdpBasicCsvApp() {}
    ~UdpBasicCsvApp();
};

} // namespace inet

#endif

