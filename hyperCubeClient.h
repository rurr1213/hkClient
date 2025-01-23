#include <stdio.h>
#include <queue>

#include "tcp.h"
#include "sthread.h"
#include "Messages.h"
#include "mserdes.h"
#include "Packet.h"
#include "GenTime.h"

#define HYPERCUBE_SERVER_NAME_PRIMARY "primary.hyperkube.net"
#define HYPERCUBE_SERVER_NAME_SECONDARY "secondary.hyperkube.net"
#define HYPERCUBE_SERVER_PORT 5054

#define HYPERCUBE_CONNECTIONINTERVAL_MS (8*1000)		// connection attempt interval in milliseconds
#define HYPERCUBE_LOCALPINGACKTIME      (8*100000000) 	// local ping max ack time

#ifdef _WIN64
#define uint128_t   UUID
#else
#define uint128_t   __int128
#endif

class IHyperCubeClientCore
{
    Ctcp::Client& rtcpClient;
public:
    IHyperCubeClientCore(Ctcp::Client& _rtcpClient) :
        rtcpClient{ _rtcpClient } {};

//    virtual bool tcpClose(void) { return rtcpClient.close(); }
    virtual bool tcpConnect(std::string addrString, int port) { return rtcpClient.connect(addrString, port); }
    virtual bool tcpSocketValid(void) { return rtcpClient.socketValid(); }
    virtual bool tcpDnsLookup(std::string _hostName, std::string& _ipAddress) { return rtcpClient.dnsLookup(_hostName, _ipAddress); }
    virtual int tcpGetSocket(void) { return (int)rtcpClient.getSocket(); }
    int tcpRecv(char* buf, const int bufSize) { return rtcpClient.recv(buf, bufSize); }
    int tcpSend(const char* buf, const int bufSize) { return rtcpClient.send(buf, bufSize); }

    virtual bool sendMsgOut(Msg& msg) = 0;
    virtual bool onReceivedData(void) = 0;
    virtual bool onConnect(void) = 0;   // tcp connection established
    virtual bool onDisconnect(void) = 0;    // tcp connection closed
    virtual bool isSignallingMsg(std::unique_ptr<Packet>& rppacket) = 0;
    virtual bool onOpenForData(void) = 0;  // open for data
    virtual bool onClosedForData(void) = 0; // closed for data
};

class HyperCubeClientCore : IHyperCubeClientCore
{
    private:

        class SignallingObject;

        class PacketQWithLock : std::deque<Packet::UniquePtr> {
            std::mutex qLock;
        public:
            void init(void);
            void deinit(void);
            void push(std::unique_ptr<Packet>& rpacket);
            bool pop(std::unique_ptr<Packet>& rpacket);
            bool isEmpty(void);
        };

        class RecvActivity : CstdThread, RecvPacketBuilder::IReadDataObject {
        private:
            IHyperCubeClientCore* pIHyperCubeClientCore = 0;
            RecvPacketBuilder recvPacketBuilder;
            std::mutex recvPacketBuilderLock;
            PacketQWithLock inPacketQ;
            std::unique_ptr<Packet> pinputPacket = 0;
            CstdConditional eventReadyToRead;
            virtual bool threadFunction(void);
            RecvPacketBuilder::READSTATUS readPackets(void);
            int readData(void* pdata, int dataLen);
        public:
            RecvActivity(IHyperCubeClientCore* pIHyperCubeClientCore, SignallingObject& _signallingObject);
            bool init(void);
            bool deinit(void);
            bool receiveIn(Packet::UniquePtr& rppacket);
            bool onConnect(void);
            bool onDisconnect(void);
        };

        class SendActivity : public CstdThread {
        private:
            IHyperCubeClientCore* pIHyperCubeClientCore = 0;
            WritePacketBuilder writePacketBuilder;
            std::mutex writePacketBuilderLock;
            PacketQWithLock outPacketQ;
            virtual bool threadFunction(void);
            bool writePacket(void);
            bool writePackets(void);

            CstdConditional eventPacketsAvailableToSend;
            int totalBytesSent = 0;
            int sendDataOut(const void* pdata, const int dataLen);

        public:
            SendActivity(IHyperCubeClientCore* _pIHyperCubeClientCore);
            ~SendActivity();
            bool init(void);
            bool deinit(void);
            bool sendOut(Packet::UniquePtr& rppacket);
            bool onConnect(void);
            bool onDisconnect(void);
        };

        class SignallingObject : CstdThread {
            MSerDes mserdes;
            GenTimer localPingAckTimer;
            int numLocalPingAcks = 0;
            CstdConditional eventDisconnectedFromServer;
            std::atomic<bool> connected = false;
            std::atomic<bool> justDisconnected = false;
            int warnedOfFailedConnectionAttempts = 0;
            int numFailedConnectionAttempts = 0;
            int numSuccessfullConnectionAttempts = 0;
            ConnectionInfo connectionInfo;
            GroupInfo defaultGroupInfo;

            class Server {
            public:
                std::string name;
                std::string address;
                unsigned int port = HYPERCUBE_SERVER_PORT;
                IHyperCubeClientCore* pIHyperCubeClientCore = 0;
                Server() {};
                bool init(IHyperCubeClientCore* _pIHyperCubeClientCore, std::string _serverName = "");
                bool updateDnsAddress(std::string _serverName = "");
                bool isValid(void) { return (address != ""); }
            };

