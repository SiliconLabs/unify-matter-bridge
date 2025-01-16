#include "ClusterTestContext.h"
#include "command_translator.hpp"
#include "attribute_translator.hpp"


// Third party library
#include <gtest/gtest.h>

using namespace unify::matter_bridge;
using namespace chip::app;
using namespace chip::app::Clusters;
using TestContext = unify::matter_bridge::Test::ClusterContext<PressureMeasurementAttributeAccess, PressureMeasurementClusterCommandHandler>;

class TestPressureMeasurement : public TestContext {
public:

    void SetUp() {
        EXPECT_EQ(TestPressureMeasurement::Initialize(this),1);
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
};

TEST_F(TestPressureMeasurement, TestPressureMeasurementAttributeFeatureMap) 
{
    CHIP_ERROR err    = attribute_test<Clusters::PressureMeasurement::Attributes::FeatureMap::TypeInfo>(
        0);
    EXPECT_EQ( err, CHIP_NO_ERROR);
}

TEST_F(TestPressureMeasurement, TestPressureMeasurementAttributeScaledValue)
{
    DataModel::Nullable<int16_t> value;
    value.SetNonNull((int16_t) 0);
    CHIP_ERROR err    = attribute_test<Clusters::PressureMeasurement::Attributes::ScaledValue::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/PressureMeasurement/Attributes/ScaledValue/Reported", R"({ "value": 0 })", value);
    EXPECT_EQ( err, CHIP_NO_ERROR);
}

TEST_F(TestPressureMeasurement, TestPressureMeasurementAttributeMinScaledValue)
{
    DataModel::Nullable<int16_t> value;
    value.SetNonNull((int16_t) 0);
    CHIP_ERROR err    = attribute_test<Clusters::PressureMeasurement::Attributes::MinScaledValue::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/PressureMeasurement/Attributes/MinScaledValue/Reported", R"({ "value": 0 })", value);
    EXPECT_EQ( err, CHIP_NO_ERROR);
}

TEST_F(TestPressureMeasurement, TestPressureMeasurementAttributeMaxScaledValue)
{
    DataModel::Nullable<int16_t> value;
    value.SetNonNull((int16_t) 10);
    CHIP_ERROR err    = attribute_test<Clusters::PressureMeasurement::Attributes::MaxScaledValue::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/PressureMeasurement/Attributes/MaxScaledValue/Reported", R"({ "value": 10 })", value);
    EXPECT_EQ( err, CHIP_NO_ERROR);
}

TEST_F(TestPressureMeasurement, TestPressureMeasurementAttributeScale)
{
    CHIP_ERROR err    = attribute_test<Clusters::PressureMeasurement::Attributes::Scale::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/PressureMeasurement/Attributes/Scale/Reported", R"({ "value": 0 })", 0);
    EXPECT_EQ( err, CHIP_NO_ERROR);
}

TEST_F(TestPressureMeasurement, TestPressureMeasurementAttributeFeatureMapEXT) 
{
    CHIP_ERROR err    = attribute_test<Clusters::PressureMeasurement::Attributes::FeatureMap::TypeInfo>(
        1);
    EXPECT_EQ( err, CHIP_NO_ERROR);
}
