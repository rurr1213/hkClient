#include <gtest/gtest.h>
#include <memory>
#include "groupActivityData.h"

class Command {
public:
    bool operator<(const Command& other) const {
        // Implement comparison logic if needed
        return false;
    }
};

class CommandInstanceInfo {
public:
    std::string uuid;
};

using GroupName = std::string;

class GroupActivityDataTest : public ::testing::Test {
protected:
    GroupActivityData<GroupName, Command, CommandInstanceInfo> groupActivityData;
};

TEST_F(GroupActivityDataTest, AddCommandInstanceInfo) {
    auto pcommandInstanceInfo = std::make_shared<CommandInstanceInfo>();
    pcommandInstanceInfo->uuid = "uuid1";
    Command command;
    GroupName groupName = "testGroup";

    bool result = groupActivityData.add(groupName, command, pcommandInstanceInfo);
    EXPECT_TRUE(result);
}

TEST_F(GroupActivityDataTest, RemoveCommandInstanceInfo) {
    auto pcommandInstanceInfo = std::make_shared<CommandInstanceInfo>();
    pcommandInstanceInfo->uuid = "uuid1";
    Command command;
    GroupName groupName = "testGroup";

    groupActivityData.add(groupName, command, pcommandInstanceInfo);

    std::shared_ptr<CommandInstanceInfo> removedInstanceInfo;
    bool result = groupActivityData.removeWait(groupName, command, removedInstanceInfo);
    EXPECT_TRUE(result);
    EXPECT_EQ(pcommandInstanceInfo, removedInstanceInfo);
}

TEST_F(GroupActivityDataTest, FindCommandInstanceInfo) {
    auto pcommandInstanceInfo = std::make_shared<CommandInstanceInfo>();
    pcommandInstanceInfo->uuid = "uuid1";
    Command command;
    GroupName groupName = "testGroup";

    groupActivityData.add(groupName, command, pcommandInstanceInfo);

    std::shared_ptr<CommandInstanceInfo> foundInstanceInfo;
    bool result = groupActivityData.findWait(groupName, command, "uuid1", foundInstanceInfo);
    EXPECT_TRUE(result);
    EXPECT_EQ(pcommandInstanceInfo, foundInstanceInfo);
}

TEST_F(GroupActivityDataTest, ContainsGroup) {
    auto pcommandInstanceInfo = std::make_shared<CommandInstanceInfo>();
    pcommandInstanceInfo->uuid = "uuid1";
    Command command;
    GroupName groupName = "testGroup";

    groupActivityData.add(groupName, command, pcommandInstanceInfo);

    bool result = groupActivityData.contains(groupName);
    EXPECT_TRUE(result);
}

TEST_F(GroupActivityDataTest, EraseGroup) {
    auto pcommandInstanceInfo = std::make_shared<CommandInstanceInfo>();
    pcommandInstanceInfo->uuid = "uuid1";
    Command command;
    GroupName groupName = "testGroup";

    groupActivityData.add(groupName, command, pcommandInstanceInfo);

    bool result = groupActivityData.erase(groupName);
    EXPECT_TRUE(result);

    result = groupActivityData.contains(groupName);
    EXPECT_FALSE(result);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}