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

// Chip components
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Third party library
#include <nlunit-test.h>


using namespace unify::matter_bridge;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::WindowCovering;
using TestContext = unify::matter_bridge::Test::ClusterContext<WindowCoveringAttributeAccess, 
WindowCoveringClusterCommandHandler>;
    
static int Initialize(void * context)
{
    if (TestContext::Initialize(context, true) != SUCCESS)
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

static int Finalize(void * context)
{
    if (TestContext::Finalize(context, true) != SUCCESS)
    {
        return FAILURE;
    }
    return SUCCESS;
}

static void TestWindowCoveringAttributeInstalledOpenLimitLift(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::WindowCovering::Attributes::InstalledOpenLimitLift::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/InstalledOpenLimitLift/Reported", R"({ "value": 0 })",
        0);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestWindowCoveringAttributeInstalledClosedLimitLift(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::WindowCovering::Attributes::InstalledClosedLimitLift::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/InstalledClosedLimitLift/Reported", R"({ "value": 65534 })",
        65534);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestWindowCoveringAttributeCurrentPositionLift(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::WindowCovering::Attributes::CurrentPositionLift::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/CurrentPositionLift/Reported", R"({ "value": 0 })",
        static_cast<Clusters::WindowCovering::Attributes::CurrentPositionLift::TypeInfo::Type>(0));
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestWindowCoveringAttributeCurrentPositionLiftPercentage(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/CurrentPositionLiftPercentage/Reported", R"({ "value": 0 })",
        static_cast<Clusters::WindowCovering::Attributes::CurrentPositionLiftPercentage::TypeInfo::Type>(0));
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestWindowCoveringCommandUpOrOpen(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::WindowCovering::Commands::UpOrOpen::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::WindowCovering::Commands::UpOrOpen::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Commands/UpOrOpen", "{}", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestWindowCoveringCommandDownOrClose(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::WindowCovering::Commands::DownOrClose::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::WindowCovering::Commands::DownOrClose::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Commands/DownOrClose", "{}", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestWindowCoveringCommandStopMotion(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    Clusters::WindowCovering::Commands::StopMotion::Type request;
    CHIP_ERROR err = ctx.command_test<Clusters::WindowCovering::Commands::StopMotion::Type>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Commands/Stop", "{}", request);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
} 

static void TestWindowCoveringAttributeMode(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err = ctx.attribute_test<Clusters::WindowCovering::Attributes::Mode::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/Mode/Reported", R"({ "value": {
        "CalibrationMode": false,
        "LEDFeedback": false,
        "MaintenanceMode": false,
        "MotorDirectionReversed": false
        } })",
        static_cast<Clusters::WindowCovering::Attributes::Mode::TypeInfo::Type>(0));
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestWindowCoveringAttributeType(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err = ctx.attribute_test<Clusters::WindowCovering::Attributes::Type::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/WindowCoveringType/Reported",R"({ "value": "Rollershade" })",
        Clusters::WindowCovering::Type::kRollerShade);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestWindowCoveringAttributeConfigStatus(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err = ctx.attribute_test<Clusters::WindowCovering::Attributes::ConfigStatus::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/ConfigOrStatus/Reported", R"({ "value": {
        "LiftClosedLoop": false,
        "LiftEncoderControlled": false,
        "Online": false,
        "OpenAndUpCommandsReversed": false,
        "Operational": false,
        "TiltClosedLoop": false,
        "TiltEncoderControlled": false
        } })",
        static_cast<Clusters::WindowCovering::Attributes::ConfigStatus::TypeInfo::Type>(0));
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}
 
static void TestWindowCoveringAttributeEndProductType(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err = ctx.attribute_test<Clusters::WindowCovering::Attributes::EndProductType::TypeInfo>(
        sSuite, EndProductType::kRollerShade);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestWindowCoveringAttributeClusterRevision(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::WindowCovering::Attributes::ClusterRevision::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/WindowCovering/Attributes/ClusterRevision/Reported", R"({ "value": 5 })",
        5);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestWindowCoveringAttributeOperationalStatus(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::WindowCovering::Attributes::OperationalStatus::TypeInfo>(
        sSuite, 0);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {

    NL_TEST_DEF("WindowCovering::TestWindowCoveringAttributeInstalledOpenLimitLift",TestWindowCoveringAttributeInstalledOpenLimitLift),
    NL_TEST_DEF("WindowCovering::TestWindowCoveringAttributeInstalledClosedLimitLift",TestWindowCoveringAttributeInstalledClosedLimitLift),
    NL_TEST_DEF("WindowCovering::TestWindowCoveringAttributeCurrentPositionLift",TestWindowCoveringAttributeCurrentPositionLift),
    NL_TEST_DEF("WindowCovering::TestWindowCoveringCommandDownOrClose", TestWindowCoveringCommandDownOrClose),
    NL_TEST_DEF("WindowCovering::TestWindowCoveringCommandUpOrOpen", TestWindowCoveringCommandUpOrOpen),
    NL_TEST_DEF("WindowCovering::TestWindowCoveringAttributeCurrentPositionLiftPercentage",TestWindowCoveringAttributeCurrentPositionLiftPercentage),
    NL_TEST_DEF("WindowCovering::TestWindowCoveringCommandDownOrClose", TestWindowCoveringCommandDownOrClose),
    NL_TEST_DEF("WindowCovering::TestWindowCoveringCommandStopMotion", TestWindowCoveringCommandStopMotion),
    NL_TEST_DEF("WindowCovering::TestWindowCoveringAttributeMode",TestWindowCoveringAttributeMode),
    NL_TEST_DEF("WindowCovering::TestWindowCoveringAttributeEndProductType",TestWindowCoveringAttributeEndProductType),
    NL_TEST_DEF("WindowCovering::TestWindowCoveringAttributeClusterRevision",TestWindowCoveringAttributeClusterRevision),
    NL_TEST_DEF("WindowCovering::TestWindowCoveringAttributeType",TestWindowCoveringAttributeType),
    NL_TEST_DEF("WindowCovering::TestWindowCoveringAttributeConfigStatus",TestWindowCoveringAttributeConfigStatus),
    NL_TEST_DEF("WindowCovering::TestWindowCoveringAttributeOperationalStatus",TestWindowCoveringAttributeOperationalStatus),
    NL_TEST_SENTINEL()
    };

static nlTestSuite sSuite = { "WindowCoveringTests", &sTests[0], Initialize, Finalize };

int TestWindowCoveringSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestWindowCoveringSuite)