#include "ClusterTestContext.h"
#include "TestGroupCommandTranslatorHelper.h"
#include "group_command_translator.hpp"
#include "matter_device_translator.hpp"

// Third party library
#include <gtest/gtest.h>

// mocks
#include "MockGroupTranslator.hpp"
#include "MockNodeStateMonitor.hpp"
#include "MockUnifyMqtt.hpp"

//
#include <credentials/GroupDataProviderImpl.h>
#include <lib/support/TestGroupData.h>
#include <lib/support/TestPersistentStorageDelegate.h>

using namespace unify::matter_bridge;
using namespace chip::app;

#define TEST_LOG_TAG "TestGroupCommandTranslator"

constexpr uint16_t kMaxGroupsPerFabric    = 5;
constexpr uint16_t kMaxGroupKeysPerFabric = 8;

chip::TestPersistentStorageDelegate gTestStorage;
chip::Credentials::GroupDataProviderImpl gGroupsProvider(kMaxGroupsPerFabric, kMaxGroupKeysPerFabric);

using TestContext = unify::matter_bridge::Test::ClusterContext<GroupClusterAttributeTranslatorHelper, GroupClusterCommandHandler>;

class TestGroupCommand : public TestContext {
public:

    void SetUp() {
        EXPECT_EQ(TestGroupCommand::Initialize(this),1);
    }

    void TearDown() {
        EXPECT_EQ(TestContext::Finalize(this),1);
    }

static int Initialize(void * context)
{
    if (TestContext::Initialize(context) != SUCCESS)
        return FAILURE;

    auto * ctx         = static_cast<TestContext *>(context);
    auto & ep          = ctx->get_endpoint();
    auto & cluster     = ep.emplace_cluster("Groups");
    cluster.attributes = {
        "NameSupport",
    };
    cluster.supported_commands = {
        "AddGroup",
        "RemoveGroup",
        "RemoveAllGroups",
    };
    // Adding additional cluster support to make sure the test dynamic endpoint has device type
    auto & cluster_identify             = ep.emplace_cluster("Identify");
    cluster_identify.supported_commands = { "Identify" };
    cluster_identify.attributes         = { "IdentifyTime" };
    auto & cluster_scene                = ep.emplace_cluster("Scenes");
    cluster_scene.supported_commands    = { "AddScene" };
    cluster_scene.supported_commands    = { "SceneCount" };
    return ctx->register_endpoint(ep);
}
};

TEST_F(TestGroupCommand,TestAddGroupCommand)
{
    gTestStorage.ClearStorage();
    gGroupsProvider.SetStorageDelegate(&gTestStorage);
    gGroupsProvider.Init();
    chip::Credentials::SetGroupDataProvider(&gGroupsProvider);
    Clusters::Groups::Commands::AddGroup::Type request;
    request.groupID   = 1;
    request.groupName = chip::CharSpan::fromCharString("test_group_1");
    Clusters::Groups::Commands::AddGroupResponse::DecodableType response;
    CHIP_ERROR err = command_test<Clusters::Groups::Commands::AddGroup::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Groups/Commands/AddGroup", R"({"GroupId":1,"GroupName":"test_group_1"})", request,
        response);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

// TEST_F(TestGroupCommand, TestRemoveGroupCommand)
// {
//     Clusters::Groups::Commands::RemoveGroup::Type request_remove;
//     request_remove.groupID = 1;
//     Clusters::Groups::Commands::RemoveGroupResponse::DecodableType response_remove;
//     CHIP_ERROR err = command_test<Clusters::Groups::Commands::RemoveGroup::Type>(
//         "ucl/by-unid/zw-0x0002/ep2/Groups/Commands/RemoveGroup", R"({"GroupId":1})", request_remove, response_remove);
//     EXPECT_EQ(err, CHIP_NO_ERROR);
// }

TEST_F(TestGroupCommand, TestRemoveAllGroupsCommand)
{
    Clusters::Groups::Commands::AddGroup::Type request;
    request.groupID   = 2;
    request.groupName = chip::CharSpan::fromCharString("test_group_2");
    Clusters::Groups::Commands::AddGroupResponse::DecodableType response;
    CHIP_ERROR err = command_test<Clusters::Groups::Commands::AddGroup::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Groups/Commands/AddGroup", R"({"GroupId":1,"GroupName":"test_group_2"})", request,
        response);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    Clusters::Groups::Commands::RemoveAllGroups::Type request_remove;
    err = command_test<Clusters::Groups::Commands::RemoveAllGroups::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Groups/Commands/RemoveAllGroups", R"({})", request_remove);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}