#include <gtest/gtest.h>
#include <memory>
#include "groupActivityData.h"

class Command {
public:
    bool operator<(const Command&) const {
        return false; // Implement comparison logic if needed
    }
};

class CommandInstanceInfo {};

using GroupName = std::string;

class GroupActivityDataTest : public ::testing::Test {
protected:
    GroupActivityData<GroupName, Command, CommandInstanceInfo> groupActivityData;
};

TEST_F(GroupActivityDataTest, AddCommandInstanceInfo) {
    auto pcommandInstanceInfo = std::make_shared<CommandInstanceInfo>();
    Command command;
    GroupName groupName = "testGroup";

    bool result = groupActivityData.groups.add(groupName, command, pcommandInstanceInfo);
    EXPECT_TRUE(result);
}

TEST_F(GroupActivityDataTest, RemoveCommandInstanceInfo) {
    auto pcommandInstanceInfo = std::make_shared<CommandInstanceInfo>();
    Command command;
    GroupName groupName = "testGroup";

    groupActivityData.groups.add(groupName, command, pcommandInstanceInfo);

    std::shared_ptr<CommandInstanceInfo> removedInstanceInfo;
    bool result = groupActivityData.groups.removeWait(groupName, command, removedInstanceInfo);
    EXPECT_TRUE(result);
    EXPECT_EQ(pcommandInstanceInfo, removedInstanceInfo);
}

TEST_F(GroupActivityDataTest, ContainsGroup) {
    auto pcommandInstanceInfo = std::make_shared<CommandInstanceInfo>();
    Command command;
    GroupName groupName = "testGroup";

    groupActivityData.groups.add(groupName, command, pcommandInstanceInfo);

    bool result = groupActivityData.groups.contains(groupName);
    EXPECT_TRUE(result);
}

TEST_F(GroupActivityDataTest, EraseGroup) {
    auto pcommandInstanceInfo = std::make_shared<CommandInstanceInfo>();
    Command command;
    GroupName groupName = "testGroup";

    groupActivityData.groups.add(groupName, command, pcommandInstanceInfo);

    bool result = groupActivityData.groups.erase(groupName);
    EXPECT_TRUE(result);

    result = groupActivityData.groups.contains(groupName);
    EXPECT_FALSE(result);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}