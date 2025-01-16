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
#include <app/InteractionModelEngine.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include "command_translator_interface.hpp"
#include "matter.h"

// Mocks
#include "MockClusterCommandHandler.hpp"
#include "MockEventInteractionModel.hpp"


// Third party library
#include <gtest/gtest.h>


using namespace unify::matter_bridge;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::DoorLock;
using TestContext = unify::matter_bridge::Test::ClusterContext<DoorLockAttributeAccess,
                                unify::matter_bridge::Test::MockClusterCommandHandler>;
chip::EndpointId kTestEndpointId   = 2;
    
class TestDoorLock : public TestContext {
public:
void SetUp() {
    EXPECT_EQ(TestDoorLock::Initialize(this),1);
}

void TearDown() {
    EXPECT_EQ(TestDoorLock::Finalize(this),1);
}

static int Initialize(void * context)
{
    if (TestContext::Initialize(context, true) != SUCCESS)
        return FAILURE;

    auto * ctx         = static_cast<TestContext *>(context);
    auto & ep          = ctx->get_endpoint();
    auto & cluster     = ep.emplace_cluster("DoorLock");
    cluster.attributes = {
        "LockState",
        "LockType",
        "ActuatorEnabled",
        "OperatingMode",
        "SupportedOperatingModes", 
        "DoorState",
        "GeneratedCommandList",
        "AcceptedCommandList",
        "AttributeList",
        "FeatureMap",
        "ClusterRevision",
    };
    cluster.supported_commands = {
        "LockDoor", "UnlockDoor",
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
};

TEST_F(TestDoorLock, TestDoorLockAttributeLockState)
{
    CHIP_ERROR err    = attribute_test<Clusters::DoorLock::Attributes::LockState::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/LockState/Reported", R"({ "value": "Unlocked" })",
        MakeNullable(DlLockState::kUnlocked));
    EXPECT_EQ(err, CHIP_NO_ERROR);   
}

TEST_F(TestDoorLock, TestDoorLockAttributeLockType)
{   
    CHIP_ERROR err    = attribute_test<Clusters::DoorLock::Attributes::LockType::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/LockType/Reported", R"({ "value": "Magnetic" })",
        DlLockType::kMagnetic);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestDoorLock, TestDoorLockAttributeActuatorEnabled)
{   
    CHIP_ERROR err    = attribute_test<Clusters::DoorLock::Attributes::ActuatorEnabled::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/ActuatorEnabled/Reported", R"({ "value": true })",
        true);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestDoorLock, TestDoorLockAttributeOperatingMode)
{   
    CHIP_ERROR err    = attribute_test<Clusters::DoorLock::Attributes::OperatingMode::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/OperatingMode/Reported", R"({ "value": "Vacation" })",
        OperatingModeEnum::kVacation);
    EXPECT_EQ(err, CHIP_NO_ERROR);   
}

TEST_F(TestDoorLock, TestDoorLockAttributeSupportedOperatingModes)
{
    CHIP_ERROR err    = attribute_test<Clusters::DoorLock::Attributes::SupportedOperatingModes::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/SupportedOperatingModes/Reported", R"({ "value": { 
        "NoRFLockOrUnlockModeSupported": false, "NormalModeSupported": false, "PassageModeSupported": true,
        "PrivacyModeSupported": true, "VacationModeSupported": true }})", 22);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestDoorLock, TestDoorLockAttributeFeatureMap)
{   
    CHIP_ERROR err    = attribute_test<Clusters::DoorLock::Attributes::FeatureMap::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/FeatureMap/Reported", R"({ "value": 0 })",
        0);
    EXPECT_EQ(err, CHIP_NO_ERROR);    
}

TEST_F(TestDoorLock, TestDoorLockAttributeClusterRevision)
{   
    CHIP_ERROR err    = attribute_test<Clusters::DoorLock::Attributes::ClusterRevision::TypeInfo>(
        "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/ClusterRevision/Reported", R"({ "value": 7 })",
        7);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

// TEST_F(TestDoorLock, TestDoorLockEventErrorJammed)
// {   
//     CHIP_ERROR err    = CHIP_NO_ERROR;
//     Clusters::DoorLock::Events::DoorLockAlarm::DecodableType eventdata;
        
//     // Trigger Doorstate event.
//     err = attribute_test<Clusters::DoorLock::Attributes::DoorState::TypeInfo>(
//         "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/DoorState/Reported", R"({ "value": "ErrorJammed" })",
//         MakeNullable(DoorStateEnum::kDoorJammed));
//     EXPECT_EQ(err, CHIP_NO_ERROR);
    
//     err = event_test<Clusters::DoorLock::Events::DoorLockAlarm::DecodableType>(1, eventdata);
//     EXPECT_EQ(err, CHIP_NO_ERROR);
//     EXPECT_EQ(eventdata.alarmCode, Clusters::DoorLock::AlarmCodeEnum::kLockJammed);
//     DrainAndServiceIO();
// }

// TEST_F(TestDoorLock, TestDoorLockEventLocked)
// {
//     CHIP_ERROR err    = CHIP_NO_ERROR;
//     Clusters::DoorLock::Events::LockOperation::DecodableType eventdata;

//     // Trigger LockOperation Locked event
//     err = attribute_test<Clusters::DoorLock::Attributes::LockState::TypeInfo>(
//         "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/LockState/Reported", R"({ "value": "Locked" })",
//         MakeNullable(DlLockState::kLocked));
//     EXPECT_EQ(err, CHIP_NO_ERROR); 

//     err = event_test<Clusters::DoorLock::Events::LockOperation::DecodableType>(2, eventdata);
//     EXPECT_EQ(err, CHIP_NO_ERROR);
//     EXPECT_EQ(eventdata.lockOperationType, Clusters::DoorLock::LockOperationTypeEnum::kLock);
//     DrainAndServiceIO();
// }

// TEST_F(TestDoorLock, TestDoorLockEventUnlocked)
// {   
//     CHIP_ERROR err    = CHIP_NO_ERROR;
//     Clusters::DoorLock::Events::LockOperation::DecodableType eventdata;

//     // Trigger LockOperation Unlocked event
//     err = attribute_test<Clusters::DoorLock::Attributes::LockState::TypeInfo>(
//         "ucl/by-unid/zw-0x0002/ep2/DoorLock/Attributes/LockState/Reported", R"({ "value": "Unlocked" })",
//         MakeNullable(DlLockState::kUnlocked));
//     EXPECT_EQ(err, CHIP_NO_ERROR); 
    
//     err = event_test<Clusters::DoorLock::Events::LockOperation::DecodableType>(3, eventdata);
//     EXPECT_EQ(err, CHIP_NO_ERROR);
//     EXPECT_EQ(eventdata.lockOperationType, Clusters::DoorLock::LockOperationTypeEnum::kUnlock);
//     DrainAndServiceIO();
// }
