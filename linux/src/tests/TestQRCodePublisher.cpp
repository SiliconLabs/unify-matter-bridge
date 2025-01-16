// Unify bridge components

// Chip components
// #include <lib/support/UnitTestContext.h>
// #include <lib/support/UnitTestRegistration.h>
#include "UnifyBridgeContext.h"
// Third party library
#include <gtest/gtest.h>
#include "matter_bridge_qrcode_publisher.hpp"
#include "Options.h"
#include "TestHelpers.hpp"

using namespace unify::matter_bridge::Test;

class QRCode : public chip::Test::AppContext {  
};

TEST_F(QRCode,  QRCodePublishert)
{
  auto & opt = LinuxDeviceOptions::GetInstance();
  opt.payload.commissioningFlow = chip::CommissioningFlow::kStandard;
  opt.payload.rendezvousInformation.Emplace().ClearAll();
  opt.payload.rendezvousInformation.Emplace().Set(chip::RendezvousInformationFlag::kOnNetwork);
  opt.payload.productID = 1234;
  opt.payload.vendorID  = 5678;
  opt.payload.setUpPINCode = 0xfedeab;
  opt.discriminator =  chip::Optional<uint16_t>(0xe);

  auto ctx = new UnifyBridgeContext();
  unify::matter_bridge::QRCodePublisher publisher(ctx->mMqttHandler);

  publisher.publish();
  EXPECT_EQ(ctx->mMqttHandler.nNumerUicMqttPublishCall,1);
  EXPECT_EQ(ctx->mMqttHandler.publish_topic, "ucl/SmartStart/CommissionableDevice/MT:CYUK4CRM00S05-5FD00");
  EXPECT_EQ_JSON(ctx->mMqttHandler.publish_payload, R"({ "QRCode" : "MT:CYUK4CRM00S05-5FD00" })");


  publisher.unretain();
  EXPECT_EQ( ctx->mMqttHandler.nNumerUicMqttPublishCall , 2);
  EXPECT_EQ(ctx->mMqttHandler.publish_topic, "ucl/SmartStart/CommissionableDevice/MT:CYUK4CRM00S05-5FD00");
  EXPECT_EQ(ctx->mMqttHandler.publish_payload,"");
  
  delete ctx;

}