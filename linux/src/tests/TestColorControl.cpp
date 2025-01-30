#include "ClusterTestContext.h"
#include "command_translator.hpp"
#include "attribute_translator.hpp"


// Third party library
#include <gtest/gtest.h>

using namespace unify::matter_bridge;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::ColorControl;
using TestContext = unify::matter_bridge::Test::ClusterContext<ColorControlAttributeAccess, ColorControlClusterCommandHandler>;

class TestColorControl : public TestContext {
public:
void SetUp() {
    EXPECT_EQ(TestColorControl::Initialize(this),1);
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
    auto & cluster     = ep.emplace_cluster("ColorControl");
    cluster.attributes = {
        "CurrentHue",
        "CurrentSaturation",
        "RemainingTime",
        "CurrentX",
        "CurrentY",
        "DriftCompensation",
        "CompensationText", // Unify SDK doesn't support it.
        "ColorTemperatureMireds",
        "ColorMode", // Matter doesn't support enum.
        "Options",   // Unify SDK doesn't support it.
        "NumberOfPrimaries",
        "Primary1X",
        "Primary1Y",
        "Primary1Intensity",
        "Primary2X",
        "Primary2Y",
        "Primary2Intensity",
        "Primary3X",
        "Primary3Y",
        "Primary3Intensity",
        "Primary4X",
        "Primary4Y",
        "Primary4Intensity",
        "Primary5X",
        "Primary5Y",
        "Primary5Intensity",
        "Primary6X",
        "Primary6Y",
        "Primary6Intensity",
        "WhitePointX",
        "WhitePointY",
        "ColorPointRX",
        "ColorPointRY",
        "ColorPointRIntensity",
        "ColorPointGX",
        "ColorPointGY",
        "ColorPointGIntensity",
        "ColorPointBX",
        "ColorPointBY",
        "ColorPointBIntensity",
        "EnhancedCurrentHue",
        "EnhancedColorMode", // Matter doesn't support enum.
        "ColorLoopActive",
        "ColorLoopDirection",
        "ColorLoopTime",
        "ColorLoopStartEnhancedHue",
        "ColorLoopStoredEnhancedHue",
        "ColorCapabilities", // Matter doesn't support bitmask.
        "ColorTempPhysicalMinMireds",
        "ColorTempPhysicalMaxMireds",
        "CoupleColorTempToLevelMinMireds",
        "StartUpColorTemperatureMireds",
        "GeneratedCommandList",
        "AcceptedCommandList",
        "AttributeList",
        "FeatureMap",
        "ClusterRevision",
    };

    cluster.supported_commands = {
        // A checklist of UnifySDK handler implementations for ColorControl.
        // For more in detail, see;
        // unify_dotdot_attribute_store_command_callbacks_color_control.c:color_control_cluster_mapper_init
        /* - [X] */ "MoveToHue",
        /* - [X] */ "MoveHue",
        /* - [X] */ "StepHue",
        /* - [X] */ "MoveToSaturation",
        /* - [ ] */ "MoveSaturation", // Handling MoveMode unimplemented.
        /* - [X] */ "StepSaturation",
        /* - [X] */ "MoveToHueAndSaturation",
        /* - [ ] */ "MoveToColor",
        /* - [ ] */ "MoveColor",
        /* - [ ] */ "StepColor",
        /* - [X] */ "MoveToColorTemperature",
        /* - [ ] */ "EnhancedMoveToHue",
        /* - [ ] */ "EnhancedMoveHue",
        /* - [ ] */ "EnhancedStepHue",
        /* - [ ] */ "EnhancedMoveToHueAndSaturation",
        /* - [ ] */ "ColorLoopSet",
        /* - [X] */ "StopMoveStep",
        /* - [X] */ "MoveColorTemperature",
        /* - [X] */ "StepColorTemperature",
    };

    return ctx->register_endpoint(ep);
}
};

TEST_F(TestColorControl, TestColorControlAttributeCurrentHue)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::CurrentHue::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/CurrentHue/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err,CHIP_NO_ERROR);
}

TEST_F(TestColorControl, TestColorControlAttributeCurrentSaturation)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::CurrentSaturation::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/CurrentSaturation/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err,CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeRemainingTime)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::RemainingTime::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/RemainingTime/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeCurrentX)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::CurrentX::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/CurrentX/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeCurrentY)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::CurrentY::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/CurrentY/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

// TEST_F(TestColorControl, AttributeDriftCompensation)
// {
//     CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::DriftCompensation::TypeInfo>(
//         "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/DriftCompensation/Reported", R"({ "value": "Temperaturemonitoring" })", 
//         DriftCompensationEnum::kTemperatureMonitoring );
//     EXPECT_EQ(err, CHIP_NO_ERROR);
// }

