#include "bashRemoteService.h"
#include "Logger.h"
#include <iostream>

using namespace std;


BashRemoteServer::BashRemoteServer() {
}

BashRemoteServer::~BashRemoteServer()
{

}

bool BashRemoteServer::start(void)
{
    LOG_INFO("startShellService()", "starting bash process", 0);


    if (!bashProcess.start()) {
        std::cerr << "Failed to start bash process\n";
        return false;
    }

    string uname = bashProcess.execute("uname -a");
    LOG_INFO("startShellService() uname: ", uname, 0);
    return true;
}

bool BashRemoteServer::stop(void) {
    bashProcess.stop();
    return true;
}

std::string BashRemoteServer::execute(const std::string &input, bool waitForResponse)
{
    LOG_DBG("BashRemoteServer::execute() input:", input, 0);
    if (!waitForResponse) {
        bashProcess.execute(input, 0);
        return "";
    }
    std::string response = bashProcess.execute(input);
    LOG_DBG("BashRemoteServer::execute() response:", response, 0);
    return response;
}

std::string BashRemoteServer::prompt()
{
    return bashProcess.prompt();
}


//--------------------------------------------------------------------------------------------

BashRemoteService::BashRemoteService(HKIAPI& hkApi) : hkAPI(hkApi),
    HKIServerService(hkApi)
{
}

BashRemoteService::~BashRemoteService()
{

}

std::string BashRemoteService::execute(const std::string &input, bool waitForResponse)
{
    return bashRemoteServer.execute(input, waitForResponse);
}

bool BashRemoteService::onPublishInfo(PublishInfo& publishInfo)
{
    std::string command = publishInfo.publishData;
    std::string response = "@Shell";
    bool waitForResponse = publishInfo.ack;

    if (command == "@bash") {
        //startShellService();
        response = bashRemoteServer.prompt();
        hkAPI.publishAck(publishInfo, response);
        return true;
    }

    if (command != "") {
        string response = bashRemoteServer.execute(command, waitForResponse);
        if (waitForResponse) {
            hkAPI.publishAck(publishInfo, response);
            LOG_DBG("HKShell::onPublishInfo() ack: ", response.substr(0, 60), 0);
        }
    }

    return true;
}