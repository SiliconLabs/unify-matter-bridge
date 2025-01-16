#include "ClusterTestContext.h"
#include "command_translator.hpp"
#include "attribute_translator.hpp"


// Third party library
#include <gtest/gtest.h>

using namespace unify::matter_bridge;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::Thermostat;

using TestContext = unify::matter_bridge::Test::ClusterContext<ThermostatAttributeAccess, ThermostatClusterCommandHandler>;

class TestThermostat : public TestContext {
public:

    void SetUp() {
        EXPECT_EQ(TestThermostat::Initialize(this),1);
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
    auto & cluster     = ep.emplace_cluster("Thermostat");
    cluster.attributes = {
        "LocalTemperature",
        "OccupiedCoolingSetpoint",
        "OccupiedHeatingSetpoint",
        "MinHeatSetpointLimit",
        "MaxHeatSetpointLimit",
        "MinCoolSetpointLimit",
        "MaxCoolSetpointLimit",
        "ControlSequenceOfOperation",
        "SystemMode",
        "FeatureMap",
    };
    cluster.supported_commands = {
        "SetpointRaiseOrLower",   
    };
    return ctx->register_endpoint(ep);
}
};

TEST_F(TestThermostat, TestThermostatAttributeLocalTemprature)
{
    CHIP_ERROR err    = attribute_test<Attributes::LocalTemperature::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/LocalTemperature/Reported", R"({ "value": 2000 })", 
        chip::app::DataModel::Nullable<int16_t>(2000));
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatAttributeOccupiedCoolingSetpoint)
{
    CHIP_ERROR err    = attribute_test<Attributes::OccupiedCoolingSetpoint::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/OccupiedCoolingSetpoint/Reported", R"({ "value": 2000 })", 2000);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatAttributeOccupiedHeatingSetpoint)
{
    CHIP_ERROR err    = attribute_test<Attributes::OccupiedHeatingSetpoint::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/OccupiedHeatingSetpoint/Reported", R"({ "value": 2700 })", 2700);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatAttributeMinHeatSetpointLimit)
{
    CHIP_ERROR err    = attribute_test<Attributes::MinHeatSetpointLimit::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/MinHeatSetpointLimit/Reported", R"({ "value": 1500 })", 1500);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatAttributeMaxHeatSetpointLimit)
{
    CHIP_ERROR err    = attribute_test<Attributes::MaxHeatSetpointLimit::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/MaxHeatSetpointLimit/Reported", R"({ "value": 3500 })", 3500);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatAttributeMinCoolSetpointLimit)
{
    CHIP_ERROR err    = attribute_test<Attributes::MinCoolSetpointLimit::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/MinCoolSetpointLimit/Reported", R"({ "value": 1000 })", 1000);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatAttributeMaxCoolSetpointLimit)
{
    CHIP_ERROR err    = attribute_test<Attributes::MaxCoolSetpointLimit::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/MaxCoolSetpointLimit/Reported", R"({ "value": 3000 })", 3000);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatAttributeControlSequenceOfOperation)
{
    CHIP_ERROR err = CHIP_ERROR_NOT_IMPLEMENTED;
    err    = attribute_test<Attributes::ControlSequenceOfOperation::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/ControlSequenceOfOperation/Reported", 
        R"({ "value": "CoolingOnly"})", chip::app::Clusters::Thermostat::ControlSequenceOfOperationEnum::kCoolingOnly);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err   = attribute_test<Attributes::ControlSequenceOfOperation::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/ControlSequenceOfOperation/Reported", 
        R"({ "value": "CoolingWithReheat"})", chip::app::Clusters::Thermostat::ControlSequenceOfOperationEnum::kCoolingWithReheat);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err    = attribute_test<Attributes::ControlSequenceOfOperation::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/ControlSequenceOfOperation/Reported", 
        R"({ "value": "HeatingOnly"})", chip::app::Clusters::Thermostat::ControlSequenceOfOperationEnum::kHeatingOnly);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err    = attribute_test<Attributes::ControlSequenceOfOperation::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/ControlSequenceOfOperation/Reported", 
        R"({ "value": "HeatingWithReheat"})", chip::app::Clusters::Thermostat::ControlSequenceOfOperationEnum::kHeatingWithReheat);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatAttributeSystemMode)
{
    CHIP_ERROR err = CHIP_ERROR_NOT_IMPLEMENTED;
    err     = attribute_test<Attributes::SystemMode::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "Off"})", SystemModeEnum::kOff);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err    = attribute_test<Attributes::SystemMode::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "Auto"})", SystemModeEnum::kAuto);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err    = attribute_test<Attributes::SystemMode::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "Cool"})", SystemModeEnum::kCool);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err    = attribute_test<Attributes::SystemMode::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "Heat"})", SystemModeEnum::kHeat);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err    = attribute_test<Attributes::SystemMode::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "EmergencyHeating"})", SystemModeEnum::kEmergencyHeat);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err    = attribute_test<Attributes::SystemMode::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "Precooling"})", SystemModeEnum::kPrecooling);    
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err    = attribute_test<Attributes::SystemMode::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "FanOnly"})", SystemModeEnum::kFanOnly);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err    = attribute_test<Attributes::SystemMode::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "Dry"})", SystemModeEnum::kDry);    
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err    = attribute_test<Attributes::SystemMode::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/SystemMode/Reported", R"({ "value": "Sleep"})", SystemModeEnum::kSleep);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatCommandSetpointRaiseOrLowerHeat)
{
    Commands::SetpointRaiseLower::Type request;
    request.mode = SetpointRaiseLowerModeEnum::kHeat;
    request.amount = 0;

    CHIP_ERROR err = command_test<Commands::SetpointRaiseLower::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Commands/SetpointRaiseOrLower",
        R"({"Mode":"Heat","Amount":0})", request);

    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatCommandSetpointRaiseOrLowerCool)
{
    Commands::SetpointRaiseLower::Type request;
    request.mode = SetpointRaiseLowerModeEnum::kCool;
    request.amount = 5;

    CHIP_ERROR err = command_test<Commands::SetpointRaiseLower::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Commands/SetpointRaiseOrLower",
        R"({"Mode":"Cool","Amount":5})", request);

    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatCommandSetpointRaiseOrLowerBoth)
{
    Commands::SetpointRaiseLower::Type request;
    request.mode = SetpointRaiseLowerModeEnum::kBoth;
    request.amount = 10;

    CHIP_ERROR err = command_test<Commands::SetpointRaiseLower::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Commands/SetpointRaiseOrLower",
        R"({"Mode":"Both","Amount":10})", request);

    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatCommandSetpointRaiseOrLowerUpdatOccupiedHeatingSetpointMin)
{
    Commands::SetpointRaiseLower::Type request;
    request.mode = SetpointRaiseLowerModeEnum::kHeat;
    request.amount = -128;

    CHIP_ERROR err = command_test<Commands::SetpointRaiseLower::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Commands/SetpointRaiseOrLower",
        R"({"Mode":"Heat","Amount":-120})", request);

    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatCommandSetpointRaiseOrLowerUpdatOccupiedHeatingSetpointMax)
{
    Commands::SetpointRaiseLower::Type request;
    request.mode = SetpointRaiseLowerModeEnum::kHeat;
    request.amount = 127;

    CHIP_ERROR err = command_test<Commands::SetpointRaiseLower::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Commands/SetpointRaiseOrLower",
        R"({"Mode":"Heat","Amount":130})", request);

    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatCommandSetpointRaiseOrLowerUpdatOccupiedCoolingSetpointMin)
{
    Commands::SetpointRaiseLower::Type request;
    request.mode = SetpointRaiseLowerModeEnum::kCool;
    request.amount = -120;

    CHIP_ERROR err = command_test<Commands::SetpointRaiseLower::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Commands/SetpointRaiseOrLower",
        R"({"Mode":"Cool","Amount":-50})", request);

    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatCommandSetpointRaiseOrLowerUpdatOccupiedCoolingSetpointMax)
{
    Commands::SetpointRaiseLower::Type request;
    request.mode = SetpointRaiseLowerModeEnum::kCool;
    request.amount = 110;

    CHIP_ERROR err = command_test<Commands::SetpointRaiseLower::Type>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Commands/SetpointRaiseOrLower",
        R"({"Mode":"Cool","Amount":100})", request);

    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatWriteAttributeOccupiedCoolingSetpoint)
{
    using namespace chip::app::Clusters::Thermostat; 
    CHIP_ERROR err = attribute_write_test<Attributes::OccupiedCoolingSetpoint::TypeInfo>(
        "Thermostat/Commands/WriteAttributes", R"({"OccupiedCoolingSetpoint": 2000})", 2000);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatWriteAttributeOccupiedCoolingSetpointUpperBound)
{
    using namespace chip::app::Clusters::Thermostat; 
    CHIP_ERROR err = attribute_write_test<Attributes::OccupiedCoolingSetpoint::TypeInfo>(
        "Thermostat/Commands/WriteAttributes", R"({"OccupiedCoolingSetpoint": 3800})", 3800);
    EXPECT_EQ(chip::app::StatusIB(err).mStatus, Protocols::InteractionModel::Status::ConstraintError);
}