            class Servers {
                enum {
                    WORKING = 0,
                    SECONDARY,
                    PRIMARY,
                };
                int index = WORKING;
                Server servers[3];
                Server& activeServer = servers[WORKING];
                IHyperCubeClientCore* pIHyperCubeClientCore = 0;
            public:
                Servers() {
                    servers[WORKING].name = HYPERCUBE_SERVER_NAME_SECONDARY;
                    servers[SECONDARY].name = HYPERCUBE_SERVER_NAME_SECONDARY;
                    servers[PRIMARY].name = HYPERCUBE_SERVER_NAME_PRIMARY;
                };
                bool init(IHyperCubeClientCore* _pIHyperCubeClientCore, std::string _serverName);
                Server& getActiveServer(void);
                Server& getActiveServerInfo(void) { return activeServer; }
                std::string getActiveServerAddress(void) { return activeServer.address; }
                Server& getNextActiveServer(void);
            };

            IHyperCubeClientCore* pIHyperCubeClientCore = 0;
            bool socketValid(void) { return pIHyperCubeClientCore->tcpSocketValid(); }
            bool connect(void);
            Servers servers;
          //  std::string serverIpAddressPrimary;
          //  std::string serverIpAddressSecondary;
          //  std::string serverIpAddressActive;
            bool connectIfNotConnected(void);
            bool processSigMsgJson(const Packet* ppacket);
            bool threadFunction(void);
            bool sendMsgOut(Msg& msg) {
                return pIHyperCubeClientCore->sendMsgOut(msg);
            }
            bool sendCmdOut(HYPERCUBECOMMANDS command, CommonInfoBase& commonInfoBase, bool ack = false ) {
                HyperCubeCommand hypeCubeCommand(command, commonInfoBase.to_json(), true);
                hypeCubeCommand.ack = ack;
                SigMsg signallingMsg(hypeCubeCommand.to_json().dump());
                return sendMsgOut(signallingMsg);
            }
            bool sendConnectionInfo(std::string _connectionName);
            bool createDefaultGroup(void);
            bool localPing(bool ack = false, std::string data = "localPingFromMatrix");
            bool remotePing(bool ack = false, std::string data = "remotePingFromMatrix");
            bool setupConnection(void);

            bool onCreateGroupAck(HyperCubeCommand& hyperCubeCommand);
            bool onConnectionInfoAck(HyperCubeCommand& hyperCubeCommand);
            bool onRemotePing(HyperCubeCommand& hyperCubeCommand);
            bool onLocalPing(HyperCubeCommand& hyperCubeCommand);
            bool onEchoData(HyperCubeCommand& hyperCubeCommand);
        protected:
        public:
            uint64_t connectionId;
            //uuid_t applicationInstanceUUID;

            SignallingObject(IHyperCubeClientCore* _pIHyperCubeClientCore);
            void init(std::string _serverName);
            void deinit(void);
            virtual bool isSignallingMsg(std::unique_ptr<Packet>& rppacket);
            virtual bool onConnect(void);
            virtual bool onDisconnect(void);
            virtual bool onOpenForData(void);
            virtual bool onClosedForData(void);
            void setConnectionInfo(const ConnectionInfo& rconnectionInfo) { connectionInfo = rconnectionInfo; }
            void setDefaultGroupInfo(const GroupInfo& rgroupInfo) { defaultGroupInfo = rgroupInfo; }

            bool publish(void);
            bool subscribe(std::string _groupName);
            bool createGroup(const GroupInfo& _rgroupName);
            bool createGroup(const std::string _groupName);
            bool echoData(std::string data = "");
            bool isConnected(void) { return connected; }
        };

        virtual bool onConnect(void);
        virtual bool onDisconnect(void);
        virtual bool onOpenForData(void);
        virtual bool onClosedForData(void);
        virtual bool isSignallingMsg(std::unique_ptr<Packet>& rppacket);

        Ctcp::Client client;
        MSerDes mserdes;
        RecvActivity receiveActivity;
        SendActivity sendActivity;

protected:
        SignallingObject signallingObject;

        static const int SERVER_PORT = HYPERCUBE_SERVER_PORT;

        double totalTime = 0;
        std::string dataString;

        int numOutputMsgs = 0;
        int numInputMsgs = 0;

protected:
        bool sendMsgOut(Msg& msg);
        virtual bool onReceivedData(void);
public:
        HyperCubeClientCore();
        ~HyperCubeClientCore();

        bool init(std::string serverName = HYPERCUBE_SERVER_NAME_PRIMARY, bool reInit = true);
        bool deinit(void);

        virtual bool connectionClosed(void) { return true; };

        bool getPacket(Packet& packet);

        SOCKET getSocket(void) { return client.getSocket(); }
        void setConnectionInfo(const ConnectionInfo& rconnectionInfo) { signallingObject.setConnectionInfo(rconnectionInfo); }
        void setDefaultGroupInfo(const GroupInfo& rgroupInfo) { signallingObject.setDefaultGroupInfo(rgroupInfo); }
};

class HyperCubeClient : public HyperCubeClientCore
{
protected:
public:
    HyperCubeClient();
    ~HyperCubeClient();
    bool subscribe(std::string _groupName) { return signallingObject.subscribe(_groupName);}
    bool publish(void) { return signallingObject.publish();}
    bool createGroup(const std::string _groupName) { return signallingObject.createGroup(_groupName);}
    bool sendMsg(Msg& msg) { return sendMsgOut(msg);}
    bool sendEcho(std::string data = "") { return signallingObject.echoData(data); }
    bool isConnected(void) { return signallingObject.isConnected(); }
};
