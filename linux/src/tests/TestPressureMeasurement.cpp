#include "ClusterTestContext.h"
#include "command_translator.hpp"
#include "attribute_translator.hpp"

// Chip components
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Third party library
#include <nlunit-test.h>

using namespace unify::matter_bridge;
using namespace chip::app;
using namespace chip::app::Clusters;
using TestContext = unify::matter_bridge::Test::ClusterContext<PressureMeasurementAttributeAccess, PressureMeasurementClusterCommandHandler>;

static int Initialize(void * context)
{
    if (TestContext::Initialize(context) != SUCCESS)
        return FAILURE;

    auto * ctx         = static_cast<TestContext *>(context);
    auto & ep          = ctx->get_endpoint();
    auto & cluster     = ep.emplace_cluster("PressureMeasurement");
    cluster.attributes = {
        "ScaledValue",
        "MinScaledValue",
        "MaxScaledValue",
        "Scale",
        "FeatureMap",
    };

    return ctx->register_endpoint(ep);
}

static void TestPressureMeasurementAttributeFeatureMap(nlTestSuite * sSuite, void * apContext) {
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::PressureMeasurement::Attributes::FeatureMap::TypeInfo>(
        sSuite, 0);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestPressureMeasurementAttributeScaledValue(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<int16_t> value;
    value.SetNonNull((int16_t) 0);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::PressureMeasurement::Attributes::ScaledValue::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/PressureMeasurement/Attributes/ScaledValue/Reported", R"({ "value": 0 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestPressureMeasurementAttributeMinScaledValue(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<int16_t> value;
    value.SetNonNull((int16_t) 0);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::PressureMeasurement::Attributes::MinScaledValue::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/PressureMeasurement/Attributes/MinScaledValue/Reported", R"({ "value": 0 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestPressureMeasurementAttributeMaxScaledValue(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    DataModel::Nullable<int16_t> value;
    value.SetNonNull((int16_t) 10);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::PressureMeasurement::Attributes::MaxScaledValue::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/PressureMeasurement/Attributes/MaxScaledValue/Reported", R"({ "value": 10 })", value);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestPressureMeasurementAttributeScale(nlTestSuite * sSuite, void * apContext)
{
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::PressureMeasurement::Attributes::Scale::TypeInfo>(
        sSuite, "ucl/by-unid/zw-0x0002/ep2/PressureMeasurement/Attributes/Scale/Reported", R"({ "value": 0 })", 0);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

static void TestPressureMeasurementAttributeFeatureMapEXT(nlTestSuite * sSuite, void * apContext) {
    TestContext & ctx = *static_cast<TestContext *>(apContext);
    CHIP_ERROR err    = ctx.attribute_test<Clusters::PressureMeasurement::Attributes::FeatureMap::TypeInfo>(
        sSuite, 1);
    NL_TEST_ASSERT(sSuite, err == CHIP_NO_ERROR);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = {
    NL_TEST_DEF("PressureMeasurement::TestPressureMeasurementAttributeFeatureMap", TestPressureMeasurementAttributeFeatureMap),
    NL_TEST_DEF("PressureMeasurement::TestPressureMeasurementAttributeScaledValue", TestPressureMeasurementAttributeScaledValue),
    NL_TEST_DEF("PressureMeasurement::TestPressureMeasurementAttributeFeatureMap", TestPressureMeasurementAttributeFeatureMap),
    NL_TEST_DEF("PressureMeasurement::TestPressureMeasurementAttributeMinScaledValue", TestPressureMeasurementAttributeMinScaledValue),
    NL_TEST_DEF("PressureMeasurement::TestPressureMeasurementAttributeFeatureMap", TestPressureMeasurementAttributeFeatureMap),
    NL_TEST_DEF("PressureMeasurement::TestPressureMeasurementAttributeMaxScaledValue", TestPressureMeasurementAttributeMaxScaledValue),
    NL_TEST_DEF("PressureMeasurement::TestPressureMeasurementAttributeFeatureMap", TestPressureMeasurementAttributeFeatureMap),
    NL_TEST_DEF("PressureMeasurement::TestPressureMeasurementAttributeScale", TestPressureMeasurementAttributeScale),
    NL_TEST_DEF("PressureMeasurement::TestPressureMeasurementAttributeFeatureMapEXT", TestPressureMeasurementAttributeFeatureMapEXT),
    NL_TEST_SENTINEL()
};

static nlTestSuite sSuite = { "PressureMeasurementTests", &sTests[0], Initialize, TestContext::Finalize };

int TestPressureMeasurementSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPressureMeasurementSuite)
