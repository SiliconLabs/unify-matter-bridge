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

#include <app-common/zap-generated/ids/Clusters.h>
#include <map>
#include <string>
#include <vector>

typedef struct {
    std::vector<std::string> additional_required_attributes;
} CustomClusterRequirementData;

static const std::map<std::string, CustomClusterRequirementData> custom_cluster_mandatory_command_attribute_custom_map = {
    { "WindowCovering", { { "CurrentPositionLiftPercentage", "CurrentPositionLift", "InstalledOpenLimitLift", "InstalledClosedLimitLift" } } }
};