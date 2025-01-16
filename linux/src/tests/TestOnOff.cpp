#include "ClusterTestContext.h"
#include "command_translator.hpp"
#include "attribute_translator.hpp"


// Third party library
#include <gtest/gtest.h>
#include <app/tests/AppTestContext.h>
#include <pw_unit_test/framework.h>

using namespace unify::matter_bridge;
using namespace chip::app;
using TestContext = unify::matter_bridge::Test::ClusterContext<OnOffAttributeAccess, OnOffClusterCommandHandler>;

class TestOnOff : public TestContext {
public:

    void SetUp() {
        EXPECT_EQ(TestOnOff::Initialize(this),1);
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
    auto & cluster     = ep.emplace_cluster("OnOff");
    cluster.attributes = {
        "OnOff",
        "GlobalSceneControl",
        "OnTime",
        "OffWaitTime",
        "StartUpOnOff", // Unify SDK doesn't handle OnOffStartUpOnOff enum properly.
        "GeneratedCommandList",
        "AcceptedCommandList",
        "AttributeList",
        "FeatureMap",
        "ClusterRevision",
    };
    cluster.supported_commands = {
        "Off", "On", "Toggle", "OffWithEffect", "OnWithRecallGlobalScene", "OnWithTimedOff",
    };

    auto & identify_cluster = ep.emplace_cluster("Identify");
    identify_cluster.attributes.emplace("IdentifyTime");

    auto & scenes_cluster = ep.emplace_cluster("Scenes");
    scenes_cluster.attributes.emplace("SceneCount");

    auto & groups_cluster = ep.emplace_cluster("Groups");
    groups_cluster.attributes.emplace("NameSupport");

    return ctx->register_endpoint(ep);
}
};

TEST_F(TestOnOff, TestOnOffAttributeOnOff) {
    CHIP_ERROR err    = attribute_test<Clusters::OnOff::Attributes::OnOff::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/OnOff/Attributes/OnOff/Reported", R"({ "value": true })", true);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestOnOff, TestOnOffAttributeGlobalSceneControl) {
    CHIP_ERROR err    = attribute_test<Clusters::OnOff::Attributes::GlobalSceneControl::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/OnOff/Attributes/GlobalSceneControl/Reported", R"({ "value": true })", true);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestOnOff, TestOnOffAttributeOnTime)
{
    CHIP_ERROR err    = attribute_test<Clusters::OnOff::Attributes::OnTime::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/OnOff/Attributes/OnTime/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestOnOff, TestOnOffAttributeOffWaitTime)
{
    CHIP_ERROR err    = attribute_test<Clusters::OnOff::Attributes::OffWaitTime::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/OnOff/Attributes/OffWaitTime/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestOnOff, TestOnOffAttributeStartUpOnOff)
{
/*
TODO make the text context be able to handle nullable types
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;
    DataModel::Nullable<Clusters::OnOff::OnOffStartUpOnOff> value;
    value.SetNonNull(Clusters::OnOff::OnOffStartUpOnOff::kOff);
    err = attribute_test<Clusters::OnOff::Attributes::StartUpOnOff::TypeInfo, false>(
        "ucl/by-unid/zw-0x0002/ep2/OnOff/Attributes/StartUpOnOff/Reported", R"({ "value": "SetOnOffTo0" })", value);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    value.SetNonNull(Clusters::OnOff::OnOffStartUpOnOff::kOn);
    err = attribute_test<Clusters::OnOff::Attributes::StartUpOnOff::TypeInfo, false>(
        "ucl/by-unid/zw-0x0002/ep2/OnOff/Attributes/StartUpOnOff/Reported", R"({ "value": "SetOnOffTo1" })", value);
    EXPECT_EQ(err, CHIP_NO_ERROR);
 */
}

TEST_F(TestOnOff, TestOnOffCommandOff)
{
    Clusters::OnOff::Commands::Off::Type request;
    CHIP_ERROR err = command_test<Clusters::OnOff::Commands::Off::Type>("ucl/by-unid/zw-0x0002/ep2/OnOff/Commands/Off",
                                                                            "{}", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestOnOff, TestOnOffCommandOn)
{
    Clusters::OnOff::Commands::On::Type request;
    CHIP_ERROR err =
        command_test<Clusters::OnOff::Commands::On::Type>("ucl/by-unid/zw-0x0002/ep2/OnOff/Commands/On", "{}", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestOnOff, TestOnOffCommandToggle)
{
    Clusters::OnOff::Commands::Toggle::Type request;
    CHIP_ERROR err = command_test<Clusters::OnOff::Commands::Toggle::Type>(
        "ucl/by-unid/zw-0x0002/ep2/OnOff/Commands/Toggle", "{}", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestOnOff, TestOnOffCommandOffWithEffect)
{
    Clusters::OnOff::Commands::OffWithEffect::Type request;
    CHIP_ERROR err = command_test<Clusters::OnOff::Commands::OffWithEffect::Type>(
        "ucl/by-unid/zw-0x0002/ep2/OnOff/Commands/OffWithEffect",
        R"({"EffectIdentifier":"DelayedAllOff","EffectVariant":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestOnOff, TestOnOffCommandOnWithRecallGlobalScene)
{
    Clusters::OnOff::Commands::OnWithRecallGlobalScene::Type request;
    CHIP_ERROR err = command_test<Clusters::OnOff::Commands::OnWithRecallGlobalScene::Type>(
        "ucl/by-unid/zw-0x0002/ep2/OnOff/Commands/OnWithRecallGlobalScene", "{}", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestOnOff, TestOnOffCommandOnWithTimedOff)
{
    Clusters::OnOff::Commands::OnWithTimedOff::Type request;
    CHIP_ERROR err = command_test<Clusters::OnOff::Commands::OnWithTimedOff::Type>(
        "ucl/by-unid/zw-0x0002/ep2/OnOff/Commands/OnWithTimedOff",
        R"({"OffWaitTime":0,"OnOffControl":{"AcceptOnlyWhenOn":false},"OnTime":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}