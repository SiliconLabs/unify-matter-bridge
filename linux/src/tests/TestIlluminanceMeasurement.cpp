#include "ClusterTestContext.h"
#include "command_translator.hpp"
#include "attribute_translator.hpp"


// Third party library
#include <gtest/gtest.h>
#include <app/tests/AppTestContext.h>
#include <pw_unit_test/framework.h>

using namespace unify::matter_bridge;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::IlluminanceMeasurement;

using TestContext = unify::matter_bridge::Test::ClusterContext<IlluminanceMeasurementAttributeAccess, IlluminanceMeasurementClusterCommandHandler>;

class TestIlluminanceMeasurement : public TestContext {
public:

    void SetUp() {
        EXPECT_EQ(TestIlluminanceMeasurement::Initialize(this),1);
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
        auto & cluster     = ep.emplace_cluster("IlluminanceMeasurement");
        cluster.attributes = {
            "LightSensorType",
        };
        return ctx->register_endpoint(ep);
    }
};


TEST_F(TestIlluminanceMeasurement, TestIlluminanceMeasurementAttributeIllumLightSensorType_1) {
    
    CHIP_ERROR err = attribute_test<Attributes::LightSensorType::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/IlluminanceMeasurement/Attributes/LightSensorType/Reported", 
        R"({ "value": "Photodiode"})", 
        MakeNullable(LightSensorTypeEnum::kPhotodiode));
    EXPECT_EQ(err,CHIP_NO_ERROR);
}

TEST_F(TestIlluminanceMeasurement, TestIlluminanceMeasurementAttributeIllumLightSensorType_2)
{   
    CHIP_ERROR err    = attribute_test<Attributes::LightSensorType::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/IlluminanceMeasurement/Attributes/LightSensorType/Reported", R"({ "value": "CMOS"})", MakeNullable(LightSensorTypeEnum::kCmos));
    EXPECT_EQ(err,CHIP_NO_ERROR);
}