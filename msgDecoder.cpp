#include <memory>
#include "msgDecoder.h"
#include "Logger.h"
#include "MsgExt.h"
#include "mserdes.h"
#include "hkClient.h"

using namespace std;

Cmd::Cmd(string jsonString) : command{CMD_NONE},
	g_mapStringToCommands {
		{"none", CMD_NONE },
		{"suspendProcesses", PROC_SUSPEND},
}
{
	fromJsonString(jsonString);
}

Cmd::~Cmd()
{
	codeString.clear();
	g_mapStringToCommands.clear();
}

void Cmd::fromJsonString(std::string jsonString)
{
	json jsonCmd = json::parse(jsonString);
	codeString = jsonCmd["code"].get<std::string>();
	command = g_mapStringToCommands[codeString];
}

string Cmd::toJsonString(void)
{
	json jsonCmd;
	jsonCmd["code"] = codeString;
	string jsonString = jsonCmd.dump();
	return jsonString;
}

// -----------------------------------------------------------------------
/*
bool MsgDecoder::onNewPacket(PacketEx& packetEx)
{
	bool stat = false;
   	MSerDes mserDes;
	Msg msg;
	mserDes.packetToMsg(&packetEx.packet, msg);

	switch (msg.subSys) {
		case SUBSYS_DISCOVERY:
			break;
		case SUBSYS_STATS:
			break;
		case SUBSYS_CMD:
			switch(msg.command) {
				case CMD_JSON:
					stat = processCmdMsgJson(packetEx);
					break;
				default:
				break;
			}
		case SUBSYS_OBJ:
			break;
		default:
			break;
	}

	return true;
}
*/
template <typename T>
bool MsgDecoder::getCommandFromJson(T& obj, const json& jsonData, const std::string& command) {
    try {
        obj.from_json(jsonData);
        return true;
    } catch (const std::exception& e) {
		std::string errorMsg = "MsgDecoder::getCommandFromJson():" + command;
		LOG_WARNING(errorMsg, "jsonData:" + jsonData.dump(), 0);
		LOG_WARNING(errorMsg, "Failed to decode json" + std::string(e.what()), 0);
        return false;
    }
}

template <typename T, typename U>
bool MsgDecoder::getObjFromSrcObjJson(T& obj, U& srcObj, const std::string& command) {
    try {
        obj.from_json(srcObj.getJsonData());
        return true;
    } catch (const std::exception& e) {
		std::string errorMsg = "MsgDecoder::getObjFromSrcObjJson():" + command;
		json jsonData = srcObj.getJsonData();
		LOG_WARNING(errorMsg, "jsonData:" + jsonData.dump(), 0);
		LOG_WARNING(errorMsg, "Failed to decode json" + std::string(e.what()), 0);
        return false;
    }
}

/*
bool MsgDecoder::processCmdMsgJson(PacketEx& packetEx)
{
    MSerDes mserDes;
	MsgJson msgJson;
	json jsonData;
	assert(mserDes.packetToMsgJson(&packetEx.packet, msgJson, jsonData));

	bool msgProcessed = false;
	std::string cmdString;

	HyperCubeCommand hyperCubeCommand(HYPERCUBECOMMANDS::NONE, NULL, true);
	HyperCubeCommand hyperCubeCommandAck(HYPERCUBECOMMANDS::NONE, NULL, true);;
	hyperCubeCommand.from_json(jsonData);

	try{
		getCommandFromJson(hyperCubeCommand, jsonData, "MsgDecoder::processSigMsgJson():command");
		switch(hyperCubeCommand.command) {
			case HYPERCUBECOMMANDS::PUBLISHINFO:
			{
				cmdString= "PUBLISHINFOACK";
				PublishInfoAck publishInfoAck;
				getObjFromSrcObjJson(publishInfoAck, hyperCubeCommand, cmdString);
			} break;
			default:
				break;
		}
	} catch (const std::exception& e) {
		LOG_WARNING("MsgDecoder::processCmdMsgJson()", "Failed to decode json" + std::string(e.what()), 0);
	}

	return true;
}
*/
bool MsgDecoder::processCmdMsgJson(MsgJson& rmsgJson, std::unique_ptr<CommonInfoBase> &pcommonInfoBase, HYPERCUBECOMMANDS &command)
{
	json jsonData =json::parse(rmsgJson.jsonData);

	bool msgProcessed = false;
	std::string cmdString;

	HyperCubeCommand hyperCubeCommand(HYPERCUBECOMMANDS::NONE, NULL, true);
	HyperCubeCommand hyperCubeCommandAck(HYPERCUBECOMMANDS::NONE, NULL, true);;
	hyperCubeCommand.from_json(jsonData);
	command = hyperCubeCommand.command;

	try{
		getCommandFromJson(hyperCubeCommand, jsonData, "MsgDecoder::processSigMsgJson():command");
		switch(command) {
			case HYPERCUBECOMMANDS::PUBLISHINFO:
			{
				cmdString= "PUBLISHINFO";
				pcommonInfoBase = std::make_unique<PublishInfo>();
				return getObjFromSrcObjJson(*pcommonInfoBase, hyperCubeCommand, cmdString);
			} break;
			case HYPERCUBECOMMANDS::PUBLISHINFOACK:
			{
				cmdString= "PUBLISHINFOACK";
				pcommonInfoBase = std::make_unique<PublishInfoAck>();
				return getObjFromSrcObjJson(*pcommonInfoBase, hyperCubeCommand, cmdString);
			} break;
			default:
				break;
		}
	} catch (const std::exception& e) {
		LOG_WARNING("MsgDecoder::processCmdMsgJson()", "Failed to decode json" + std::string(e.what()), 0);
	}

	return false;
}
