#pragma once

#include "Packet.h"
#include "MsgExt.h"

enum COMMANDS {
	CMD_NONE,
	PROC_SUSPEND,
	ADMIN_UPLOADSTATE,
	ADMIN_UPLOADSTATS,
	ADMIN_UPLOADLOGS,
	CMD_TOGGLEMATRXIGUI,
};

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
	template <typename T>
	bool getCommandFromJson(T& obj, const json& jsonData, const std::string& command);
	template <typename T, typename U>
	bool getObjFromSrcObjJson(T& obj, U& srcObj, const std::string& command);
public:
    bool onNewPacket(PacketEx& packetEx);
    virtual bool processCmdMsgJson(PacketEx& packetEx);
	std::unique_ptr<CommonInfoBase> processCmdMsgJson(MsgJson& rmsgJson);
};