TEST_F(TestThermostat, TestThermostatWriteAttributeOccupiedCoolingSetpointLowerBound)
{
    using namespace chip::app::Clusters::Thermostat; 
    CHIP_ERROR err = attribute_write_test<Attributes::OccupiedCoolingSetpoint::TypeInfo>(
        "Thermostat/Commands/WriteAttributes", R"({"OccupiedCoolingSetpoint": 800})", 800);
    EXPECT_EQ(chip::app::StatusIB(err).mStatus, Protocols::InteractionModel::Status::ConstraintError);
}

TEST_F(TestThermostat, TestThermostatWriteAttributeOccupiedHeatingSetpoint)
{
    using namespace chip::app::Clusters::Thermostat; 
    CHIP_ERROR err = attribute_write_test<Attributes::OccupiedHeatingSetpoint::TypeInfo>(
        "Thermostat/Commands/WriteAttributes", R"({"OccupiedHeatingSetpoint": 2200})", 2200);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatWriteAttributeOccupiedHeatingSetpointUpperBound)
{
    using namespace chip::app::Clusters::Thermostat; 
    CHIP_ERROR err = attribute_write_test<Attributes::OccupiedHeatingSetpoint::TypeInfo>(
        "Thermostat/Commands/WriteAttributes", R"({"OccupiedHeatingSetpoint": 3800})", 3800);
    EXPECT_EQ(chip::app::StatusIB(err).mStatus, Protocols::InteractionModel::Status::ConstraintError);
}

