#pragma once

#include "Packet.h"
#include "MsgExt.h"

class Cmd {
	//static map <std::string, COMMANDS> g_mapStringToCommands;
	std::map <std::string, COMMANDS > g_mapStringToCommands;
	std::string codeString;
public:
	COMMANDS command;
	Cmd(std::string jsonString);
	~Cmd();

	void fromJsonString(std::string jsonString);
	std::string toJsonString(void);
};

class MsgDecoder
{
public:
    bool MsgDecoder::onNewPacket(PacketEx& packetEx);
    virtual bool MsgDecoder::onCmd(const MsgCmd& _msgCmd);
    virtual bool MsgDecoder::onMsgObj(const MsgObjectEx& _msgObjectEx);
    virtual bool MsgDecoder::onJsonCmd(const MsgCmd& _msgCmd);
};

enum COMMANDS {
	CMD_NONE,
	ADMIN_UPLOADSTATE,
	ADMIN_UPLOADSTATS,
	ADMIN_UPLOADLOGS,
};