// TEST_F(TestColorControl, AttributeCompensationText)
// {
//     auto result       = attribute_test<Clusters::ColorControl::Attributes::CompensationText::TypeInfo>(
//         "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/CompensationText/Reported", R"({ "value": "SomeValue" })");
//     EXPECT_TRUE(result.unwrap().empty());
// }

TEST_F(TestColorControl, AttributeColorTemperatureMireds)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::ColorTemperatureMireds::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorTemperatureMireds/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

// TEST_F(TestColorControl, AttributeColorMode)
// {
//     CHIP_ERROR err    =  attribute_test<Clusters::ColorControl::Attributes::ColorMode::TypeInfo, false>(
//         "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorMode/Reported", R"({ "value": 42 })", 42);
//     EXPECT_EQ(err, CHIP_NO_ERROR);
// }

// TEST_F(TestColorControl, AttributeOptions)
// {
//     TestContext & ctx = *static_cast<TestContext *>(apContext);
//     CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::Options::TypeInfo, false>(
//         "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Options/Reported", R"({ "value": 42 })", 42);
//     EXPECT_EQ(err, CHIP_ERROR(0x00000586));
// }

TEST_F(TestColorControl, AttributeNumberOfPrimaries)
{
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = attribute_test<Clusters::ColorControl::Attributes::NumberOfPrimaries::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/NumberOfPrimaries/Reported", R"({ "value": 42 })", value);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary1X)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::Primary1X::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary1X/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary1Y)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::Primary1Y::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary1Y/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary1Intensity)
{
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = attribute_test<Clusters::ColorControl::Attributes::Primary1Intensity::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary1Intensity/Reported", R"({ "value": 42 })", value);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary2X)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::Primary2X::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary2X/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary2Y)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::Primary2Y::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary2Y/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary2Intensity)
{
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = attribute_test<Clusters::ColorControl::Attributes::Primary2Intensity::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary2Intensity/Reported", R"({ "value": 42 })", value);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary3X)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::Primary3X::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary3X/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary3Y)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::Primary3Y::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary3Y/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary3Intensity)
{
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = attribute_test<Clusters::ColorControl::Attributes::Primary3Intensity::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary3Intensity/Reported", R"({ "value": 42 })", value);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary4X)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::Primary4X::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary4X/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary4Y)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::Primary4Y::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary4Y/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary4Intensity)
{
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = attribute_test<Clusters::ColorControl::Attributes::Primary4Intensity::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary4Intensity/Reported", R"({ "value": 42 })", value);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary5X)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::Primary5X::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary5X/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary5Y)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::Primary5Y::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary5Y/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary5Intensity)
{
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = attribute_test<Clusters::ColorControl::Attributes::Primary5Intensity::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary5Intensity/Reported", R"({ "value": 42 })", value);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary6X)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::Primary6X::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary6X/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary6Y)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::Primary6Y::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary6Y/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributePrimary6Intensity)
{
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = attribute_test<Clusters::ColorControl::Attributes::Primary6Intensity::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/Primary6Intensity/Reported", R"({ "value": 42 })", value);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeWhitePointX)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::WhitePointX::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/WhitePointX/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeWhitePointY)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::WhitePointY::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/WhitePointY/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorPointRX)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::ColorPointRX::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointRX/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorPointRY)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::ColorPointRY::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointRY/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorPointRIntensity)
{
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = attribute_test<Clusters::ColorControl::Attributes::ColorPointRIntensity::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointRIntensity/Reported", R"({ "value": 42 })", value);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorPointGX)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::ColorPointGX::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointGX/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorPointGY)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::ColorPointGY::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointGY/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorPointGIntensity)
{
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = attribute_test<Clusters::ColorControl::Attributes::ColorPointGIntensity::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointGIntensity/Reported", R"({ "value": 42 })", value);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorPointBX)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::ColorPointBX::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointBX/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorPointBY)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::ColorPointBY::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointBY/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorPointBIntensity)
{
    DataModel::Nullable<uint8_t> value;
    value.SetNonNull((uint8_t) 42);
    CHIP_ERROR err = attribute_test<Clusters::ColorControl::Attributes::ColorPointBIntensity::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorPointBIntensity/Reported", R"({ "value": 42 })", value);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeEnhancedCurrentHue)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::EnhancedCurrentHue::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/EnhancedCurrentHue/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeEnhancedColorMode)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::EnhancedColorMode::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/EnhancedColorMode/Reported", R"({ "value": "ColorTemperatureMireds" })",
        ColorControl::EnhancedColorModeEnum::kColorTemperatureMireds);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorLoopActive)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::ColorLoopActive::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorLoopActive/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorLoopDirection)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::ColorLoopDirection::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorLoopDirection/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorLoopTime)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::ColorLoopTime::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorLoopTime/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorLoopStartEnhancedHue)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::ColorLoopStartEnhancedHue::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorLoopStartEnhancedHue/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorLoopStoredEnhancedHue)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::ColorLoopStoredEnhancedHue::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorLoopStoredEnhancedHue/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorCapabilities)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::ColorCapabilities::TypeInfo, false>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorCapabilities/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorTempPhysicalMinMireds)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::ColorTempPhysicalMinMireds::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorTempPhysicalMinMireds/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeColorTempPhysicalMaxMireds)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::ColorTempPhysicalMaxMireds::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/ColorTempPhysicalMaxMireds/Reported", R"({ "value": 42 })", 42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeCoupleColorTempToLevelMinMireds)
{
    CHIP_ERROR err    = attribute_test<Clusters::ColorControl::Attributes::CoupleColorTempToLevelMinMireds::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/CoupleColorTempToLevelMinMireds/Reported", R"({ "value": 42 })",
        42);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeStartUpColorTemperatureMireds)
{
    DataModel::Nullable<uint16_t> value;
    value.SetNonNull((uint16_t) 42);
    CHIP_ERROR err = attribute_test<Clusters::ColorControl::Attributes::StartUpColorTemperatureMireds::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Attributes/StartUpColorTemperatureMireds/Reported", R"({ "value": 42 })",
        value);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, AttributeGeneratedCommandList) {}