TEST_F(TestThermostat, TestThermostatWriteAttributeOccupiedHeatingSetpointLowerBound)
{
    using namespace chip::app::Clusters::Thermostat; 
    CHIP_ERROR err = attribute_write_test<Attributes::OccupiedHeatingSetpoint::TypeInfo>(
        "Thermostat/Commands/WriteAttributes", R"({"OccupiedHeatingSetpoint": 500})", 500);
    EXPECT_EQ(chip::app::StatusIB(err).mStatus, Protocols::InteractionModel::Status::ConstraintError);
}

TEST_F(TestThermostat, TestThermostatWriteAttributeMinHeatSetpointLimit)
{
    using namespace chip::app::Clusters::Thermostat; 
    CHIP_ERROR err = attribute_write_test<Attributes::MinHeatSetpointLimit::TypeInfo>(
        "Thermostat/Commands/WriteAttributes", R"({"MinHeatSetpointLimit": 1000})", 1000);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatWriteAttributeMaxHeatSetpointLimit)
{
    using namespace chip::app::Clusters::Thermostat; 
    CHIP_ERROR err = attribute_write_test<Attributes::MaxHeatSetpointLimit::TypeInfo>(
        "Thermostat/Commands/WriteAttributes", R"({"MaxHeatSetpointLimit": 3000})", 3000);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatWriteAttributeMinCoolSetpointLimit)
{
    using namespace chip::app::Clusters::Thermostat; 
    CHIP_ERROR err = attribute_write_test<Attributes::MinCoolSetpointLimit::TypeInfo>(
        "Thermostat/Commands/WriteAttributes", R"({"MinCoolSetpointLimit": 1500})", 1500);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatWriteAttributeMaxCoolSetpointLimit)
{
    using namespace chip::app::Clusters::Thermostat; 
    CHIP_ERROR err = attribute_write_test<Attributes::MaxCoolSetpointLimit::TypeInfo>(
        "Thermostat/Commands/WriteAttributes", R"({"MaxCoolSetpointLimit": 2500})", 2500);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatWriteAttributeControlSequenceOfOperation)
{
    using namespace chip::app::Clusters::Thermostat; 
    CHIP_ERROR err = attribute_write_test<Attributes::ControlSequenceOfOperation::TypeInfo>(
        "Thermostat/Commands/WriteAttributes", R"({"ControlSequenceOfOperation": "CoolingOnly"})",
        ControlSequenceOfOperationEnum::kCoolingOnly);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatWriteAttributeSystemMode)
{
    using namespace chip::app::Clusters::Thermostat; 
    CHIP_ERROR err = attribute_write_test<Attributes::SystemMode::TypeInfo>(
        "Thermostat/Commands/WriteAttributes", R"({"SystemMode":"Auto"})", SystemModeEnum::kAuto);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestThermostat, TestThermostatAttributeFeatureMap)
{
    CHIP_ERROR err    = attribute_test<Attributes::FeatureMap::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/Thermostat/Attributes/FeatureMap/Reported", R"({ "value": 3 })", 3);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}