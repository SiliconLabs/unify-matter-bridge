/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include <app-common/zap-generated/ids/Clusters.h>
#include <map>
#include <string>
#include <vector>

typedef struct {
    const char* cluster_name;
    bool is_mandatory;
    std::vector<std::string> required_commands;
    std::vector<std::string> required_attributes;
} DeviceClusterData;

typedef struct {
    std::vector<DeviceClusterData> clusters;
    const char* device_type_name;
} DeviceTypeData;

// Device type IDs mapping
static const std::map<chip::DeviceTypeId, DeviceTypeData> matter_device_type_vs_clusters_map = {
    { 0x000A, { { { "DoorLock", true, {}, {} }, { "Identify", true, {
                                                                        "Identify",
                                                                    },
                                                    { "IdentifyTime", "IdentifyType" } },
                    { "TimeSynchronization", false, {}, {} } },
                  "doorlock" } },
    { 0x0100, { {
                    { "Groups", true, { "AddGroup", "ViewGroup", "GetGroupMembership", "RemoveGroup", "RemoveAllGroups", "AddGroupIfIdentifying" }, { "NameSupport" } },
                    { "Identify", true, {
                                            "Identify",
                                        },
                        { "IdentifyTime", "IdentifyType" } },
                    { "LevelControl", false, { "MoveToLevel", "Move", "Step", "Stop", "MoveToLevelWithOnOff", "MoveWithOnOff", "StepWithOnOff", "StopWithOnOff" }, { "CurrentLevel", "Options", "RemainingTime", "MinLevel", "OnLevel", "StartUpCurrentLevel" } },
                    { "OccupancySensing", false, {}, {} },
                    { "OnOff", true, { "Off", "On", "Toggle", "OffWithEffect", "OnWithRecallGlobalScene", "OnWithTimedOff" }, { "OnOff", "GlobalSceneControl", "OnTime", "OffWaitTime", "StartUpOnOff" } },
                },
                  "onofflight" } },
    { 0x0101, { {
                    { "Binding", false, {}, {} },
                    { "Groups", true, { "AddGroup", "ViewGroup", "GetGroupMembership", "RemoveGroup", "RemoveAllGroups", "AddGroupIfIdentifying" }, { "NameSupport" } },
                    { "Identify", true, {
                                            "Identify",
                                        },
                        { "IdentifyTime", "IdentifyType" } },
                    { "LevelControl", true, { "MoveToLevel", "Move", "Step", "Stop", "MoveToLevelWithOnOff", "MoveWithOnOff", "StepWithOnOff", "StopWithOnOff" }, { "CurrentLevel", "Options", "RemainingTime", "MinLevel", "OnLevel", "StartUpCurrentLevel" } },
                    { "OccupancySensing", false, {}, {} },
                    { "OnOff", true, { "Off", "On", "Toggle", "OffWithEffect", "OnWithRecallGlobalScene", "OnWithTimedOff" }, { "OnOff", "GlobalSceneControl", "OnTime", "OffWaitTime", "StartUpOnOff" } },
                },
                  "dimmablelight" } },
    { 0x0103, { {
                    { "Binding", true, {}, {} },
                    { "Groups", false, { "AddGroup", "ViewGroup", "GetGroupMembership", "RemoveGroup", "RemoveAllGroups", "AddGroupIfIdentifying" }, { "NameSupport" } },
                    { "Identify", true, {
                                            "Identify",
                                        },
                        { "IdentifyTime", "IdentifyType" } },
                    { "OnOff", true, {}, {} },
                },
                  "onofflightswitch" } },
    { 0x0106, { { { "Binding", false, {}, {} }, { "Groups", false, {}, {} }, { "Identify", true, {
                                                                                                     "Identify",
                                                                                                 },
                                                                                 { "IdentifyTime", "IdentifyType" } },
                    { "IlluminanceMeasurement", true, {}, { "MeasuredValue", "MinMeasuredValue", "MaxMeasuredValue" } } },
                  "lightsensor" } },
    { 0x0107, { { { "Binding", false, {}, {} }, { "Groups", false, {}, {} }, { "Identify", true, {
                                                                                                     "Identify",
                                                                                                 },
                                                                                 { "IdentifyTime", "IdentifyType" } },
                    { "OccupancySensing", true, {}, { "Occupancy", "OccupancySensorType" } } },
                  "occupancysensor" } },
    { 0x010A, { {
                    { "Groups", true, { "AddGroup", "ViewGroup", "GetGroupMembership", "RemoveGroup", "RemoveAllGroups", "AddGroupIfIdentifying" }, { "NameSupport" } },
                    { "Identify", true, {
                                            "Identify",
                                        },
                        { "IdentifyTime", "IdentifyType" } },
                    { "LevelControl", false, { "MoveToLevel", "Move", "Step", "Stop", "MoveToLevelWithOnOff", "MoveWithOnOff", "StepWithOnOff", "StopWithOnOff" }, { "CurrentLevel", "Options", "RemainingTime", "MinLevel", "OnLevel", "StartUpCurrentLevel" } },
                    { "OnOff", true, { "Off", "On", "Toggle", "OffWithEffect", "OnWithRecallGlobalScene", "OnWithTimedOff" }, { "OnOff", "GlobalSceneControl", "OnTime", "OffWaitTime", "StartUpOnOff" } },
                },
                  "onoffpluginunit" } },
    { 0x010B, { {
                    { "Groups", true, { "AddGroup", "ViewGroup", "GetGroupMembership", "RemoveGroup", "RemoveAllGroups", "AddGroupIfIdentifying" }, { "NameSupport" } },
                    { "Identify", true, {
                                            "Identify",
                                        },
                        { "IdentifyTime", "IdentifyType" } },
                    { "LevelControl", true, { "MoveToLevel", "Move", "Step", "Stop", "MoveToLevelWithOnOff", "MoveWithOnOff", "StepWithOnOff", "StopWithOnOff" }, { "CurrentLevel", "Options", "RemainingTime", "MinLevel", "OnLevel", "StartUpCurrentLevel" } },
                    { "OnOff", true, { "Off", "On", "Toggle", "OffWithEffect", "OnWithRecallGlobalScene", "OnWithTimedOff" }, { "OnOff", "GlobalSceneControl", "OnTime", "OffWaitTime", "StartUpOnOff" } },
                },
                  "dimmablepluginunit" } },
    { 0x010D, { {
                    { "ColorControl", true, { "MoveToColor", "MoveColor", "StepColor", "MoveToColorTemperature", "StopMoveStep", "MoveColorTemperature", "StepColorTemperature" }, { "RemainingTime", "CurrentX", "CurrentY", "ColorTemperatureMireds", "ColorMode", "Options", "NumberOfPrimaries", "EnhancedColorMode", "ColorCapabilities", "ColorTempPhysicalMinMireds", "ColorTempPhysicalMaxMireds", "CoupleColorTempToLevelMinMireds", "StartUpColorTemperatureMireds" } },
                    { "Groups", true, { "AddGroup", "ViewGroup", "GetGroupMembership", "RemoveGroup", "RemoveAllGroups", "AddGroupIfIdentifying" }, { "NameSupport" } },
                    { "Identify", true, {
                                            "Identify",
                                        },
                        { "IdentifyTime", "IdentifyType" } },
                    { "LevelControl", true, { "MoveToLevel", "Move", "Step", "Stop", "MoveToLevelWithOnOff", "MoveWithOnOff", "StepWithOnOff", "StopWithOnOff" }, { "CurrentLevel", "Options", "RemainingTime", "MinLevel", "OnLevel", "StartUpCurrentLevel" } },
                    { "OnOff", true, { "Off", "On", "Toggle", "OffWithEffect", "OnWithRecallGlobalScene", "OnWithTimedOff" }, { "OnOff", "GlobalSceneControl", "OnTime", "OffWaitTime", "StartUpOnOff" } },
                },
                  "extendedcolorlight" } },
    { 0x0202, { { { "Groups", false, { "AddGroup", "ViewGroup", "GetGroupMembership", "RemoveGroup", "RemoveAllGroups", "AddGroupIfIdentifying" }, { "NameSupport" } }, { "Identify", true, {
                                                                                                                                                                                                "Identify",
                                                                                                                                                                                            },
                                                                                                                                                                            { "IdentifyTime", "IdentifyType" } },
                    { "WindowCovering", true, {}, {} } },
                  "windowcovering" } },
    { 0x0301, { { { "Binding", false, {}, {} }, { "FanControl", false, {}, {} }, { "Groups", false, { "AddGroup", "ViewGroup", "GetGroupMembership", "RemoveGroup", "RemoveAllGroups", "AddGroupIfIdentifying" }, { "NameSupport" } }, { "Identify", true, {
                                                                                                                                                                                                                                                               "Identify",
                                                                                                                                                                                                                                                           },
                                                                                                                                                                                                                                           { "IdentifyTime", "IdentifyType" } },
                    { "OccupancySensing", false, {}, {} }, { "RelativeHumidityMeasurement", false, {}, {} }, { "TemperatureMeasurement", false, {}, {} }, { "Thermostat", true, {}, {} }, { "ThermostatUserInterfaceConfiguration", false, {}, {} }, { "TimeSynchronization", false, {}, {} } },
                  "thermostat" } },
    { 0x0305, { { { "Identify", true, {
                                          "Identify",
                                      },
                      { "IdentifyTime", "IdentifyType" } },
                    { "PressureMeasurement", true, {}, {} } },
                  "pressuresensor" } },
    { 0x0306, { { { "FlowMeasurement", true, {}, {} }, { "Identify", true, {
                                                                               "Identify",
                                                                           },
                                                           { "IdentifyTime", "IdentifyType" } } },
                  "flowsensor" } },
};