/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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


// Third party library
#include <gtest/gtest.h>

using namespace unify::matter_bridge;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::DoorLock;
using TestContext = unify::matter_bridge::Test::ClusterContext<DoorLockAttributeAccess,DoorLockClusterCommandHandler>;

class TestDoorLockCommand : public TestContext {
public:
void SetUp() {
    EXPECT_EQ(TestDoorLockCommand::Initialize(this),1);
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
    auto & cluster     = ep.emplace_cluster("DoorLock");
    cluster.attributes = {
        "GeneratedCommandList",
        "AcceptedCommandList",
    };
    cluster.supported_commands = {
        "LockDoor", "UnlockDoor",
    };

    return ctx->register_endpoint(ep);
}
};

TEST_F(TestDoorLockCommand, TestDoorLockCommandLockDoor)
{
    Clusters::DoorLock::Commands::LockDoor::Type request;

    CHIP_ERROR err = command_test<Clusters::DoorLock::Commands::LockDoor::Type>(
        "ucl/by-unid/zw-0x0002/ep2/DoorLock/Commands/LockDoor", R"({ "PINOrRFIDCode": "" })", request, 50000);

    mqtt_subscribeCb("ucl/by-unid/zw-0x0002/ep2/DoorLock/GeneratedCommands/LockDoorResponse", R"({ "Status": "1" })");

    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();
}

TEST_F(TestDoorLockCommand, TestDoorLockCommandUnlockDoor)
{
    Clusters::DoorLock::Commands::UnlockDoor::Type request;
        
    CHIP_ERROR err = command_test<Clusters::DoorLock::Commands::UnlockDoor::Type>(
        "ucl/by-unid/zw-0x0002/ep2/DoorLock/Commands/UnlockDoor", R"({ "PINOrRFIDCode": "" })", request, 50000);

    mqtt_subscribeCb("ucl/by-unid/zw-0x0002/ep2/DoorLock/GeneratedCommands/UnlockDoorResponse", R"({ "Status": "1" })");
    
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();
}
