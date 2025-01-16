#include "ClusterTestContext.h"
#include "attribute_translator.hpp"
#include "command_translator.hpp"

// Third party library
#include <gtest/gtest.h>
#include <pw_unit_test/framework.h>
#include <app/tests/test-interaction-model-api.h>

using namespace unify::matter_bridge;
using namespace chip::app;
using TestContext = unify::matter_bridge::Test::ClusterContext<LevelControlAttributeAccess, LevelControlClusterCommandHandler>;

class TestLevelControl : public TestContext
{
public:
void SetUp() {
    EXPECT_EQ(TestLevelControl::Initialize(this),1);
}

void TearDown() {
    EXPECT_EQ(TestContext::Finalize(this),1);
}

int Initialize(void * context) {
        if (TestContext::Initialize(context) != SUCCESS)
                return FAILURE;
                
        auto * ctx         = static_cast<TestContext *>(context);
        auto & ep          = ctx->get_endpoint();
        auto & cluster     = ep.emplace_cluster("Level");
        cluster.attributes = {
            "CurrentLevel",
            "RemainingTime",
            "MinLevel",
            "MaxLevel",
            "CurrentFrequency",
            "MinFrequency",
            "MaxFrequency",
            "Options",
            "OnOffTransitionTime",
            "OnLevel",
            "OnTransitionTime",
            "OffTransitionTime",
            "DefaultMoveRate",
            "StartUpCurrentLevel",
            "GeneratedCommandList",
            "AcceptedCommandList",
            "AttributeList",
            "FeatureMap",
            "ClusterRevision",
        };
        cluster.supported_commands = {
            "MoveToLevel",           "Move", "Step", "Stop", "MoveToLevelWithOnOff", "MoveWithOnOff", "StepWithOnOff", "StopWithOnOff",
            "MoveToClosestFrequency" // UNSUPPORTED_COMMAND
        };

        return ctx->register_endpoint(ep);
}

};