TEST_F(TestColorControl, AttributeAcceptedCommandList) {}
TEST_F(TestColorControl, AttributeAttributeList) {}
TEST_F(TestColorControl, AttributeFeatureMap) {}
TEST_F(TestColorControl, AttributeClusterRevision) {}

TEST_F(TestColorControl, CommandMoveToHue)
{
    Clusters::ColorControl::Commands::MoveToHue::Type request;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::MoveToHue::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveToHue",
        R"({"Direction":"ShortestDistance","Hue":0,"OptionsMask":0,"OptionsOverride":0,"TransitionTime":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandMoveHue)
{
    Clusters::ColorControl::Commands::MoveHue::Type request;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::MoveHue::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveHue",
        R"({"MoveMode":"Stop","OptionsMask":0,"OptionsOverride":0,"Rate":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandStepHue)
{
    Clusters::ColorControl::Commands::StepHue::Type request;
    request.stepMode = chip::app::Clusters::ColorControl::HueStepMode::kUp;
    request.stepSize = 1;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::StepHue::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/StepHue",
        R"({"OptionsMask":0,"OptionsOverride":0,"StepMode":"Up","StepSize":1,"TransitionTime":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    Clusters::ColorControl::Commands::StopMoveStep::Type request2;
    err = command_test<Clusters::ColorControl::Commands::StopMoveStep::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/StopMoveStep", R"({"OptionsMask":0,"OptionsOverride":0})",
        request2);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandMoveToSaturation)
{
    Clusters::ColorControl::Commands::MoveToSaturation::Type request;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::MoveToSaturation::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveToSaturation",
        R"({"OptionsMask":0,"OptionsOverride":0,"Saturation":0,"TransitionTime":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandMoveSaturation)
{
    Clusters::ColorControl::Commands::MoveSaturation::Type request;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::MoveSaturation::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveSaturation",
        R"({"MoveMode":"Stop","OptionsMask":0,"OptionsOverride":0,"Rate":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandStepSaturation)
{
    Clusters::ColorControl::Commands::StepSaturation::Type request;
    request.stepMode = chip::app::Clusters::ColorControl::SaturationStepMode::kUp;
    request.stepSize = 1;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::StepSaturation::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/StepSaturation",
        R"({"OptionsMask":0,"OptionsOverride":0,"StepMode":"Up","StepSize":1,"TransitionTime":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    Clusters::ColorControl::Commands::StopMoveStep::Type request2;
    err = command_test<Clusters::ColorControl::Commands::StopMoveStep::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/StopMoveStep", R"({"OptionsMask":0,"OptionsOverride":0})",
        request2);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandMoveToHueAndSaturation)
{
    Clusters::ColorControl::Commands::MoveToHueAndSaturation::Type request;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::MoveToHueAndSaturation::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveToHueAndSaturation",
        R"({"Hue":0,"OptionsMask":0,"OptionsOverride":0,"Saturation":0,"TransitionTime":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandMoveToColor)
{
    Clusters::ColorControl::Commands::MoveToColor::Type request;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::MoveToColor::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveToColor",
        R"({"ColorX":0,"ColorY":0,"OptionsMask":0,"OptionsOverride":0,"TransitionTime":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandMoveColor)
{
    Clusters::ColorControl::Commands::MoveColor::Type request;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::MoveColor::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveColor",
        R"({"OptionsMask":0,"OptionsOverride":0,"RateX":0,"RateY":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandStepColor)
{
    Clusters::ColorControl::Commands::StepColor::Type request;
    request.stepX = 1;
    request.stepY = 1;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::StepColor::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/StepColor",
        R"({"OptionsMask":0,"OptionsOverride":0,"StepX":1,"StepY":1,"TransitionTime":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    Clusters::ColorControl::Commands::StopMoveStep::Type request2;
    err = command_test<Clusters::ColorControl::Commands::StopMoveStep::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/StopMoveStep", R"({"OptionsMask":0,"OptionsOverride":0})",
        request2);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandMoveToColorTemperature)
{
    Clusters::ColorControl::Commands::MoveToColorTemperature::Type request;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::MoveToColorTemperature::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveToColorTemperature",
        R"({"ColorTemperatureMireds":0,"OptionsMask":0,"OptionsOverride":0,"TransitionTime":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandEnhancedMoveToHue)
{
    Clusters::ColorControl::Commands::EnhancedMoveToHue::Type request;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::EnhancedMoveToHue::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/EnhancedMoveToHue",
        R"({"Direction":"ShortestDistance","EnhancedHue":0,"OptionsMask":0,"OptionsOverride":0,"TransitionTime":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandEnhancedMoveHue)
{
    Clusters::ColorControl::Commands::EnhancedMoveHue::Type request;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::EnhancedMoveHue::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/EnhancedMoveHue",
        R"({"MoveMode":"Stop","OptionsMask":0,"OptionsOverride":0,"Rate":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandEnhancedStepHue)
{
    Clusters::ColorControl::Commands::EnhancedStepHue::Type request;
    request.stepMode = chip::app::Clusters::ColorControl::HueStepMode::kUp;
    request.stepSize = 1;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::EnhancedStepHue::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/EnhancedStepHue",
        R"({"OptionsMask":0,"OptionsOverride":0,"StepMode":"Up","StepSize":1,"TransitionTime":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    Clusters::ColorControl::Commands::StopMoveStep::Type request2;
    err = command_test<Clusters::ColorControl::Commands::StopMoveStep::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/StopMoveStep", R"({"OptionsMask":0,"OptionsOverride":0})",
        request2);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandEnhancedMoveToHueAndSaturation)
{
    Clusters::ColorControl::Commands::EnhancedMoveToHueAndSaturation::Type request;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::EnhancedMoveToHueAndSaturation::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/EnhancedMoveToHueAndSaturation",
        R"({"EnhancedHue":0,"OptionsMask":0,"OptionsOverride":0,"Saturation":0,"TransitionTime":0})", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandColorLoopSet)
{
    Clusters::ColorControl::Commands::ColorLoopSet::Type request;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::ColorLoopSet::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/ColorLoopSet",
        R"({"Action":"DeactivateColorLoop","Direction":"DecrementEnhancedCurrentHue","OptionsMask":0,"OptionsOverride":0,"StartHue":0,"Time":0,"UpdateFlags":{"UpdateAction":false,"UpdateDirection":false,"UpdateStartHue":false,"UpdateTime":false}})",
        request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandStopMoveStep)
{
    Clusters::ColorControl::Commands::StopMoveStep::Type request;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::StopMoveStep::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/StopMoveStep", R"({"OptionsMask":0,"OptionsOverride":0})",
        request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandMoveColorTemperature)
{
    Clusters::ColorControl::Commands::MoveColorTemperature::Type request;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::MoveColorTemperature::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/MoveColorTemperature",
        R"({"ColorTemperatureMaximumMireds":0,"ColorTemperatureMinimumMireds":0,"MoveMode":"Stop","OptionsMask":0,"OptionsOverride":0,"Rate":0})",
        request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestColorControl, CommandStepColorTemperature)
{
    Clusters::ColorControl::Commands::StepColorTemperature::Type request;
    request.stepMode = chip::app::Clusters::ColorControl::HueStepMode::kUp;
    request.stepSize = 1;
    CHIP_ERROR err = command_test<Clusters::ColorControl::Commands::StepColorTemperature::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/StepColorTemperature",
        R"({"ColorTemperatureMaximumMireds":0,"ColorTemperatureMinimumMireds":0,"OptionsMask":0,"OptionsOverride":0,"StepMode":"Up","StepSize":1,"TransitionTime":0})",
        request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    Clusters::ColorControl::Commands::StopMoveStep::Type request2;
    err = command_test<Clusters::ColorControl::Commands::StopMoveStep::Type>(
        "ucl/by-unid/zw-0x0002/ep2/ColorControl/Commands/StopMoveStep", R"({"OptionsMask":0,"OptionsOverride":0})",
        request2);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}