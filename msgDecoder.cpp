#include <memory>
#include "msgDecoder.h"
#include "Logger.h"
#include "MessagesCommon.h"
#include "MsgExt.h"

using namespace std;

Cmd::Cmd(string jsonString) : command{CMD_NONE},
	g_mapStringToCommands {
		{"none", CMD_NONE }
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

bool MsgDecoder::onNewPacket(PacketEx& packetEx)
{
	unique_ptr<Msg> pmsg = MsgExt::factoryMethod(packetEx);

	switch (pmsg->subSys) {
		case SUBSYS_DISCOVERY:
			break;
		case SUBSYS_STATS:
			break;
		case SUBSYS_CMD:
			{
				const MsgCmd* pmsgCmd = (MsgCmd*)pmsg.get();
				onCmd(*pmsgCmd);
			}
			break;
		case SUBSYS_OBJ:
			{
				const MsgObjectEx* pmsgOE = (MsgObjectEx*)pmsg.get();
				onMsgObj(*pmsgOE);
			}
			break;
		default:
			break;
	}

	return true;
}

bool MsgDecoder::onCmd(const MsgCmd& _msgCmd)
{
	switch (_msgCmd.command) {
	case CMD_PCJSON:
		onJsonCmd(_msgCmd);
		break;
	default:
		break;
	}
	return true;
}

bool MsgDecoder::onMsgObj(const MsgObjectEx& _msgObjectEx)
{
	switch (_msgObjectEx.command) {
	case OBJ_MATRIXMGR:
		break;
	case OBJ_LOGGER:
		break;
	case OBJ_APPMGR:
		break;
	case OBJ_STATSMGR:
		break;
	default:
		LOG_WARNING("MsgDecoder::onMsgObj()", "unknown command", _msgObjectEx.command);
		break;
	}
	return true;
}

bool MsgDecoder::onJsonCmd(const MsgCmd& _msgCmd)
{
	Cmd cmd(_msgCmd.jsonData);
	switch (cmd.command) {
	default:
		break;
	}
	return true;
}
