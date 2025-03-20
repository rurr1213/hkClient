#pragma once
#include "hkAPI.h"
#include "hkIServerServiceBase.h"
#include <string>

class HKDirService : public HKIServerServiceBase
{
    std::string getDirNodes(std::string path);
    std::string currentDir = "~/";
public:

    const std::string GROUPNAME = "hkFileServiceGroup";

    HKDirService(HKIAPI& api);
    ~HKDirService();

    bool onPublishInfo(PublishInfo& publishInfo) override;
};