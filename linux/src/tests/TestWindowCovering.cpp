/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "ClusterTestContext.h"
#include <app/CommandHandler.h>
#include "command_translator.hpp"
#include "attribute_translator.hpp"
#include <app/InteractionModelEngine.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include "command_translator_interface.hpp"
#include "matter.h"

// Mocks
#include "MockClusterCommandHandler.hpp"


// Third party library
#include <gtest/gtest.h>


using namespace unify::matter_bridge;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::WindowCovering;
using TestContext = unify::matter_bridge::Test::ClusterContext<WindowCoveringAttributeAccess, 
WindowCoveringClusterCommandHandler>;

class TestWindowCovering : public TestContext {
public:

    void SetUp() {
        EXPECT_EQ(TestWindowCovering::Initialize(this),1);
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
    auto & cluster     = ep.emplace_cluster("WindowCovering");
    cluster.attributes = {
        "WindowCoveringType",
        "ConfigOrStatus",
        "OperationalStatus",
        "EndProductType",
        "Mode",
        "CurrentPositionLift",
        "CurrentPositionLiftPercentage",
        "InstalledOpenLimitLift",
        "InstalledClosedLimitLift",
        "GeneratedCommandList",
        "AcceptedCommandList",
        "AttributeList",
        "FeatureMap",
        "ClusterRevision",
    };
    cluster.supported_commands = {
        "UpOrOpen", "DownOrClose", "StopMotion",
    };
    return ctx->register_endpoint(ep);
}
};

