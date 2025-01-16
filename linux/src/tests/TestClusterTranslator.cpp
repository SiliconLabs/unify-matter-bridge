// Chip components
#include <app/MessageDef/AttributeDataIB.h>
// #include <lib/support/UnitTestContext.h>
// #include <lib/support/UnitTestRegistration.h>

// Third party library
#include <iostream>
#include <gtest/gtest.h>
#include <string>

#include "attribute_translator.hpp"

// Mocks
#include "MockNodeStateMonitor.hpp"
#include "MockUnifyMqtt.hpp"

using namespace unify::matter_bridge;

#define TEST_LOG_TAG "AttributeTranslatorTest"

static UnifyEmberInterface ember_interface = UnifyEmberInterface();
static device_translator dev_translator    = device_translator(false);
static ClusterEmulator emulator;

namespace chip {
namespace app {
namespace TestPath {
    
TEST(TestClusterTranslator, TestClusterTranslatorRevision)
{
    unify::matter_bridge::Test::MockNodeStateMonitor test_matter_node_state_monitor(dev_translator, emulator, ember_interface);
    unify::matter_bridge::Test::MockUnifyMqtt mqtt_publish_test;
    // testing Identify Cluster revision
    IdentifyAttributeAccess test_identify_attribute_handler(test_matter_node_state_monitor, mqtt_publish_test, dev_translator);
    const uint16_t endpoint                             = 4;
    chip::app::ConcreteReadAttributePath test_attr_path = chip::app::ConcreteReadAttributePath(
        endpoint, chip::app::Clusters::Identify::Id, chip::app::Clusters::Identify::Attributes::ClusterRevision::Id);

    chip::DataVersion dataVersion = 0;
    const AttributeEncodeState & aState = AttributeEncodeState();
    chip::app::AttributeReportIBs::Builder builder;
    Access::SubjectDescriptor subject;
    subject.fabricIndex = static_cast<uint8_t>(endpoint); 
    chip::app::AttributeValueEncoder encoder(builder, subject, test_attr_path, dataVersion, true, aState);
    uint8_t buf[4096];
    chip::TLV::TLVWriter writer;
    writer.Init(buf);
    chip::TLV::TLVType ignored;
    writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, ignored);
    builder.Init(&writer, 1);

    auto err = test_identify_attribute_handler.Read(test_attr_path, encoder);

    EXPECT_EQ(err,CHIP_NO_ERROR);

    writer.Finalize();
    writer.EndContainer(ignored);
}

}

}

}