TEST_F(TestLevelControl, TestLevelControlAttributeWrite) {
    using namespace chip::app::Clusters::LevelControl;
    CHIP_ERROR err = attribute_write_test<Attributes::OnLevel::TypeInfo>("Level/Commands/WriteAttributes", R"({ "OnLevel": 3 })",
                                                            Attributes::OnLevel::TypeInfo::Type(3));
    EXPECT_EQ( err,CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlAttributeRemainingTime) { 
    CHIP_ERROR err    = attribute_test<Clusters::LevelControl::Attributes::RemainingTime::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/RemainingTime/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ( err,CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlAttributeMinLevel)
{
    CHIP_ERROR err;
    err = attribute_test<Clusters::LevelControl::Attributes::MinLevel::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/MinLevel/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ( err,CHIP_NO_ERROR);

    // Check that the emualtor clamps the level
    err = attribute_test<Clusters::LevelControl::Attributes::MinLevel::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/MinLevel/Reported", R"({ "value": 0 })", 1);
    EXPECT_EQ( err,CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlAttributeMaxLevel)
{
    CHIP_ERROR err    = attribute_test<Clusters::LevelControl::Attributes::MaxLevel::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/MaxLevel/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ( err,CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlAttributeCurrentFrequency)
{ 
    CHIP_ERROR err    = attribute_test<Clusters::LevelControl::Attributes::CurrentFrequency::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/CurrentFrequency/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err,CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlAttributeMinFrequency)
{
    CHIP_ERROR err    = attribute_test<Clusters::LevelControl::Attributes::MinFrequency::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/MinFrequency/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlAttributeMaxFrequency)
{
    CHIP_ERROR err    = attribute_test<Clusters::LevelControl::Attributes::MaxFrequency::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/MaxFrequency/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlAttributeOptions)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;

    // Bitmask
    err = attribute_test<Clusters::LevelControl::Attributes::Options::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/Options/Reported",
        R"({ "value": {"ExecuteIfOff":false,"CoupleColorTempToLevel":false } })", false);
    EXPECT_EQ( err , CHIP_NO_ERROR);

    err = attribute_test<Clusters::LevelControl::Attributes::Options::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/Options/Reported",
        R"({ "value": {"ExecuteIfOff":true,"CoupleColorTempToLevel":true } })", 3);
    EXPECT_EQ( err , CHIP_NO_ERROR);

    // it is also ok to skip parameters
    err = attribute_test<Clusters::LevelControl::Attributes::Options::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/Options/Reported", R"({ "value": { } })", false);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlAttributeOnOffTransitionTime)
{
    CHIP_ERROR err    = attribute_test<Clusters::LevelControl::Attributes::OnOffTransitionTime::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/OnOffTransitionTime/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlAttributeOnLevel)
{
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = attribute_test<Clusters::LevelControl::Attributes::OnLevel::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/OnLevel/Reported", R"({ "value": 42 })", value);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlAttributeOnTransitionTime)
{
    DataModel::Nullable<uint16_t> value;
    value.SetNonNull((uint16_t) 42);
    CHIP_ERROR err = attribute_test<Clusters::LevelControl::Attributes::OnTransitionTime::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/OnTransitionTime/Reported", R"({ "value": 42 })", value);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlAttributeOffTransitionTime)
{
    DataModel::Nullable<uint16_t> value;
    value.SetNonNull((uint16_t) 42);
    CHIP_ERROR err = attribute_test<Clusters::LevelControl::Attributes::OffTransitionTime::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/OffTransitionTime/Reported", R"({ "value": 42 })", value);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlAttributeDefaultMoveRate)
{
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = attribute_test<Clusters::LevelControl::Attributes::DefaultMoveRate::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/DefaultMoveRate/Reported", R"({ "value": 42 })", value);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlAttributeStartUpCurrentLevel)
{
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = attribute_test<Clusters::LevelControl::Attributes::StartUpCurrentLevel::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/StartUpCurrentLevel/Reported", R"({ "value": 42 })", value);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlAttributeStartUpCurrentLevelForMinimumValue)
{
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 0);
    CHIP_ERROR err = attribute_test<Clusters::LevelControl::Attributes::StartUpCurrentLevel::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/StartUpCurrentLevel/Reported", R"({ "value": "MinimumDeviceValuePermitted" })", value);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

// TEST_F(TestLevelControl, TestLevelControlAttributeStartUpCurrentLevelForPreviousValue)
// {
//     DataModel::Nullable<uint8_t> value;
//     value.SetNull();
//     CHIP_ERROR err = attribute_test<Clusters::LevelControl::Attributes::StartUpCurrentLevel::TypeInfo>(
//         "ucl/by-unid/zw-0x0002/ep2/Level/Attributes/StartUpCurrentLevel/Reported", R"({ "value": "SetToPreviousValue" })", value);
//     EXPECT_EQ( err , CHIP_NO_ERROR);
// }

TEST_F(TestLevelControl, TestLevelControlCommandMoveToLevel)
{
    Clusters::LevelControl::Commands::MoveToLevel::Type request;
    CHIP_ERROR err = command_test<Clusters::LevelControl::Commands::MoveToLevel::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Commands/MoveToLevel",
        R"({"Level":0,"OptionsMask":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"OptionsOverride":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"TransitionTime":null})",
        request);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlCommandMove)
{
    Clusters::LevelControl::Commands::Move::Type request;
    CHIP_ERROR err = command_test<Clusters::LevelControl::Commands::Move::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Commands/Move",
        R"({"MoveMode":"Up","OptionsMask":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"OptionsOverride":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"Rate":null})",
        request);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlCommandStep)
{
    Clusters::LevelControl::Commands::Step::Type request;
    CHIP_ERROR err = command_test<Clusters::LevelControl::Commands::Step::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Commands/Step",
        R"({"OptionsMask":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"OptionsOverride":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"StepMode":"Up","StepSize":0,"TransitionTime":null})",
        request);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlCommandStop)
{
    Clusters::LevelControl::Commands::Stop::Type request;
    CHIP_ERROR err = command_test<Clusters::LevelControl::Commands::Stop::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Commands/Stop",
        R"({"OptionsMask":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"OptionsOverride":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false}})",
        request);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlCommandMoveToLevelWithOnOff)
{
    Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Type request;
    CHIP_ERROR err = command_test<Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Commands/MoveToLevelWithOnOff",
        R"({"Level":0,"OptionsMask":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"OptionsOverride":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"TransitionTime":null})",
        request);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlCommandMoveWithOnOff)
{
    Clusters::LevelControl::Commands::MoveWithOnOff::Type request;
    CHIP_ERROR err = command_test<Clusters::LevelControl::Commands::MoveWithOnOff::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Commands/MoveWithOnOff",
        R"({"MoveMode":"Up","OptionsMask":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"OptionsOverride":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"Rate":null})",
        request);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlCommandStepWithOnOff)
{
    Clusters::LevelControl::Commands::StepWithOnOff::Type request;
    CHIP_ERROR err = command_test<Clusters::LevelControl::Commands::StepWithOnOff::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Level/Commands/StepWithOnOff",
        R"({"OptionsMask":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"OptionsOverride":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"StepMode":"Up","StepSize":0,"TransitionTime":null})",
        request);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlCommandStopWithOnOff)
{
    Clusters::LevelControl::Commands::StopWithOnOff::Type request;
    CHIP_ERROR err = command_test<Clusters::LevelControl::Commands::StopWithOnOff::Type>("ucl/by-unid/zw-0x0002/ep2/Level/Commands/StopWithOnOff",
        R"({"OptionsMask":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false},"OptionsOverride":{"CoupleColorTempToLevel":false,"ExecuteIfOff":false}})",
        request);
    EXPECT_EQ( err , CHIP_NO_ERROR);
}

TEST_F(TestLevelControl, TestLevelControlCommandMoveToClosestFrequency)
{
   chip::app::Clusters::LevelControl::Commands::MoveToClosestFrequency::Type request;
   request.frequency = 142;
   CHIP_ERROR err = command_test("ucl/by-unid/zw-0x0002/ep2/Level/Commands/MoveToClosestFrequency", R"({"Frequency": 142})", request);
   EXPECT_EQ( err , CHIP_NO_ERROR);
}