TEST_F(TestWindowCovering, TestWindowCoveringAttributeInstalledOpenLimitLift)
{
    CHIP_ERROR err    = attribute_test<Clusters::WindowCovering::Attributes::InstalledOpenLimitLift::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/InstalledOpenLimitLift/Reported", R"({ "value": 0 })",
        0);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestWindowCovering, _pw_unit_test_Info_TestWindowCovering_TestWindowCoveringAttributeMode)
{
    Clusters::WindowCovering::Attributes::Mode::TypeInfo::Type mode = static_cast<Clusters::WindowCovering::Attributes::Mode::TypeInfo::Type>(0);
    mode.Set(WindowCovering::Mode::kMaintenanceMode);
    CHIP_ERROR err    = attribute_test<Clusters::WindowCovering::Attributes::Mode::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/Mode/Reported", R"({
            "value": {
                "CalibrationMode": false,
                "LEDFeedback": false,
                "MaintenanceMode": true,
                "MotorDirectionReversed": false
                }
            })", mode);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestWindowCovering, TestWindowCoveringAttributeInstalledClosedLimitLift)
{
    CHIP_ERROR err    = attribute_test<Clusters::WindowCovering::Attributes::InstalledClosedLimitLift::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/InstalledClosedLimitLift/Reported", R"({ "value": 65534 })",
        65534);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestWindowCovering, TestWindowCoveringAttributeCurrentPositionLift)
{
    CHIP_ERROR err    = attribute_test<Clusters::WindowCovering::Attributes::CurrentPositionLift::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/CurrentPositionLift/Reported", R"({ "value": 0 })",
        static_cast<Clusters::WindowCovering::Attributes::CurrentPositionLift::TypeInfo::Type>(0));
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestWindowCovering, TestWindowCoveringAttributeCurrentPositionLiftPercentage)
{
    CHIP_ERROR err    = attribute_test<Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/CurrentPositionLiftPercentage/Reported", R"({ "value": 0 })",
        static_cast<Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::TypeInfo::Type>(0));
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestWindowCovering, TestWindowCoveringCommandUpOrOpen)
{
    CHIP_ERROR expected_err = CHIP_ERROR_IM_GLOBAL_STATUS_VALUE(::chip::Protocols::InteractionModel::Status::Busy);
    Clusters::WindowCovering::Commands::UpOrOpen::Type request;
    CHIP_ERROR err = command_test<Clusters::WindowCovering::Commands::UpOrOpen::Type>(
        "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Commands/UpOrOpen", "{}", request);
    EXPECT_EQ(err, expected_err);

    Clusters::WindowCovering::Attributes::Mode::TypeInfo::Type mode = static_cast<Clusters::WindowCovering::Attributes::Mode::TypeInfo::Type>(0);
    CHIP_ERROR err_mode    = attribute_test<Clusters::WindowCovering::Attributes::Mode::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/Mode/Reported", R"({
            "value": {
                "CalibrationMode": false,
                "LEDFeedback": false,
                "MaintenanceMode": false,
                "MotorDirectionReversed": false
                }
            })", mode);
    EXPECT_EQ(err_mode, CHIP_NO_ERROR);

    err = command_test<Clusters::WindowCovering::Commands::UpOrOpen::Type>(
        "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Commands/UpOrOpen", "{}", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestWindowCovering, TestWindowCoveringCommandDownOrClose)
{
    Clusters::WindowCovering::Attributes::Mode::TypeInfo::Type mode = static_cast<Clusters::WindowCovering::Attributes::Mode::TypeInfo::Type>(0);
    CHIP_ERROR err_mode    = attribute_test<Clusters::WindowCovering::Attributes::Mode::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/Mode/Reported", R"({
            "value": {
                "CalibrationMode": false,
                "LEDFeedback": false,
                "MaintenanceMode": false,
                "MotorDirectionReversed": false
                }
            })", mode);
    EXPECT_EQ(err_mode, CHIP_NO_ERROR);

    Clusters::WindowCovering::Commands::DownOrClose::Type request;
    CHIP_ERROR err = command_test<Clusters::WindowCovering::Commands::DownOrClose::Type>(
        "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Commands/DownOrClose", "{}", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestWindowCovering, TestWindowCoveringCommandStopMotion)
{
    Clusters::WindowCovering::Attributes::Mode::TypeInfo::Type mode = static_cast<Clusters::WindowCovering::Attributes::Mode::TypeInfo::Type>(0);
    CHIP_ERROR err_mode    = attribute_test<Clusters::WindowCovering::Attributes::Mode::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/Mode/Reported", R"({
            "value": {
                "CalibrationMode": false,
                "LEDFeedback": false,
                "MaintenanceMode": false,
                "MotorDirectionReversed": false
                }
            })", mode);
    EXPECT_EQ(err_mode, CHIP_NO_ERROR);

    Clusters::WindowCovering::Commands::StopMotion::Type request;
    CHIP_ERROR err = command_test<Clusters::WindowCovering::Commands::StopMotion::Type>(
        "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Commands/Stop", "{}", request);
    EXPECT_EQ(err, CHIP_NO_ERROR);
} 

TEST_F(TestWindowCovering, TestWindowCoveringAttributeMode)
{
    CHIP_ERROR err = attribute_test<Clusters::WindowCovering::Attributes::Mode::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/Mode/Reported", R"({ "value": {
        "CalibrationMode": false,
        "LEDFeedback": false,
        "MaintenanceMode": false,
        "MotorDirectionReversed": false
        } })",
        static_cast<Clusters::WindowCovering::Attributes::Mode::TypeInfo::Type>(0));
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestWindowCovering, TestWindowCoveringAttributeType)
{
    CHIP_ERROR err = attribute_test<Clusters::WindowCovering::Attributes::Type::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/WindowCoveringType/Reported",R"({ "value": "Rollershade" })",
        Clusters::WindowCovering::Type::kRollerShade);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestWindowCovering, TestWindowCoveringAttributeConfigStatus)
{
    CHIP_ERROR err = attribute_test<Clusters::WindowCovering::Attributes::ConfigStatus::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/ConfigOrStatus/Reported", R"({ "value": {
        "LiftClosedLoop": false,
        "LiftEncoderControlled": false,
        "Online": false,
        "OpenAndUpCommandsReversed": false,
        "Operational": false,
        "TiltClosedLoop": false,
        "TiltEncoderControlled": false
        } })",
        static_cast<Clusters::WindowCovering::Attributes::ConfigStatus::TypeInfo::Type>(0));
    EXPECT_EQ(err, CHIP_NO_ERROR);
}
 
TEST_F(TestWindowCovering, TestWindowCoveringAttributeEndProductType)
{
    CHIP_ERROR err = attribute_test<Clusters::WindowCovering::Attributes::EndProductType::TypeInfo>(
        EndProductType::kRollerShade);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestWindowCovering, TestWindowCoveringAttributeClusterRevision)
{
    CHIP_ERROR err    = attribute_test<Clusters::WindowCovering::Attributes::ClusterRevision::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/ClusterRevision/Reported", R"({ "value": 5 })",
        5);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestWindowCovering, TestWindowCoveringAttributeOperationalStatus)
{
    CHIP_ERROR err    = attribute_test<Clusters::WindowCovering::Attributes::OperationalStatus::TypeInfo>(
        0);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}