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

#define CHIP_USE_ENUM_CLASS_FOR_IM_ENUM

#include "matter.h"
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <type_traits>

#include "attribute_translator.hpp"
#include "cluster_emulator.hpp"
#include "matter_device_translator.hpp"
#include <attribute_state_cache.hpp>

#include "app-common/zap-generated/attributes/Accessors.h"
#include "sl_log.h"
#include "uic_mqtt.h"
#define LOG_TAG "attribute_translator"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace unify::matter_bridge;

#include "chip_types_from_json.hpp"
#include "chip_types_to_json.hpp"

CHIP_ERROR
IdentifyAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::Identify::Attributes;
    namespace UN = unify::matter_bridge::Identify::Attributes;
    if (aPath.mClusterId != Clusters::Identify::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::IdentifyTime::Id: { // type is int16u
            MN::IdentifyTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::IdentifyType::Id: { // type is IdentifyTypeEnum
            MN::IdentifyType::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR IdentifyAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::Identify;

    if (aPath.mClusterId != Clusters::Identify::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        auto err_Result = m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
        if (err_Result != CHIP_ERROR_IN_PROGRESS) {
            return err_Result;
        }
    }

    switch (aPath.mAttributeId) {
    case Attributes::IdentifyTime::Id: {

        Attributes::IdentifyTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["IdentifyTime"] = to_json(value);
        break;
    }
        // IdentifyType is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/Identify/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void IdentifyAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::Identify::Attributes;
    namespace UN = unify::matter_bridge::Identify::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::Identify::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::Identify::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is int16u
    case MN::IdentifyTime::Id: {
        using T = MN::IdentifyTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "IdentifyTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Identify::Id, MN::IdentifyTime::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
GroupsAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::Groups::Attributes;
    namespace UN = unify::matter_bridge::Groups::Attributes;
    if (aPath.mClusterId != Clusters::Groups::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::NameSupport::Id: { // type is NameSupportBitmap
            MN::NameSupport::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::Groups;

    if (aPath.mClusterId != Clusters::Groups::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        auto err_Result = m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
        if (err_Result != CHIP_ERROR_IN_PROGRESS) {
            return err_Result;
        }
    }

    switch (aPath.mAttributeId) {
        // NameSupport is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/Groups/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void GroupsAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::Groups::Attributes;
    namespace UN = unify::matter_bridge::Groups::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::Groups::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::Groups::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is NameSupportBitmap
    case MN::NameSupport::Id: {
        using T = MN::NameSupport::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NameSupport attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Groups::Id, MN::NameSupport::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
OnOffAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::OnOff::Attributes;
    namespace UN = unify::matter_bridge::OnOff::Attributes;
    if (aPath.mClusterId != Clusters::OnOff::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::OnOff::Id: { // type is boolean
            MN::OnOff::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::GlobalSceneControl::Id: { // type is boolean
            MN::GlobalSceneControl::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OnTime::Id: { // type is int16u
            MN::OnTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OffWaitTime::Id: { // type is int16u
            MN::OffWaitTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::StartUpOnOff::Id: { // type is StartUpOnOffEnum
            MN::StartUpOnOff::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OnOffAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::OnOff;

    if (aPath.mClusterId != Clusters::OnOff::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        auto err_Result = m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
        if (err_Result != CHIP_ERROR_IN_PROGRESS) {
            return err_Result;
        }
    }

    switch (aPath.mAttributeId) {
    // OnOff is not supported by UCL
    // GlobalSceneControl is not supported by UCL
    case Attributes::OnTime::Id: {

        Attributes::OnTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OnTime"] = to_json(value);
        break;
    }
    case Attributes::OffWaitTime::Id: {

        Attributes::OffWaitTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OffWaitTime"] = to_json(value);
        break;
    }
    case Attributes::StartUpOnOff::Id: {

        Attributes::StartUpOnOff::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["StartUpOnOff"] = to_json(value);
        break;
    }
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/OnOff/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void OnOffAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster, const std::string& attribute,
    const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::OnOff::Attributes;
    namespace UN = unify::matter_bridge::OnOff::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::OnOff::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::OnOff::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is boolean
    case MN::OnOff::Id: {
        using T = MN::OnOff::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OnOff attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::OnOff::Id);
        }
        break;
    }
        // type is boolean
    case MN::GlobalSceneControl::Id: {
        using T = MN::GlobalSceneControl::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "GlobalSceneControl attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::GlobalSceneControl::Id);
        }
        break;
    }
        // type is int16u
    case MN::OnTime::Id: {
        using T = MN::OnTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OnTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::OnTime::Id);
        }
        break;
    }
        // type is int16u
    case MN::OffWaitTime::Id: {
        using T = MN::OffWaitTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OffWaitTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::OffWaitTime::Id);
        }
        break;
    }
        // type is StartUpOnOffEnum
    case MN::StartUpOnOff::Id: {
        using T = MN::StartUpOnOff::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "StartUpOnOff attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OnOff::Id, MN::StartUpOnOff::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
LevelControlAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::LevelControl::Attributes;
    namespace UN = unify::matter_bridge::LevelControl::Attributes;
    if (aPath.mClusterId != Clusters::LevelControl::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::CurrentLevel::Id: { // type is int8u
            MN::CurrentLevel::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RemainingTime::Id: { // type is int16u
            MN::RemainingTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinLevel::Id: { // type is int8u
            MN::MinLevel::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxLevel::Id: { // type is int8u
            MN::MaxLevel::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CurrentFrequency::Id: { // type is int16u
            MN::CurrentFrequency::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinFrequency::Id: { // type is int16u
            MN::MinFrequency::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxFrequency::Id: { // type is int16u
            MN::MaxFrequency::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Options::Id: { // type is OptionsBitmap
            MN::Options::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OnOffTransitionTime::Id: { // type is int16u
            MN::OnOffTransitionTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OnLevel::Id: { // type is int8u
            MN::OnLevel::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OnTransitionTime::Id: { // type is int16u
            MN::OnTransitionTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OffTransitionTime::Id: { // type is int16u
            MN::OffTransitionTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DefaultMoveRate::Id: { // type is int8u
            MN::DefaultMoveRate::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::StartUpCurrentLevel::Id: { // type is int8u
            MN::StartUpCurrentLevel::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::LevelControl;

    if (aPath.mClusterId != Clusters::LevelControl::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        auto err_Result = m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
        if (err_Result != CHIP_ERROR_IN_PROGRESS) {
            return err_Result;
        }
    }

    switch (aPath.mAttributeId) {
    // CurrentLevel is not supported by UCL
    // RemainingTime is not supported by UCL
    // MinLevel is not supported by UCL
    // MaxLevel is not supported by UCL
    // CurrentFrequency is not supported by UCL
    // MinFrequency is not supported by UCL
    // MaxFrequency is not supported by UCL
    case Attributes::Options::Id: {

        Attributes::Options::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["Options"] = to_json(value);
        break;
    }
    case Attributes::OnOffTransitionTime::Id: {

        Attributes::OnOffTransitionTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OnOffTransitionTime"] = to_json(value);
        break;
    }
    case Attributes::OnLevel::Id: {

        Attributes::OnLevel::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OnLevel"] = to_json(value);
        break;
    }
    case Attributes::OnTransitionTime::Id: {

        Attributes::OnTransitionTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OnTransitionTime"] = to_json(value);
        break;
    }
    case Attributes::OffTransitionTime::Id: {

        Attributes::OffTransitionTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OffTransitionTime"] = to_json(value);
        break;
    }
    case Attributes::DefaultMoveRate::Id: {

        Attributes::DefaultMoveRate::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["DefaultMoveRate"] = to_json(value);
        break;
    }
    case Attributes::StartUpCurrentLevel::Id: {

        Attributes::StartUpCurrentLevel::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["StartUpCurrentLevel"] = to_json(value);
        break;
    }
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/Level/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void LevelControlAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::LevelControl::Attributes;
    namespace UN = unify::matter_bridge::LevelControl::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::LevelControl::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::LevelControl::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is int8u
    case MN::CurrentLevel::Id: {
        using T = MN::CurrentLevel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CurrentLevel attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::CurrentLevel::Id);
        }
        break;
    }
        // type is int16u
    case MN::RemainingTime::Id: {
        using T = MN::RemainingTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RemainingTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::RemainingTime::Id);
        }
        break;
    }
        // type is int8u
    case MN::MinLevel::Id: {
        using T = MN::MinLevel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinLevel attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::MinLevel::Id);
        }
        break;
    }
        // type is int8u
    case MN::MaxLevel::Id: {
        using T = MN::MaxLevel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxLevel attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::MaxLevel::Id);
        }
        break;
    }
        // type is int16u
    case MN::CurrentFrequency::Id: {
        using T = MN::CurrentFrequency::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CurrentFrequency attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::CurrentFrequency::Id);
        }
        break;
    }
        // type is int16u
    case MN::MinFrequency::Id: {
        using T = MN::MinFrequency::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinFrequency attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::MinFrequency::Id);
        }
        break;
    }
        // type is int16u
    case MN::MaxFrequency::Id: {
        using T = MN::MaxFrequency::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxFrequency attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::MaxFrequency::Id);
        }
        break;
    }
        // type is OptionsBitmap
    case MN::Options::Id: {
        using T = MN::Options::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Options attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::Options::Id);
        }
        break;
    }
        // type is int16u
    case MN::OnOffTransitionTime::Id: {
        using T = MN::OnOffTransitionTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OnOffTransitionTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::OnOffTransitionTime::Id);
        }
        break;
    }
        // type is int8u
    case MN::OnLevel::Id: {
        using T = MN::OnLevel::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OnLevel attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::OnLevel::Id);
        }
        break;
    }
        // type is int16u
    case MN::OnTransitionTime::Id: {
        using T = MN::OnTransitionTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OnTransitionTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::OnTransitionTime::Id);
        }
        break;
    }
        // type is int16u
    case MN::OffTransitionTime::Id: {
        using T = MN::OffTransitionTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OffTransitionTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::OffTransitionTime::Id);
        }
        break;
    }
        // type is int8u
    case MN::DefaultMoveRate::Id: {
        using T = MN::DefaultMoveRate::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DefaultMoveRate attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::DefaultMoveRate::Id);
        }
        break;
    }
        // type is int8u
    case MN::StartUpCurrentLevel::Id: {
        using T = MN::StartUpCurrentLevel::TypeInfo::Type;
        nlohmann::json modified_unify_value = unify_value;

        if (strcmp(modified_unify_value.dump().c_str(), "\"MinimumDeviceValuePermitted\"") == 0) {
            modified_unify_value = 0;
        } else if (strcmp(modified_unify_value.dump().c_str(), "\"SetToPreviousValue\"") == 0) {
            modified_unify_value = 0xFF;
        }

        std::optional<T> value = from_json<T>(modified_unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "StartUpCurrentLevel attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::LevelControl::Id, MN::StartUpCurrentLevel::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
DoorLockAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::DoorLock::Attributes;
    namespace UN = unify::matter_bridge::DoorLock::Attributes;
    if (aPath.mClusterId != Clusters::DoorLock::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::LockState::Id: { // type is DlLockState
            MN::LockState::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::LockType::Id: { // type is DlLockType
            MN::LockType::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActuatorEnabled::Id: { // type is boolean
            MN::ActuatorEnabled::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DoorState::Id: { // type is DoorStateEnum
            MN::DoorState::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DoorOpenEvents::Id: { // type is int32u
            MN::DoorOpenEvents::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DoorClosedEvents::Id: { // type is int32u
            MN::DoorClosedEvents::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OpenPeriod::Id: { // type is int16u
            MN::OpenPeriod::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfTotalUsersSupported::Id: { // type is int16u
            MN::NumberOfTotalUsersSupported::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfPINUsersSupported::Id: { // type is int16u
            MN::NumberOfPINUsersSupported::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfRFIDUsersSupported::Id: { // type is int16u
            MN::NumberOfRFIDUsersSupported::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfWeekDaySchedulesSupportedPerUser::Id: { // type is int8u
            MN::NumberOfWeekDaySchedulesSupportedPerUser::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfYearDaySchedulesSupportedPerUser::Id: { // type is int8u
            MN::NumberOfYearDaySchedulesSupportedPerUser::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfHolidaySchedulesSupported::Id: { // type is int8u
            MN::NumberOfHolidaySchedulesSupported::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxPINCodeLength::Id: { // type is int8u
            MN::MaxPINCodeLength::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinPINCodeLength::Id: { // type is int8u
            MN::MinPINCodeLength::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxRFIDCodeLength::Id: { // type is int8u
            MN::MaxRFIDCodeLength::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinRFIDCodeLength::Id: { // type is int8u
            MN::MinRFIDCodeLength::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CredentialRulesSupport::Id: { // type is DlCredentialRuleMask
            MN::CredentialRulesSupport::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfCredentialsSupportedPerUser::Id: { // type is int8u
            MN::NumberOfCredentialsSupportedPerUser::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Language::Id: { // type is char_string
            MN::Language::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::LEDSettings::Id: { // type is int8u
            MN::LEDSettings::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AutoRelockTime::Id: { // type is int32u
            MN::AutoRelockTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SoundVolume::Id: { // type is int8u
            MN::SoundVolume::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OperatingMode::Id: { // type is OperatingModeEnum
            MN::OperatingMode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SupportedOperatingModes::Id: { // type is DlSupportedOperatingModes
            MN::SupportedOperatingModes::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DefaultConfigurationRegister::Id: { // type is DlDefaultConfigurationRegister
            MN::DefaultConfigurationRegister::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::EnableLocalProgramming::Id: { // type is boolean
            MN::EnableLocalProgramming::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::EnableOneTouchLocking::Id: { // type is boolean
            MN::EnableOneTouchLocking::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::EnableInsideStatusLED::Id: { // type is boolean
            MN::EnableInsideStatusLED::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::EnablePrivacyModeButton::Id: { // type is boolean
            MN::EnablePrivacyModeButton::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::LocalProgrammingFeatures::Id: { // type is DlLocalProgrammingFeatures
            MN::LocalProgrammingFeatures::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::WrongCodeEntryLimit::Id: { // type is int8u
            MN::WrongCodeEntryLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UserCodeTemporaryDisableTime::Id: { // type is int8u
            MN::UserCodeTemporaryDisableTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SendPINOverTheAir::Id: { // type is boolean
            MN::SendPINOverTheAir::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RequirePINforRemoteOperation::Id: { // type is boolean
            MN::RequirePINforRemoteOperation::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ExpiringUserTimeout::Id: { // type is int16u
            MN::ExpiringUserTimeout::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AliroReaderVerificationKey::Id: { // type is octet_string
            MN::AliroReaderVerificationKey::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AliroReaderGroupIdentifier::Id: { // type is octet_string
            MN::AliroReaderGroupIdentifier::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AliroReaderGroupSubIdentifier::Id: { // type is octet_string
            MN::AliroReaderGroupSubIdentifier::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AliroGroupResolvingKey::Id: { // type is octet_string
            MN::AliroGroupResolvingKey::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AliroBLEAdvertisingVersion::Id: { // type is int8u
            MN::AliroBLEAdvertisingVersion::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfAliroCredentialIssuerKeysSupported::Id: { // type is int16u
            MN::NumberOfAliroCredentialIssuerKeysSupported::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfAliroEndpointKeysSupported::Id: { // type is int16u
            MN::NumberOfAliroEndpointKeysSupported::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DoorLockAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::DoorLock;

    if (aPath.mClusterId != Clusters::DoorLock::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        auto err_Result = m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
        if (err_Result != CHIP_ERROR_IN_PROGRESS) {
            return err_Result;
        }
    }

    switch (aPath.mAttributeId) {
    // LockState is not supported by UCL
    // LockType is not supported by UCL
    // ActuatorEnabled is not supported by UCL
    // DoorState is not supported by UCL
    case Attributes::DoorOpenEvents::Id: {

        Attributes::DoorOpenEvents::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["DoorOpenEvents"] = to_json(value);
        break;
    }
    case Attributes::DoorClosedEvents::Id: {

        Attributes::DoorClosedEvents::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["DoorClosedEvents"] = to_json(value);
        break;
    }
    case Attributes::OpenPeriod::Id: {

        Attributes::OpenPeriod::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OpenPeriod"] = to_json(value);
        break;
    }
    // NumberOfTotalUsersSupported is not supported by UCL
    // NumberOfPINUsersSupported is not supported by UCL
    // NumberOfRFIDUsersSupported is not supported by UCL
    // NumberOfWeekDaySchedulesSupportedPerUser is not supported by UCL
    // NumberOfYearDaySchedulesSupportedPerUser is not supported by UCL
    // NumberOfHolidaySchedulesSupported is not supported by UCL
    // MaxPINCodeLength is not supported by UCL
    // MinPINCodeLength is not supported by UCL
    // MaxRFIDCodeLength is not supported by UCL
    // MinRFIDCodeLength is not supported by UCL
    // CredentialRulesSupport is not supported by UCL
    // NumberOfCredentialsSupportedPerUser is not supported by UCL
    case Attributes::Language::Id: {

        Attributes::Language::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["Language"] = to_json(value);
        break;
    }
    case Attributes::LEDSettings::Id: {

        Attributes::LEDSettings::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["LEDSettings"] = to_json(value);
        break;
    }
    case Attributes::AutoRelockTime::Id: {

        Attributes::AutoRelockTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["AutoRelockTime"] = to_json(value);
        break;
    }
    case Attributes::SoundVolume::Id: {

        Attributes::SoundVolume::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["SoundVolume"] = to_json(value);
        break;
    }
    case Attributes::OperatingMode::Id: {

        Attributes::OperatingMode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OperatingMode"] = to_json(value);
        break;
    }
    // SupportedOperatingModes is not supported by UCL
    // DefaultConfigurationRegister is not supported by UCL
    case Attributes::EnableLocalProgramming::Id: {

        Attributes::EnableLocalProgramming::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["EnableLocalProgramming"] = to_json(value);
        break;
    }
    case Attributes::EnableOneTouchLocking::Id: {

        Attributes::EnableOneTouchLocking::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["EnableOneTouchLocking"] = to_json(value);
        break;
    }
    case Attributes::EnableInsideStatusLED::Id: {

        Attributes::EnableInsideStatusLED::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["EnableInsideStatusLED"] = to_json(value);
        break;
    }
    case Attributes::EnablePrivacyModeButton::Id: {

        Attributes::EnablePrivacyModeButton::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["EnablePrivacyModeButton"] = to_json(value);
        break;
    }
    case Attributes::LocalProgrammingFeatures::Id: {

        Attributes::LocalProgrammingFeatures::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["LocalProgrammingFeatures"] = to_json(value);
        break;
    }
    case Attributes::WrongCodeEntryLimit::Id: {

        Attributes::WrongCodeEntryLimit::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["WrongCodeEntryLimit"] = to_json(value);
        break;
    }
    case Attributes::UserCodeTemporaryDisableTime::Id: {

        Attributes::UserCodeTemporaryDisableTime::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["UserCodeTemporaryDisableTime"] = to_json(value);
        break;
    }
    case Attributes::SendPINOverTheAir::Id: {

        Attributes::SendPINOverTheAir::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["SendPINOverTheAir"] = to_json(value);
        break;
    }
    case Attributes::RequirePINforRemoteOperation::Id: {

        Attributes::RequirePINforRemoteOperation::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["RequirePINforRFOperation"] = to_json(value);
        break;
    }
    case Attributes::ExpiringUserTimeout::Id: {

        Attributes::ExpiringUserTimeout::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ExpiringUserTimeout"] = to_json(value);
        break;
    }
        // AliroReaderVerificationKey is not supported by UCL
        // AliroReaderGroupIdentifier is not supported by UCL
        // AliroReaderGroupSubIdentifier is not supported by UCL
        // AliroExpeditedTransactionSupportedProtocolVersions is not supported by UCL
        // AliroGroupResolvingKey is not supported by UCL
        // AliroSupportedBLEUWBProtocolVersions is not supported by UCL
        // AliroBLEAdvertisingVersion is not supported by UCL
        // NumberOfAliroCredentialIssuerKeysSupported is not supported by UCL
        // NumberOfAliroEndpointKeysSupported is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/DoorLock/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void DoorLockAttributeAccess::trigger_event(const bridged_endpoint* ep, const chip::EventId& eventid,
    const nlohmann::json& unify_value)
{
    namespace ME = chip::app::Clusters::DoorLock::Events;

    EventNumber eventNumber;
    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    bool event_valid = false;

    switch (eventid) {
    case ME::DoorLockAlarm::Id: {
        chip::app::Clusters::DoorLock::Events::DoorLockAlarm::Type event;

        if (strcmp(unify_value.dump().c_str(), "\"ErrorJammed\"") == 0) { // DoorJammed
            event.alarmCode = chip::app::Clusters::DoorLock::AlarmCodeEnum::kLockJammed;
            event_valid = true;
        }
        if (event_valid) {
            if (CHIP_NO_ERROR != LogEvent(event, node_matter_endpoint, eventNumber)) {
                sl_log_warning(LOG_TAG, "DoorLockAlarm: Failed to trigger event");
            }
        }
        break;
    }
    case ME::DoorStateChange::Id: {
        chip::app::Clusters::DoorLock::Events::DoorStateChange::Type event;

        if (event_valid) {
            if (CHIP_NO_ERROR != LogEvent(event, node_matter_endpoint, eventNumber)) {
                sl_log_warning(LOG_TAG, "DoorStateChange: Failed to trigger event");
            }
        }
        break;
    }
    case ME::LockOperation::Id: {
        chip::app::Clusters::DoorLock::Events::LockOperation::Type event;

        if (strcmp(unify_value.dump().c_str(), "\"Locked\"") == 0) { // Door locked event
            event.lockOperationType = chip::app::Clusters::DoorLock::LockOperationTypeEnum::kLock;
            event_valid = true;
        } else if (strcmp(unify_value.dump().c_str(), "\"Unlocked\"") == 0) { // Door unlocked event
            event.lockOperationType = chip::app::Clusters::DoorLock::LockOperationTypeEnum::kUnlock;
            event_valid = true;
        }
        if (event_valid) {
            if (CHIP_NO_ERROR != LogEvent(event, node_matter_endpoint, eventNumber)) {
                sl_log_warning(LOG_TAG, "LockOperation: Failed to trigger event");
            }
        }
        break;
    }
    case ME::LockOperationError::Id: {
        chip::app::Clusters::DoorLock::Events::LockOperationError::Type event;

        if (event_valid) {
            if (CHIP_NO_ERROR != LogEvent(event, node_matter_endpoint, eventNumber)) {
                sl_log_warning(LOG_TAG, "LockOperationError: Failed to trigger event");
            }
        }
        break;
    }
    case ME::LockUserChange::Id: {
        chip::app::Clusters::DoorLock::Events::LockUserChange::Type event;

        if (event_valid) {
            if (CHIP_NO_ERROR != LogEvent(event, node_matter_endpoint, eventNumber)) {
                sl_log_warning(LOG_TAG, "LockUserChange: Failed to trigger event");
            }
        }
        break;
    }
    }
}

void DoorLockAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::DoorLock::Attributes;
    namespace UN = unify::matter_bridge::DoorLock::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::DoorLock::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::DoorLock::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is DlLockState
    case MN::LockState::Id: {
        using T = MN::LockState::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "LockState attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::LockState::Id);
            trigger_event(ep, chip::app::Clusters::DoorLock::Events::LockOperation::Id, unify_value);
        }
        break;
    }
        // type is DlLockType
    case MN::LockType::Id: {
        using T = MN::LockType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "LockType attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::LockType::Id);
        }
        break;
    }
        // type is boolean
    case MN::ActuatorEnabled::Id: {
        using T = MN::ActuatorEnabled::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ActuatorEnabled attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::ActuatorEnabled::Id);
        }
        break;
    }
        // type is DoorStateEnum
    case MN::DoorState::Id: {
        using T = MN::DoorState::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DoorState attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::DoorState::Id);
            trigger_event(ep, chip::app::Clusters::DoorLock::Events::DoorLockAlarm::Id, unify_value);
        }
        break;
    }
        // type is int32u
    case MN::DoorOpenEvents::Id: {
        using T = MN::DoorOpenEvents::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DoorOpenEvents attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::DoorOpenEvents::Id);
        }
        break;
    }
        // type is int32u
    case MN::DoorClosedEvents::Id: {
        using T = MN::DoorClosedEvents::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DoorClosedEvents attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::DoorClosedEvents::Id);
        }
        break;
    }
        // type is int16u
    case MN::OpenPeriod::Id: {
        using T = MN::OpenPeriod::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OpenPeriod attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::OpenPeriod::Id);
        }
        break;
    }
        // type is int16u
    case MN::NumberOfTotalUsersSupported::Id: {
        using T = MN::NumberOfTotalUsersSupported::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfTotalUsersSupported attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::NumberOfTotalUsersSupported::Id);
        }
        break;
    }
        // type is int16u
    case MN::NumberOfPINUsersSupported::Id: {
        using T = MN::NumberOfPINUsersSupported::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfPINUsersSupported attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::NumberOfPINUsersSupported::Id);
        }
        break;
    }
        // type is int16u
    case MN::NumberOfRFIDUsersSupported::Id: {
        using T = MN::NumberOfRFIDUsersSupported::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfRFIDUsersSupported attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::NumberOfRFIDUsersSupported::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfWeekDaySchedulesSupportedPerUser::Id: {
        using T = MN::NumberOfWeekDaySchedulesSupportedPerUser::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfWeekDaySchedulesSupportedPerUser attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::NumberOfWeekDaySchedulesSupportedPerUser::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfYearDaySchedulesSupportedPerUser::Id: {
        using T = MN::NumberOfYearDaySchedulesSupportedPerUser::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfYearDaySchedulesSupportedPerUser attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::NumberOfYearDaySchedulesSupportedPerUser::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfHolidaySchedulesSupported::Id: {
        using T = MN::NumberOfHolidaySchedulesSupported::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfHolidaySchedulesSupported attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::NumberOfHolidaySchedulesSupported::Id);
        }
        break;
    }
        // type is int8u
    case MN::MaxPINCodeLength::Id: {
        using T = MN::MaxPINCodeLength::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxPINCodeLength attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::MaxPINCodeLength::Id);
        }
        break;
    }
        // type is int8u
    case MN::MinPINCodeLength::Id: {
        using T = MN::MinPINCodeLength::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinPINCodeLength attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::MinPINCodeLength::Id);
        }
        break;
    }
        // type is int8u
    case MN::MaxRFIDCodeLength::Id: {
        using T = MN::MaxRFIDCodeLength::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxRFIDCodeLength attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::MaxRFIDCodeLength::Id);
        }
        break;
    }
        // type is int8u
    case MN::MinRFIDCodeLength::Id: {
        using T = MN::MinRFIDCodeLength::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinRFIDCodeLength attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::MinRFIDCodeLength::Id);
        }
        break;
    }
        // type is DlCredentialRuleMask
    case MN::CredentialRulesSupport::Id: {
        using T = MN::CredentialRulesSupport::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CredentialRulesSupport attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::CredentialRulesSupport::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfCredentialsSupportedPerUser::Id: {
        using T = MN::NumberOfCredentialsSupportedPerUser::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfCredentialsSupportedPerUser attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::NumberOfCredentialsSupportedPerUser::Id);
        }
        break;
    }
        // type is char_string
    case MN::Language::Id: {
        using T = MN::Language::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Language attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::Language::Id);
        }
        break;
    }
        // type is int8u
    case MN::LEDSettings::Id: {
        using T = MN::LEDSettings::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "LEDSettings attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::LEDSettings::Id);
        }
        break;
    }
        // type is int32u
    case MN::AutoRelockTime::Id: {
        using T = MN::AutoRelockTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AutoRelockTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::AutoRelockTime::Id);
        }
        break;
    }
        // type is int8u
    case MN::SoundVolume::Id: {
        using T = MN::SoundVolume::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "SoundVolume attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::SoundVolume::Id);
        }
        break;
    }
        // type is OperatingModeEnum
    case MN::OperatingMode::Id: {
        using T = MN::OperatingMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OperatingMode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::OperatingMode::Id);
        }
        break;
    }
        // type is DlSupportedOperatingModes
    case MN::SupportedOperatingModes::Id: {
        using T = MN::SupportedOperatingModes::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "SupportedOperatingModes attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::SupportedOperatingModes::Id);
        }
        break;
    }
        // type is DlDefaultConfigurationRegister
    case MN::DefaultConfigurationRegister::Id: {
        using T = MN::DefaultConfigurationRegister::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DefaultConfigurationRegister attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::DefaultConfigurationRegister::Id);
        }
        break;
    }
        // type is boolean
    case MN::EnableLocalProgramming::Id: {
        using T = MN::EnableLocalProgramming::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "EnableLocalProgramming attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::EnableLocalProgramming::Id);
        }
        break;
    }
        // type is boolean
    case MN::EnableOneTouchLocking::Id: {
        using T = MN::EnableOneTouchLocking::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "EnableOneTouchLocking attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::EnableOneTouchLocking::Id);
        }
        break;
    }
        // type is boolean
    case MN::EnableInsideStatusLED::Id: {
        using T = MN::EnableInsideStatusLED::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "EnableInsideStatusLED attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::EnableInsideStatusLED::Id);
        }
        break;
    }
        // type is boolean
    case MN::EnablePrivacyModeButton::Id: {
        using T = MN::EnablePrivacyModeButton::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "EnablePrivacyModeButton attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::EnablePrivacyModeButton::Id);
        }
        break;
    }
        // type is DlLocalProgrammingFeatures
    case MN::LocalProgrammingFeatures::Id: {
        using T = MN::LocalProgrammingFeatures::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "LocalProgrammingFeatures attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::LocalProgrammingFeatures::Id);
        }
        break;
    }
        // type is int8u
    case MN::WrongCodeEntryLimit::Id: {
        using T = MN::WrongCodeEntryLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "WrongCodeEntryLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::WrongCodeEntryLimit::Id);
        }
        break;
    }
        // type is int8u
    case MN::UserCodeTemporaryDisableTime::Id: {
        using T = MN::UserCodeTemporaryDisableTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UserCodeTemporaryDisableTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::UserCodeTemporaryDisableTime::Id);
        }
        break;
    }
        // type is boolean
    case MN::SendPINOverTheAir::Id: {
        using T = MN::SendPINOverTheAir::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "SendPINOverTheAir attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::SendPINOverTheAir::Id);
        }
        break;
    }
        // type is boolean
    case MN::RequirePINforRemoteOperation::Id: {
        using T = MN::RequirePINforRemoteOperation::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RequirePINforRemoteOperation attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id,
                MN::RequirePINforRemoteOperation::Id);
        }
        break;
    }
        // type is int16u
    case MN::ExpiringUserTimeout::Id: {
        using T = MN::ExpiringUserTimeout::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ExpiringUserTimeout attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::ExpiringUserTimeout::Id);
        }
        break;
    }
        // type is bitmap32
    case MN::FeatureMap::Id: {
        using T = MN::FeatureMap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "FeatureMap attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::DoorLock::Id, MN::FeatureMap::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
WindowCoveringAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::WindowCovering::Attributes;
    namespace UN = unify::matter_bridge::WindowCovering::Attributes;
    if (aPath.mClusterId != Clusters::WindowCovering::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::Type::Id: { // type is Type
            MN::Type::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PhysicalClosedLimitLift::Id: { // type is int16u
            MN::PhysicalClosedLimitLift::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PhysicalClosedLimitTilt::Id: { // type is int16u
            MN::PhysicalClosedLimitTilt::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CurrentPositionLift::Id: { // type is int16u
            MN::CurrentPositionLift::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CurrentPositionTilt::Id: { // type is int16u
            MN::CurrentPositionTilt::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfActuationsLift::Id: { // type is int16u
            MN::NumberOfActuationsLift::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfActuationsTilt::Id: { // type is int16u
            MN::NumberOfActuationsTilt::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ConfigStatus::Id: { // type is ConfigStatus
            MN::ConfigStatus::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CurrentPositionLiftPercentage::Id: { // type is percent
            MN::CurrentPositionLiftPercentage::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CurrentPositionTiltPercentage::Id: { // type is percent
            MN::CurrentPositionTiltPercentage::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OperationalStatus::Id: { // type is OperationalStatus
            MN::OperationalStatus::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::TargetPositionLiftPercent100ths::Id: { // type is percent100ths
            MN::TargetPositionLiftPercent100ths::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::TargetPositionTiltPercent100ths::Id: { // type is percent100ths
            MN::TargetPositionTiltPercent100ths::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::EndProductType::Id: { // type is EndProductType
            MN::EndProductType::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CurrentPositionLiftPercent100ths::Id: { // type is percent100ths
            MN::CurrentPositionLiftPercent100ths::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CurrentPositionTiltPercent100ths::Id: { // type is percent100ths
            MN::CurrentPositionTiltPercent100ths::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::InstalledOpenLimitLift::Id: { // type is int16u
            MN::InstalledOpenLimitLift::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::InstalledClosedLimitLift::Id: { // type is int16u
            MN::InstalledClosedLimitLift::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::InstalledOpenLimitTilt::Id: { // type is int16u
            MN::InstalledOpenLimitTilt::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::InstalledClosedLimitTilt::Id: { // type is int16u
            MN::InstalledClosedLimitTilt::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Mode::Id: { // type is Mode
            MN::Mode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SafetyStatus::Id: { // type is SafetyStatus
            MN::SafetyStatus::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR WindowCoveringAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::WindowCovering;

    if (aPath.mClusterId != Clusters::WindowCovering::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        auto err_Result = m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
        if (err_Result != CHIP_ERROR_IN_PROGRESS) {
            return err_Result;
        }
    }

    switch (aPath.mAttributeId) {
    // Type is not supported by UCL
    // PhysicalClosedLimitLift is not supported by UCL
    // PhysicalClosedLimitTilt is not supported by UCL
    // CurrentPositionLift is not supported by UCL
    // CurrentPositionTilt is not supported by UCL
    // NumberOfActuationsLift is not supported by UCL
    // NumberOfActuationsTilt is not supported by UCL
    // ConfigStatus is not supported by UCL
    // CurrentPositionLiftPercentage is not supported by UCL
    // CurrentPositionTiltPercentage is not supported by UCL
    // OperationalStatus is not supported by UCL
    // TargetPositionLiftPercent100ths is not supported by UCL
    // TargetPositionTiltPercent100ths is not supported by UCL
    // EndProductType is not supported by UCL
    // CurrentPositionLiftPercent100ths is not supported by UCL
    // CurrentPositionTiltPercent100ths is not supported by UCL
    // InstalledOpenLimitLift is not supported by UCL
    // InstalledClosedLimitLift is not supported by UCL
    // InstalledOpenLimitTilt is not supported by UCL
    // InstalledClosedLimitTilt is not supported by UCL
    case Attributes::Mode::Id: {

        Attributes::Mode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["Mode"] = to_json(value);
        break;
    }
        // SafetyStatus is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/WindowCovering/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void WindowCoveringAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::WindowCovering::Attributes;
    namespace UN = unify::matter_bridge::WindowCovering::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::WindowCovering::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::WindowCovering::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is Type
    case MN::Type::Id: {
        using T = MN::Type::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Type attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id, MN::Type::Id);
        }
        break;
    }
        // type is int16u
    case MN::PhysicalClosedLimitLift::Id: {
        using T = MN::PhysicalClosedLimitLift::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PhysicalClosedLimitLift attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                MN::PhysicalClosedLimitLift::Id);
        }
        break;
    }
        // type is int16u
    case MN::PhysicalClosedLimitTilt::Id: {
        using T = MN::PhysicalClosedLimitTilt::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PhysicalClosedLimitTilt attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                MN::PhysicalClosedLimitTilt::Id);
        }
        break;
    }
        // type is int16u
    case MN::CurrentPositionLift::Id: {
        using T = MN::CurrentPositionLift::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            T updatedValue;
            updatedValue.SetNonNull(value->Value());

            if (value->Value() == std::numeric_limits<T::UnderlyingType>::max()) {
                updatedValue.SetNonNull(std::numeric_limits<T::UnderlyingType>::max() - 1);
            }
            if (!updatedValue.IsNull()) {
                sl_log_debug(LOG_TAG, "CurrentPositionLift attribute value is %u", updatedValue.Value());
            } else {
                sl_log_debug(LOG_TAG, "CurrentPositionLift attribute value is NULL");
            }

            value = updatedValue;
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id, MN::CurrentPositionLift::Id);
        }
        break;
    }
        // type is int16u
    case MN::CurrentPositionTilt::Id: {
        using T = MN::CurrentPositionTilt::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CurrentPositionTilt attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id, MN::CurrentPositionTilt::Id);
        }
        break;
    }
        // type is int16u
    case MN::NumberOfActuationsLift::Id: {
        using T = MN::NumberOfActuationsLift::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfActuationsLift attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                MN::NumberOfActuationsLift::Id);
        }
        break;
    }
        // type is int16u
    case MN::NumberOfActuationsTilt::Id: {
        using T = MN::NumberOfActuationsTilt::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfActuationsTilt attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                MN::NumberOfActuationsTilt::Id);
        }
        break;
    }
        // type is ConfigStatus
    case MN::ConfigStatus::Id: {
        using T = MN::ConfigStatus::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ConfigStatus attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id, MN::ConfigStatus::Id);
        }
        break;
    }
        // type is percent
    case MN::CurrentPositionLiftPercentage::Id: {
        using T = MN::CurrentPositionLiftPercentage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CurrentPositionLiftPercentage attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                MN::CurrentPositionLiftPercentage::Id);
        }
        break;
    }
        // type is percent
    case MN::CurrentPositionTiltPercentage::Id: {
        using T = MN::CurrentPositionTiltPercentage::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CurrentPositionTiltPercentage attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                MN::CurrentPositionTiltPercentage::Id);
        }
        break;
    }
        // type is int16u
    case MN::InstalledOpenLimitLift::Id: {
        using T = MN::InstalledOpenLimitLift::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            T maxValue = std::numeric_limits<T>::max();
            if (value.value() == maxValue) {
                value = maxValue - 1;
            }
            sl_log_debug(LOG_TAG, "InstalledOpenLimitLift attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                MN::InstalledOpenLimitLift::Id);
        }
        break;
    }
        // type is int16u
    case MN::InstalledClosedLimitLift::Id: {
        using T = MN::InstalledClosedLimitLift::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            T maxValue = std::numeric_limits<T>::max();
            if (value.value() == maxValue) {
                value = maxValue - 1;
            }
            sl_log_debug(LOG_TAG, "InstalledClosedLimitLift attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id,
                MN::InstalledClosedLimitLift::Id);
        }
        break;
    }
        // type is Mode
    case MN::Mode::Id: {
        using T = MN::Mode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Mode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::WindowCovering::Id, MN::Mode::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ThermostatAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::Thermostat::Attributes;
    namespace UN = unify::matter_bridge::Thermostat::Attributes;
    if (aPath.mClusterId != Clusters::Thermostat::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::LocalTemperature::Id: { // type is temperature
            MN::LocalTemperature::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OutdoorTemperature::Id: { // type is temperature
            MN::OutdoorTemperature::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Occupancy::Id: { // type is OccupancyBitmap
            MN::Occupancy::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AbsMinHeatSetpointLimit::Id: { // type is temperature
            MN::AbsMinHeatSetpointLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AbsMaxHeatSetpointLimit::Id: { // type is temperature
            MN::AbsMaxHeatSetpointLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AbsMinCoolSetpointLimit::Id: { // type is temperature
            MN::AbsMinCoolSetpointLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::AbsMaxCoolSetpointLimit::Id: { // type is temperature
            MN::AbsMaxCoolSetpointLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PICoolingDemand::Id: { // type is int8u
            MN::PICoolingDemand::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PIHeatingDemand::Id: { // type is int8u
            MN::PIHeatingDemand::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::HVACSystemTypeConfiguration::Id: { // type is HVACSystemTypeBitmap
            MN::HVACSystemTypeConfiguration::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::LocalTemperatureCalibration::Id: { // type is int8s
            MN::LocalTemperatureCalibration::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OccupiedCoolingSetpoint::Id: { // type is temperature
            MN::OccupiedCoolingSetpoint::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OccupiedHeatingSetpoint::Id: { // type is temperature
            MN::OccupiedHeatingSetpoint::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UnoccupiedCoolingSetpoint::Id: { // type is temperature
            MN::UnoccupiedCoolingSetpoint::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UnoccupiedHeatingSetpoint::Id: { // type is temperature
            MN::UnoccupiedHeatingSetpoint::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinHeatSetpointLimit::Id: { // type is temperature
            MN::MinHeatSetpointLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxHeatSetpointLimit::Id: { // type is temperature
            MN::MaxHeatSetpointLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinCoolSetpointLimit::Id: { // type is temperature
            MN::MinCoolSetpointLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxCoolSetpointLimit::Id: { // type is temperature
            MN::MaxCoolSetpointLimit::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinSetpointDeadBand::Id: { // type is int8s
            MN::MinSetpointDeadBand::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RemoteSensing::Id: { // type is RemoteSensingBitmap
            MN::RemoteSensing::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ControlSequenceOfOperation::Id: { // type is ControlSequenceOfOperationEnum
            MN::ControlSequenceOfOperation::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SystemMode::Id: { // type is SystemModeEnum
            MN::SystemMode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ThermostatRunningMode::Id: { // type is ThermostatRunningModeEnum
            MN::ThermostatRunningMode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::StartOfWeek::Id: { // type is StartOfWeekEnum
            MN::StartOfWeek::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfWeeklyTransitions::Id: { // type is int8u
            MN::NumberOfWeeklyTransitions::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfDailyTransitions::Id: { // type is int8u
            MN::NumberOfDailyTransitions::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::TemperatureSetpointHold::Id: { // type is TemperatureSetpointHoldEnum
            MN::TemperatureSetpointHold::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::TemperatureSetpointHoldDuration::Id: { // type is int16u
            MN::TemperatureSetpointHoldDuration::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ThermostatProgrammingOperationMode::Id: { // type is ProgrammingOperationModeBitmap
            MN::ThermostatProgrammingOperationMode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ThermostatRunningState::Id: { // type is RelayStateBitmap
            MN::ThermostatRunningState::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SetpointChangeSource::Id: { // type is SetpointChangeSourceEnum
            MN::SetpointChangeSource::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SetpointChangeAmount::Id: { // type is int16s
            MN::SetpointChangeAmount::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SetpointChangeSourceTimestamp::Id: { // type is epoch_s
            MN::SetpointChangeSourceTimestamp::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OccupiedSetback::Id: { // type is int8u
            MN::OccupiedSetback::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OccupiedSetbackMin::Id: { // type is int8u
            MN::OccupiedSetbackMin::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OccupiedSetbackMax::Id: { // type is int8u
            MN::OccupiedSetbackMax::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UnoccupiedSetback::Id: { // type is int8u
            MN::UnoccupiedSetback::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UnoccupiedSetbackMin::Id: { // type is int8u
            MN::UnoccupiedSetbackMin::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UnoccupiedSetbackMax::Id: { // type is int8u
            MN::UnoccupiedSetbackMax::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::EmergencyHeatDelta::Id: { // type is int8u
            MN::EmergencyHeatDelta::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ACType::Id: { // type is ACTypeEnum
            MN::ACType::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ACCapacity::Id: { // type is int16u
            MN::ACCapacity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ACRefrigerantType::Id: { // type is ACRefrigerantTypeEnum
            MN::ACRefrigerantType::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ACCompressorType::Id: { // type is ACCompressorTypeEnum
            MN::ACCompressorType::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ACErrorCode::Id: { // type is ACErrorCodeBitmap
            MN::ACErrorCode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ACLouverPosition::Id: { // type is ACLouverPositionEnum
            MN::ACLouverPosition::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ACCoilTemperature::Id: { // type is temperature
            MN::ACCoilTemperature::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ACCapacityformat::Id: { // type is ACCapacityFormatEnum
            MN::ACCapacityformat::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfPresets::Id: { // type is int8u
            MN::NumberOfPresets::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfSchedules::Id: { // type is int8u
            MN::NumberOfSchedules::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfScheduleTransitions::Id: { // type is int8u
            MN::NumberOfScheduleTransitions::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfScheduleTransitionPerDay::Id: { // type is int8u
            MN::NumberOfScheduleTransitionPerDay::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActivePresetHandle::Id: { // type is octet_string
            MN::ActivePresetHandle::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ActiveScheduleHandle::Id: { // type is octet_string
            MN::ActiveScheduleHandle::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::SetpointHoldExpiryTimestamp::Id: { // type is epoch_s
            MN::SetpointHoldExpiryTimestamp::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::Thermostat;

    if (aPath.mClusterId != Clusters::Thermostat::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        auto err_Result = m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
        if (err_Result != CHIP_ERROR_IN_PROGRESS) {
            return err_Result;
        }
    }

    switch (aPath.mAttributeId) {
    // LocalTemperature is not supported by UCL
    // OutdoorTemperature is not supported by UCL
    // Occupancy is not supported by UCL
    // AbsMinHeatSetpointLimit is not supported by UCL
    // AbsMaxHeatSetpointLimit is not supported by UCL
    // AbsMinCoolSetpointLimit is not supported by UCL
    // AbsMaxCoolSetpointLimit is not supported by UCL
    // PICoolingDemand is not supported by UCL
    // PIHeatingDemand is not supported by UCL
    case Attributes::HVACSystemTypeConfiguration::Id: {

        Attributes::HVACSystemTypeConfiguration::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["HVACSystemTypeConfiguration"] = to_json(value);
        break;
    }
    case Attributes::LocalTemperatureCalibration::Id: {

        Attributes::LocalTemperatureCalibration::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["LocalTemperatureCalibration"] = to_json(value);
        break;
    }
    case Attributes::OccupiedCoolingSetpoint::Id: {

        Attributes::OccupiedCoolingSetpoint::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OccupiedCoolingSetpoint"] = to_json(value);
        break;
    }
    case Attributes::OccupiedHeatingSetpoint::Id: {

        Attributes::OccupiedHeatingSetpoint::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OccupiedHeatingSetpoint"] = to_json(value);
        break;
    }
    case Attributes::UnoccupiedCoolingSetpoint::Id: {

        Attributes::UnoccupiedCoolingSetpoint::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["UnoccupiedCoolingSetpoint"] = to_json(value);
        break;
    }
    case Attributes::UnoccupiedHeatingSetpoint::Id: {

        Attributes::UnoccupiedHeatingSetpoint::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["UnoccupiedHeatingSetpoint"] = to_json(value);
        break;
    }
    case Attributes::MinHeatSetpointLimit::Id: {

        Attributes::MinHeatSetpointLimit::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["MinHeatSetpointLimit"] = to_json(value);
        break;
    }
    case Attributes::MaxHeatSetpointLimit::Id: {

        Attributes::MaxHeatSetpointLimit::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["MaxHeatSetpointLimit"] = to_json(value);
        break;
    }
    case Attributes::MinCoolSetpointLimit::Id: {

        Attributes::MinCoolSetpointLimit::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["MinCoolSetpointLimit"] = to_json(value);
        break;
    }
    case Attributes::MaxCoolSetpointLimit::Id: {

        Attributes::MaxCoolSetpointLimit::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["MaxCoolSetpointLimit"] = to_json(value);
        break;
    }
    case Attributes::MinSetpointDeadBand::Id: {

        Attributes::MinSetpointDeadBand::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["MinSetpointDeadBand"] = to_json(value);
        break;
    }
    case Attributes::RemoteSensing::Id: {

        Attributes::RemoteSensing::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["RemoteSensing"] = to_json(value);
        break;
    }
    case Attributes::ControlSequenceOfOperation::Id: {

        Attributes::ControlSequenceOfOperation::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ControlSequenceOfOperation"] = to_json(value);
        break;
    }
    case Attributes::SystemMode::Id: {

        Attributes::SystemMode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["SystemMode"] = to_json(value);
        break;
    }
    // ThermostatRunningMode is not supported by UCL
    // StartOfWeek is not supported by UCL
    // NumberOfWeeklyTransitions is not supported by UCL
    // NumberOfDailyTransitions is not supported by UCL
    case Attributes::TemperatureSetpointHold::Id: {

        Attributes::TemperatureSetpointHold::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["TemperatureSetpointHold"] = to_json(value);
        break;
    }
    case Attributes::TemperatureSetpointHoldDuration::Id: {

        Attributes::TemperatureSetpointHoldDuration::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["TemperatureSetpointHoldDuration"] = to_json(value);
        break;
    }
    case Attributes::ThermostatProgrammingOperationMode::Id: {

        Attributes::ThermostatProgrammingOperationMode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ThermostatProgrammingOperationMode"] = to_json(value);
        break;
    }
    // ThermostatRunningState is not supported by UCL
    // SetpointChangeSource is not supported by UCL
    // SetpointChangeAmount is not supported by UCL
    // SetpointChangeSourceTimestamp is not supported by UCL
    case Attributes::OccupiedSetback::Id: {

        Attributes::OccupiedSetback::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["OccupiedSetback"] = to_json(value);
        break;
    }
    // OccupiedSetbackMin is not supported by UCL
    // OccupiedSetbackMax is not supported by UCL
    case Attributes::UnoccupiedSetback::Id: {

        Attributes::UnoccupiedSetback::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["UnoccupiedSetback"] = to_json(value);
        break;
    }
    // UnoccupiedSetbackMin is not supported by UCL
    // UnoccupiedSetbackMax is not supported by UCL
    case Attributes::EmergencyHeatDelta::Id: {

        Attributes::EmergencyHeatDelta::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["EmergencyHeatDelta"] = to_json(value);
        break;
    }
    case Attributes::ACType::Id: {

        Attributes::ACType::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ACType"] = to_json(value);
        break;
    }
    case Attributes::ACCapacity::Id: {

        Attributes::ACCapacity::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ACCapacity"] = to_json(value);
        break;
    }
    case Attributes::ACRefrigerantType::Id: {

        Attributes::ACRefrigerantType::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ACRefrigerantType"] = to_json(value);
        break;
    }
    case Attributes::ACCompressorType::Id: {

        Attributes::ACCompressorType::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ACCompressorType"] = to_json(value);
        break;
    }
    case Attributes::ACErrorCode::Id: {

        Attributes::ACErrorCode::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ACErrorCode"] = to_json(value);
        break;
    }
    case Attributes::ACLouverPosition::Id: {

        Attributes::ACLouverPosition::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ACLouverPosition"] = to_json(value);
        break;
    }
    // ACCoilTemperature is not supported by UCL
    case Attributes::ACCapacityformat::Id: {

        Attributes::ACCapacityformat::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ACCapacityFormat"] = to_json(value);
        break;
    }
        // PresetTypes is not supported by UCL
        // ScheduleTypes is not supported by UCL
        // NumberOfPresets is not supported by UCL
        // NumberOfSchedules is not supported by UCL
        // NumberOfScheduleTransitions is not supported by UCL
        // NumberOfScheduleTransitionPerDay is not supported by UCL
        // ActivePresetHandle is not supported by UCL
        // ActiveScheduleHandle is not supported by UCL
        // SetpointHoldExpiryTimestamp is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/Thermostat/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void ThermostatAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::Thermostat::Attributes;
    namespace UN = unify::matter_bridge::Thermostat::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::Thermostat::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::Thermostat::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is temperature
    case MN::LocalTemperature::Id: {
        using T = MN::LocalTemperature::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "LocalTemperature attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::LocalTemperature::Id);
        }
        break;
    }
        // type is temperature
    case MN::OutdoorTemperature::Id: {
        using T = MN::OutdoorTemperature::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OutdoorTemperature attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OutdoorTemperature::Id);
        }
        break;
    }
        // type is OccupancyBitmap
    case MN::Occupancy::Id: {
        using T = MN::Occupancy::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Occupancy attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::Occupancy::Id);
        }
        break;
    }
        // type is temperature
    case MN::AbsMinHeatSetpointLimit::Id: {
        using T = MN::AbsMinHeatSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AbsMinHeatSetpointLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::AbsMinHeatSetpointLimit::Id);
        }
        break;
    }
        // type is temperature
    case MN::AbsMaxHeatSetpointLimit::Id: {
        using T = MN::AbsMaxHeatSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AbsMaxHeatSetpointLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::AbsMaxHeatSetpointLimit::Id);
        }
        break;
    }
        // type is temperature
    case MN::AbsMinCoolSetpointLimit::Id: {
        using T = MN::AbsMinCoolSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AbsMinCoolSetpointLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::AbsMinCoolSetpointLimit::Id);
        }
        break;
    }
        // type is temperature
    case MN::AbsMaxCoolSetpointLimit::Id: {
        using T = MN::AbsMaxCoolSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "AbsMaxCoolSetpointLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::AbsMaxCoolSetpointLimit::Id);
        }
        break;
    }
        // type is int8u
    case MN::PICoolingDemand::Id: {
        using T = MN::PICoolingDemand::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PICoolingDemand attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::PICoolingDemand::Id);
        }
        break;
    }
        // type is int8u
    case MN::PIHeatingDemand::Id: {
        using T = MN::PIHeatingDemand::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PIHeatingDemand attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::PIHeatingDemand::Id);
        }
        break;
    }
        // type is HVACSystemTypeBitmap
    case MN::HVACSystemTypeConfiguration::Id: {
        using T = MN::HVACSystemTypeConfiguration::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "HVACSystemTypeConfiguration attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::HVACSystemTypeConfiguration::Id);
        }
        break;
    }
        // type is int8s
    case MN::LocalTemperatureCalibration::Id: {
        using T = MN::LocalTemperatureCalibration::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "LocalTemperatureCalibration attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::LocalTemperatureCalibration::Id);
        }
        break;
    }
        // type is temperature
    case MN::OccupiedCoolingSetpoint::Id: {
        using T = MN::OccupiedCoolingSetpoint::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OccupiedCoolingSetpoint attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OccupiedCoolingSetpoint::Id);
        }
        break;
    }
        // type is temperature
    case MN::OccupiedHeatingSetpoint::Id: {
        using T = MN::OccupiedHeatingSetpoint::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OccupiedHeatingSetpoint attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OccupiedHeatingSetpoint::Id);
        }
        break;
    }
        // type is temperature
    case MN::UnoccupiedCoolingSetpoint::Id: {
        using T = MN::UnoccupiedCoolingSetpoint::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UnoccupiedCoolingSetpoint attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::UnoccupiedCoolingSetpoint::Id);
        }
        break;
    }
        // type is temperature
    case MN::UnoccupiedHeatingSetpoint::Id: {
        using T = MN::UnoccupiedHeatingSetpoint::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UnoccupiedHeatingSetpoint attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::UnoccupiedHeatingSetpoint::Id);
        }
        break;
    }
        // type is temperature
    case MN::MinHeatSetpointLimit::Id: {
        using T = MN::MinHeatSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinHeatSetpointLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::MinHeatSetpointLimit::Id);
        }
        break;
    }
        // type is temperature
    case MN::MaxHeatSetpointLimit::Id: {
        using T = MN::MaxHeatSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxHeatSetpointLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::MaxHeatSetpointLimit::Id);
        }
        break;
    }
        // type is temperature
    case MN::MinCoolSetpointLimit::Id: {
        using T = MN::MinCoolSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinCoolSetpointLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::MinCoolSetpointLimit::Id);
        }
        break;
    }
        // type is temperature
    case MN::MaxCoolSetpointLimit::Id: {
        using T = MN::MaxCoolSetpointLimit::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxCoolSetpointLimit attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::MaxCoolSetpointLimit::Id);
        }
        break;
    }
        // type is int8s
    case MN::MinSetpointDeadBand::Id: {
        using T = MN::MinSetpointDeadBand::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinSetpointDeadBand attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::MinSetpointDeadBand::Id);
        }
        break;
    }
        // type is RemoteSensingBitmap
    case MN::RemoteSensing::Id: {
        using T = MN::RemoteSensing::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RemoteSensing attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::RemoteSensing::Id);
        }
        break;
    }
        // type is ControlSequenceOfOperationEnum
    case MN::ControlSequenceOfOperation::Id: {
        using T = MN::ControlSequenceOfOperation::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ControlSequenceOfOperation attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::ControlSequenceOfOperation::Id);
        }
        break;
    }
        // type is SystemModeEnum
    case MN::SystemMode::Id: {
        using T = MN::SystemMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "SystemMode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::SystemMode::Id);
        }
        break;
    }
        // type is ThermostatRunningModeEnum
    case MN::ThermostatRunningMode::Id: {
        using T = MN::ThermostatRunningMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ThermostatRunningMode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ThermostatRunningMode::Id);
        }
        break;
    }
        // type is StartOfWeekEnum
    case MN::StartOfWeek::Id: {
        using T = MN::StartOfWeek::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "StartOfWeek attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::StartOfWeek::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfWeeklyTransitions::Id: {
        using T = MN::NumberOfWeeklyTransitions::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfWeeklyTransitions attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::NumberOfWeeklyTransitions::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfDailyTransitions::Id: {
        using T = MN::NumberOfDailyTransitions::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfDailyTransitions attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::NumberOfDailyTransitions::Id);
        }
        break;
    }
        // type is TemperatureSetpointHoldEnum
    case MN::TemperatureSetpointHold::Id: {
        using T = MN::TemperatureSetpointHold::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "TemperatureSetpointHold attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::TemperatureSetpointHold::Id);
        }
        break;
    }
        // type is int16u
    case MN::TemperatureSetpointHoldDuration::Id: {
        using T = MN::TemperatureSetpointHoldDuration::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "TemperatureSetpointHoldDuration attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::TemperatureSetpointHoldDuration::Id);
        }
        break;
    }
        // type is ProgrammingOperationModeBitmap
    case MN::ThermostatProgrammingOperationMode::Id: {
        using T = MN::ThermostatProgrammingOperationMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ThermostatProgrammingOperationMode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::ThermostatProgrammingOperationMode::Id);
        }
        break;
    }
        // type is RelayStateBitmap
    case MN::ThermostatRunningState::Id: {
        using T = MN::ThermostatRunningState::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ThermostatRunningState attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ThermostatRunningState::Id);
        }
        break;
    }
        // type is SetpointChangeSourceEnum
    case MN::SetpointChangeSource::Id: {
        using T = MN::SetpointChangeSource::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "SetpointChangeSource attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::SetpointChangeSource::Id);
        }
        break;
    }
        // type is int16s
    case MN::SetpointChangeAmount::Id: {
        using T = MN::SetpointChangeAmount::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "SetpointChangeAmount attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::SetpointChangeAmount::Id);
        }
        break;
    }
        // type is epoch_s
    case MN::SetpointChangeSourceTimestamp::Id: {
        using T = MN::SetpointChangeSourceTimestamp::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "SetpointChangeSourceTimestamp attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id,
                MN::SetpointChangeSourceTimestamp::Id);
        }
        break;
    }
        // type is int8u
    case MN::OccupiedSetback::Id: {
        using T = MN::OccupiedSetback::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OccupiedSetback attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OccupiedSetback::Id);
        }
        break;
    }
        // type is int8u
    case MN::OccupiedSetbackMin::Id: {
        using T = MN::OccupiedSetbackMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OccupiedSetbackMin attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OccupiedSetbackMin::Id);
        }
        break;
    }
        // type is int8u
    case MN::OccupiedSetbackMax::Id: {
        using T = MN::OccupiedSetbackMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OccupiedSetbackMax attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::OccupiedSetbackMax::Id);
        }
        break;
    }
        // type is int8u
    case MN::UnoccupiedSetback::Id: {
        using T = MN::UnoccupiedSetback::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UnoccupiedSetback attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::UnoccupiedSetback::Id);
        }
        break;
    }
        // type is int8u
    case MN::UnoccupiedSetbackMin::Id: {
        using T = MN::UnoccupiedSetbackMin::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UnoccupiedSetbackMin attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::UnoccupiedSetbackMin::Id);
        }
        break;
    }
        // type is int8u
    case MN::UnoccupiedSetbackMax::Id: {
        using T = MN::UnoccupiedSetbackMax::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UnoccupiedSetbackMax attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::UnoccupiedSetbackMax::Id);
        }
        break;
    }
        // type is int8u
    case MN::EmergencyHeatDelta::Id: {
        using T = MN::EmergencyHeatDelta::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "EmergencyHeatDelta attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::EmergencyHeatDelta::Id);
        }
        break;
    }
        // type is ACTypeEnum
    case MN::ACType::Id: {
        using T = MN::ACType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ACType attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACType::Id);
        }
        break;
    }
        // type is int16u
    case MN::ACCapacity::Id: {
        using T = MN::ACCapacity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ACCapacity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACCapacity::Id);
        }
        break;
    }
        // type is ACRefrigerantTypeEnum
    case MN::ACRefrigerantType::Id: {
        using T = MN::ACRefrigerantType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ACRefrigerantType attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACRefrigerantType::Id);
        }
        break;
    }
        // type is ACCompressorTypeEnum
    case MN::ACCompressorType::Id: {
        using T = MN::ACCompressorType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ACCompressorType attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACCompressorType::Id);
        }
        break;
    }
        // type is ACErrorCodeBitmap
    case MN::ACErrorCode::Id: {
        using T = MN::ACErrorCode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ACErrorCode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACErrorCode::Id);
        }
        break;
    }
        // type is ACLouverPositionEnum
    case MN::ACLouverPosition::Id: {
        using T = MN::ACLouverPosition::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ACLouverPosition attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACLouverPosition::Id);
        }
        break;
    }
        // type is temperature
    case MN::ACCoilTemperature::Id: {
        using T = MN::ACCoilTemperature::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ACCoilTemperature attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACCoilTemperature::Id);
        }
        break;
    }
        // type is ACCapacityFormatEnum
    case MN::ACCapacityformat::Id: {
        using T = MN::ACCapacityformat::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ACCapacityformat attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::Thermostat::Id, MN::ACCapacityformat::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
ColorControlAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::ColorControl::Attributes;
    namespace UN = unify::matter_bridge::ColorControl::Attributes;
    if (aPath.mClusterId != Clusters::ColorControl::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::CurrentHue::Id: { // type is int8u
            MN::CurrentHue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CurrentSaturation::Id: { // type is int8u
            MN::CurrentSaturation::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::RemainingTime::Id: { // type is int16u
            MN::RemainingTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CurrentX::Id: { // type is int16u
            MN::CurrentX::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CurrentY::Id: { // type is int16u
            MN::CurrentY::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::DriftCompensation::Id: { // type is DriftCompensationEnum
            MN::DriftCompensation::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CompensationText::Id: { // type is char_string
            MN::CompensationText::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorTemperatureMireds::Id: { // type is int16u
            MN::ColorTemperatureMireds::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorMode::Id: { // type is ColorModeEnum
            MN::ColorMode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Options::Id: { // type is OptionsBitmap
            MN::Options::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::NumberOfPrimaries::Id: { // type is int8u
            MN::NumberOfPrimaries::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary1X::Id: { // type is int16u
            MN::Primary1X::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary1Y::Id: { // type is int16u
            MN::Primary1Y::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary1Intensity::Id: { // type is int8u
            MN::Primary1Intensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary2X::Id: { // type is int16u
            MN::Primary2X::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary2Y::Id: { // type is int16u
            MN::Primary2Y::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary2Intensity::Id: { // type is int8u
            MN::Primary2Intensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary3X::Id: { // type is int16u
            MN::Primary3X::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary3Y::Id: { // type is int16u
            MN::Primary3Y::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary3Intensity::Id: { // type is int8u
            MN::Primary3Intensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary4X::Id: { // type is int16u
            MN::Primary4X::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary4Y::Id: { // type is int16u
            MN::Primary4Y::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary4Intensity::Id: { // type is int8u
            MN::Primary4Intensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary5X::Id: { // type is int16u
            MN::Primary5X::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary5Y::Id: { // type is int16u
            MN::Primary5Y::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary5Intensity::Id: { // type is int8u
            MN::Primary5Intensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary6X::Id: { // type is int16u
            MN::Primary6X::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary6Y::Id: { // type is int16u
            MN::Primary6Y::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Primary6Intensity::Id: { // type is int8u
            MN::Primary6Intensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::WhitePointX::Id: { // type is int16u
            MN::WhitePointX::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::WhitePointY::Id: { // type is int16u
            MN::WhitePointY::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointRX::Id: { // type is int16u
            MN::ColorPointRX::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointRY::Id: { // type is int16u
            MN::ColorPointRY::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointRIntensity::Id: { // type is int8u
            MN::ColorPointRIntensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointGX::Id: { // type is int16u
            MN::ColorPointGX::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointGY::Id: { // type is int16u
            MN::ColorPointGY::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointGIntensity::Id: { // type is int8u
            MN::ColorPointGIntensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointBX::Id: { // type is int16u
            MN::ColorPointBX::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointBY::Id: { // type is int16u
            MN::ColorPointBY::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorPointBIntensity::Id: { // type is int8u
            MN::ColorPointBIntensity::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::EnhancedCurrentHue::Id: { // type is int16u
            MN::EnhancedCurrentHue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::EnhancedColorMode::Id: { // type is EnhancedColorModeEnum
            MN::EnhancedColorMode::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorLoopActive::Id: { // type is int8u
            MN::ColorLoopActive::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorLoopDirection::Id: { // type is int8u
            MN::ColorLoopDirection::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorLoopTime::Id: { // type is int16u
            MN::ColorLoopTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorLoopStartEnhancedHue::Id: { // type is int16u
            MN::ColorLoopStartEnhancedHue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorLoopStoredEnhancedHue::Id: { // type is int16u
            MN::ColorLoopStoredEnhancedHue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorCapabilities::Id: { // type is ColorCapabilitiesBitmap
            MN::ColorCapabilities::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorTempPhysicalMinMireds::Id: { // type is int16u
            MN::ColorTempPhysicalMinMireds::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ColorTempPhysicalMaxMireds::Id: { // type is int16u
            MN::ColorTempPhysicalMaxMireds::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::CoupleColorTempToLevelMinMireds::Id: { // type is int16u
            MN::CoupleColorTempToLevelMinMireds::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::StartUpColorTemperatureMireds::Id: { // type is int16u
            MN::StartUpColorTemperatureMireds::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ColorControlAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::ColorControl;

    if (aPath.mClusterId != Clusters::ColorControl::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        auto err_Result = m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
        if (err_Result != CHIP_ERROR_IN_PROGRESS) {
            return err_Result;
        }
    }

    switch (aPath.mAttributeId) {
    // CurrentHue is not supported by UCL
    // CurrentSaturation is not supported by UCL
    // RemainingTime is not supported by UCL
    // CurrentX is not supported by UCL
    // CurrentY is not supported by UCL
    // DriftCompensation is not supported by UCL
    // CompensationText is not supported by UCL
    // ColorTemperatureMireds is not supported by UCL
    // ColorMode is not supported by UCL
    case Attributes::Options::Id: {

        Attributes::Options::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["Options"] = to_json(value);
        break;
    }
    // NumberOfPrimaries is not supported by UCL
    // Primary1X is not supported by UCL
    // Primary1Y is not supported by UCL
    // Primary1Intensity is not supported by UCL
    // Primary2X is not supported by UCL
    // Primary2Y is not supported by UCL
    // Primary2Intensity is not supported by UCL
    // Primary3X is not supported by UCL
    // Primary3Y is not supported by UCL
    // Primary3Intensity is not supported by UCL
    // Primary4X is not supported by UCL
    // Primary4Y is not supported by UCL
    // Primary4Intensity is not supported by UCL
    // Primary5X is not supported by UCL
    // Primary5Y is not supported by UCL
    // Primary5Intensity is not supported by UCL
    // Primary6X is not supported by UCL
    // Primary6Y is not supported by UCL
    // Primary6Intensity is not supported by UCL
    case Attributes::WhitePointX::Id: {

        Attributes::WhitePointX::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["WhitePointX"] = to_json(value);
        break;
    }
    case Attributes::WhitePointY::Id: {

        Attributes::WhitePointY::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["WhitePointY"] = to_json(value);
        break;
    }
    case Attributes::ColorPointRX::Id: {

        Attributes::ColorPointRX::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointRX"] = to_json(value);
        break;
    }
    case Attributes::ColorPointRY::Id: {

        Attributes::ColorPointRY::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointRY"] = to_json(value);
        break;
    }
    case Attributes::ColorPointRIntensity::Id: {

        Attributes::ColorPointRIntensity::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointRIntensity"] = to_json(value);
        break;
    }
    case Attributes::ColorPointGX::Id: {

        Attributes::ColorPointGX::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointGX"] = to_json(value);
        break;
    }
    case Attributes::ColorPointGY::Id: {

        Attributes::ColorPointGY::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointGY"] = to_json(value);
        break;
    }
    case Attributes::ColorPointGIntensity::Id: {

        Attributes::ColorPointGIntensity::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointGIntensity"] = to_json(value);
        break;
    }
    case Attributes::ColorPointBX::Id: {

        Attributes::ColorPointBX::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointBX"] = to_json(value);
        break;
    }
    case Attributes::ColorPointBY::Id: {

        Attributes::ColorPointBY::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointBY"] = to_json(value);
        break;
    }
    case Attributes::ColorPointBIntensity::Id: {

        Attributes::ColorPointBIntensity::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["ColorPointBIntensity"] = to_json(value);
        break;
    }
    // EnhancedCurrentHue is not supported by UCL
    // EnhancedColorMode is not supported by UCL
    // ColorLoopActive is not supported by UCL
    // ColorLoopDirection is not supported by UCL
    // ColorLoopTime is not supported by UCL
    // ColorLoopStartEnhancedHue is not supported by UCL
    // ColorLoopStoredEnhancedHue is not supported by UCL
    // ColorCapabilities is not supported by UCL
    // ColorTempPhysicalMinMireds is not supported by UCL
    // ColorTempPhysicalMaxMireds is not supported by UCL
    // CoupleColorTempToLevelMinMireds is not supported by UCL
    case Attributes::StartUpColorTemperatureMireds::Id: {

        Attributes::StartUpColorTemperatureMireds::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["StartUpColorTemperatureMireds"] = to_json(value);
        break;
    }
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/ColorControl/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void ColorControlAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::ColorControl::Attributes;
    namespace UN = unify::matter_bridge::ColorControl::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::ColorControl::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::ColorControl::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is int8u
    case MN::CurrentHue::Id: {
        using T = MN::CurrentHue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CurrentHue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::CurrentHue::Id);
        }
        break;
    }
        // type is int8u
    case MN::CurrentSaturation::Id: {
        using T = MN::CurrentSaturation::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CurrentSaturation attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::CurrentSaturation::Id);
        }
        break;
    }
        // type is int16u
    case MN::RemainingTime::Id: {
        using T = MN::RemainingTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "RemainingTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::RemainingTime::Id);
        }
        break;
    }
        // type is int16u
    case MN::CurrentX::Id: {
        using T = MN::CurrentX::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CurrentX attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::CurrentX::Id);
        }
        break;
    }
        // type is int16u
    case MN::CurrentY::Id: {
        using T = MN::CurrentY::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CurrentY attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::CurrentY::Id);
        }
        break;
    }
        // type is DriftCompensationEnum
    case MN::DriftCompensation::Id: {
        using T = MN::DriftCompensation::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "DriftCompensation attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::DriftCompensation::Id);
        }
        break;
    }
        // type is char_string
    case MN::CompensationText::Id: {
        using T = MN::CompensationText::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CompensationText attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::CompensationText::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorTemperatureMireds::Id: {
        using T = MN::ColorTemperatureMireds::TypeInfo::Type;
        nlohmann::json modified_unify_value = unify_value;

        if (strcmp(modified_unify_value.dump().c_str(), "\"Undefined\"") == 0) {
            modified_unify_value = 0;
        }

        std::optional<T> value = from_json<T>(modified_unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorTemperatureMireds attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                MN::ColorTemperatureMireds::Id);
        }
        break;
    }
        // type is ColorModeEnum
    case MN::ColorMode::Id: {
        using T = chip::app::Clusters::ColorControl::ColorMode;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorMode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorMode::Id);
        }
        break;
    }
        // type is OptionsBitmap
    case MN::Options::Id: {
        using T = MN::Options::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Options attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Options::Id);
        }
        break;
    }
        // type is int8u
    case MN::NumberOfPrimaries::Id: {
        using T = MN::NumberOfPrimaries::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "NumberOfPrimaries attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::NumberOfPrimaries::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary1X::Id: {
        using T = MN::Primary1X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary1X attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary1X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary1Y::Id: {
        using T = MN::Primary1Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary1Y attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary1Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary1Intensity::Id: {
        using T = MN::Primary1Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary1Intensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary1Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary2X::Id: {
        using T = MN::Primary2X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary2X attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary2X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary2Y::Id: {
        using T = MN::Primary2Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary2Y attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary2Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary2Intensity::Id: {
        using T = MN::Primary2Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary2Intensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary2Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary3X::Id: {
        using T = MN::Primary3X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary3X attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary3X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary3Y::Id: {
        using T = MN::Primary3Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary3Y attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary3Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary3Intensity::Id: {
        using T = MN::Primary3Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary3Intensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary3Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary4X::Id: {
        using T = MN::Primary4X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary4X attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary4X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary4Y::Id: {
        using T = MN::Primary4Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary4Y attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary4Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary4Intensity::Id: {
        using T = MN::Primary4Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary4Intensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary4Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary5X::Id: {
        using T = MN::Primary5X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary5X attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary5X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary5Y::Id: {
        using T = MN::Primary5Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary5Y attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary5Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary5Intensity::Id: {
        using T = MN::Primary5Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary5Intensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary5Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary6X::Id: {
        using T = MN::Primary6X::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary6X attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary6X::Id);
        }
        break;
    }
        // type is int16u
    case MN::Primary6Y::Id: {
        using T = MN::Primary6Y::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary6Y attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary6Y::Id);
        }
        break;
    }
        // type is int8u
    case MN::Primary6Intensity::Id: {
        using T = MN::Primary6Intensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Primary6Intensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::Primary6Intensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::WhitePointX::Id: {
        using T = MN::WhitePointX::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "WhitePointX attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::WhitePointX::Id);
        }
        break;
    }
        // type is int16u
    case MN::WhitePointY::Id: {
        using T = MN::WhitePointY::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "WhitePointY attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::WhitePointY::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointRX::Id: {
        using T = MN::ColorPointRX::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointRX attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointRX::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointRY::Id: {
        using T = MN::ColorPointRY::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointRY attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointRY::Id);
        }
        break;
    }
        // type is int8u
    case MN::ColorPointRIntensity::Id: {
        using T = MN::ColorPointRIntensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointRIntensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointRIntensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointGX::Id: {
        using T = MN::ColorPointGX::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointGX attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointGX::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointGY::Id: {
        using T = MN::ColorPointGY::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointGY attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointGY::Id);
        }
        break;
    }
        // type is int8u
    case MN::ColorPointGIntensity::Id: {
        using T = MN::ColorPointGIntensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointGIntensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointGIntensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointBX::Id: {
        using T = MN::ColorPointBX::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointBX attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointBX::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorPointBY::Id: {
        using T = MN::ColorPointBY::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointBY attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointBY::Id);
        }
        break;
    }
        // type is int8u
    case MN::ColorPointBIntensity::Id: {
        using T = MN::ColorPointBIntensity::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorPointBIntensity attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorPointBIntensity::Id);
        }
        break;
    }
        // type is int16u
    case MN::EnhancedCurrentHue::Id: {
        using T = MN::EnhancedCurrentHue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "EnhancedCurrentHue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::EnhancedCurrentHue::Id);
        }
        break;
    }
        // type is EnhancedColorModeEnum
    case MN::EnhancedColorMode::Id: {
        using T = MN::EnhancedColorMode::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "EnhancedColorMode attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::EnhancedColorMode::Id);
        }
        break;
    }
        // type is int8u
    case MN::ColorLoopActive::Id: {
        using T = MN::ColorLoopActive::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorLoopActive attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorLoopActive::Id);
        }
        break;
    }
        // type is int8u
    case MN::ColorLoopDirection::Id: {
        using T = MN::ColorLoopDirection::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorLoopDirection attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorLoopDirection::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorLoopTime::Id: {
        using T = MN::ColorLoopTime::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorLoopTime attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorLoopTime::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorLoopStartEnhancedHue::Id: {
        using T = MN::ColorLoopStartEnhancedHue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorLoopStartEnhancedHue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                MN::ColorLoopStartEnhancedHue::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorLoopStoredEnhancedHue::Id: {
        using T = MN::ColorLoopStoredEnhancedHue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorLoopStoredEnhancedHue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                MN::ColorLoopStoredEnhancedHue::Id);
        }
        break;
    }
        // type is ColorCapabilitiesBitmap
    case MN::ColorCapabilities::Id: {
        using T = MN::ColorCapabilities::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorCapabilities attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id, MN::ColorCapabilities::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorTempPhysicalMinMireds::Id: {
        using T = MN::ColorTempPhysicalMinMireds::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorTempPhysicalMinMireds attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                MN::ColorTempPhysicalMinMireds::Id);
        }
        break;
    }
        // type is int16u
    case MN::ColorTempPhysicalMaxMireds::Id: {
        using T = MN::ColorTempPhysicalMaxMireds::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ColorTempPhysicalMaxMireds attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                MN::ColorTempPhysicalMaxMireds::Id);
        }
        break;
    }
        // type is int16u
    case MN::CoupleColorTempToLevelMinMireds::Id: {
        using T = MN::CoupleColorTempToLevelMinMireds::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "CoupleColorTempToLevelMinMireds attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                MN::CoupleColorTempToLevelMinMireds::Id);
        }
        break;
    }
        // type is int16u
    case MN::StartUpColorTemperatureMireds::Id: {
        using T = MN::StartUpColorTemperatureMireds::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "StartUpColorTemperatureMireds attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::ColorControl::Id,
                MN::StartUpColorTemperatureMireds::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
IlluminanceMeasurementAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::IlluminanceMeasurement::Attributes;
    namespace UN = unify::matter_bridge::IlluminanceMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::IlluminanceMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::MeasuredValue::Id: { // type is int16u
            MN::MeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinMeasuredValue::Id: { // type is int16u
            MN::MinMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxMeasuredValue::Id: { // type is int16u
            MN::MaxMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Tolerance::Id: { // type is int16u
            MN::Tolerance::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::LightSensorType::Id: { // type is LightSensorTypeEnum
            MN::LightSensorType::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR IlluminanceMeasurementAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::IlluminanceMeasurement;

    if (aPath.mClusterId != Clusters::IlluminanceMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        auto err_Result = m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
        if (err_Result != CHIP_ERROR_IN_PROGRESS) {
            return err_Result;
        }
    }

    switch (aPath.mAttributeId) {
        // MeasuredValue is not supported by UCL
        // MinMeasuredValue is not supported by UCL
        // MaxMeasuredValue is not supported by UCL
        // Tolerance is not supported by UCL
        // LightSensorType is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/IlluminanceMeasurement/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void IlluminanceMeasurementAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::IlluminanceMeasurement::Attributes;
    namespace UN = unify::matter_bridge::IlluminanceMeasurement::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::IlluminanceMeasurement::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is int16u
    case MN::MeasuredValue::Id: {
        using T = MN::MeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id,
                MN::MeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MinMeasuredValue::Id: {
        using T = MN::MinMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id,
                MN::MinMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MaxMeasuredValue::Id: {
        using T = MN::MaxMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id,
                MN::MaxMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::Tolerance::Id: {
        using T = MN::Tolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Tolerance attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id, MN::Tolerance::Id);
        }
        break;
    }
        // type is LightSensorTypeEnum
    case MN::LightSensorType::Id: {
        using T = MN::LightSensorType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "LightSensorType attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::IlluminanceMeasurement::Id,
                MN::LightSensorType::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
TemperatureMeasurementAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::TemperatureMeasurement::Attributes;
    namespace UN = unify::matter_bridge::TemperatureMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::TemperatureMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::MeasuredValue::Id: { // type is temperature
            MN::MeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinMeasuredValue::Id: { // type is temperature
            MN::MinMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxMeasuredValue::Id: { // type is temperature
            MN::MaxMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Tolerance::Id: { // type is int16u
            MN::Tolerance::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR TemperatureMeasurementAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::TemperatureMeasurement;

    if (aPath.mClusterId != Clusters::TemperatureMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        auto err_Result = m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
        if (err_Result != CHIP_ERROR_IN_PROGRESS) {
            return err_Result;
        }
    }

    switch (aPath.mAttributeId) {
        // MeasuredValue is not supported by UCL
        // MinMeasuredValue is not supported by UCL
        // MaxMeasuredValue is not supported by UCL
        // Tolerance is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/TemperatureMeasurement/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void TemperatureMeasurementAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::TemperatureMeasurement::Attributes;
    namespace UN = unify::matter_bridge::TemperatureMeasurement::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::TemperatureMeasurement::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::TemperatureMeasurement::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is temperature
    case MN::MeasuredValue::Id: {
        using T = MN::MeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TemperatureMeasurement::Id,
                MN::MeasuredValue::Id);
        }
        break;
    }
        // type is temperature
    case MN::MinMeasuredValue::Id: {
        using T = MN::MinMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TemperatureMeasurement::Id,
                MN::MinMeasuredValue::Id);
        }
        break;
    }
        // type is temperature
    case MN::MaxMeasuredValue::Id: {
        using T = MN::MaxMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TemperatureMeasurement::Id,
                MN::MaxMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::Tolerance::Id: {
        using T = MN::Tolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Tolerance attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::TemperatureMeasurement::Id, MN::Tolerance::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
PressureMeasurementAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::PressureMeasurement::Attributes;
    namespace UN = unify::matter_bridge::PressureMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::PressureMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::MeasuredValue::Id: { // type is int16s
            MN::MeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinMeasuredValue::Id: { // type is int16s
            MN::MinMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxMeasuredValue::Id: { // type is int16s
            MN::MaxMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Tolerance::Id: { // type is int16u
            MN::Tolerance::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ScaledValue::Id: { // type is int16s
            MN::ScaledValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinScaledValue::Id: { // type is int16s
            MN::MinScaledValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxScaledValue::Id: { // type is int16s
            MN::MaxScaledValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ScaledTolerance::Id: { // type is int16u
            MN::ScaledTolerance::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Scale::Id: { // type is int8s
            MN::Scale::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PressureMeasurementAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::PressureMeasurement;

    if (aPath.mClusterId != Clusters::PressureMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        auto err_Result = m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
        if (err_Result != CHIP_ERROR_IN_PROGRESS) {
            return err_Result;
        }
    }

    switch (aPath.mAttributeId) {
        // MeasuredValue is not supported by UCL
        // MinMeasuredValue is not supported by UCL
        // MaxMeasuredValue is not supported by UCL
        // Tolerance is not supported by UCL
        // ScaledValue is not supported by UCL
        // MinScaledValue is not supported by UCL
        // MaxScaledValue is not supported by UCL
        // ScaledTolerance is not supported by UCL
        // Scale is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/PressureMeasurement/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void PressureMeasurementAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::PressureMeasurement::Attributes;
    namespace UN = unify::matter_bridge::PressureMeasurement::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::PressureMeasurement::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::PressureMeasurement::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is int16s
    case MN::MeasuredValue::Id: {
        using T = MN::MeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::MeasuredValue::Id);
        }
        break;
    }
        // type is int16s
    case MN::MinMeasuredValue::Id: {
        using T = MN::MinMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id,
                MN::MinMeasuredValue::Id);
        }
        break;
    }
        // type is int16s
    case MN::MaxMeasuredValue::Id: {
        using T = MN::MaxMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id,
                MN::MaxMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::Tolerance::Id: {
        using T = MN::Tolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Tolerance attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::Tolerance::Id);
        }
        break;
    }
        // type is int16s
    case MN::ScaledValue::Id: {
        using T = MN::ScaledValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ScaledValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::ScaledValue::Id);
        }
        break;
    }
        // type is int16s
    case MN::MinScaledValue::Id: {
        using T = MN::MinScaledValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinScaledValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::MinScaledValue::Id);
        }
        break;
    }
        // type is int16s
    case MN::MaxScaledValue::Id: {
        using T = MN::MaxScaledValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxScaledValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::MaxScaledValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::ScaledTolerance::Id: {
        using T = MN::ScaledTolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "ScaledTolerance attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id,
                MN::ScaledTolerance::Id);
        }
        break;
    }
        // type is int8s
    case MN::Scale::Id: {
        using T = MN::Scale::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Scale attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::PressureMeasurement::Id, MN::Scale::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
FlowMeasurementAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::FlowMeasurement::Attributes;
    namespace UN = unify::matter_bridge::FlowMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::FlowMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::MeasuredValue::Id: { // type is int16u
            MN::MeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinMeasuredValue::Id: { // type is int16u
            MN::MinMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxMeasuredValue::Id: { // type is int16u
            MN::MaxMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Tolerance::Id: { // type is int16u
            MN::Tolerance::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR FlowMeasurementAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::FlowMeasurement;

    if (aPath.mClusterId != Clusters::FlowMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        auto err_Result = m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
        if (err_Result != CHIP_ERROR_IN_PROGRESS) {
            return err_Result;
        }
    }

    switch (aPath.mAttributeId) {
        // MeasuredValue is not supported by UCL
        // MinMeasuredValue is not supported by UCL
        // MaxMeasuredValue is not supported by UCL
        // Tolerance is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/FlowMeasurement/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void FlowMeasurementAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::FlowMeasurement::Attributes;
    namespace UN = unify::matter_bridge::FlowMeasurement::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::FlowMeasurement::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::FlowMeasurement::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is int16u
    case MN::MeasuredValue::Id: {
        using T = MN::MeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FlowMeasurement::Id, MN::MeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MinMeasuredValue::Id: {
        using T = MN::MinMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FlowMeasurement::Id, MN::MinMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MaxMeasuredValue::Id: {
        using T = MN::MaxMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FlowMeasurement::Id, MN::MaxMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::Tolerance::Id: {
        using T = MN::Tolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Tolerance attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::FlowMeasurement::Id, MN::Tolerance::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
RelativeHumidityMeasurementAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::RelativeHumidityMeasurement::Attributes;
    namespace UN = unify::matter_bridge::RelativeHumidityMeasurement::Attributes;
    if (aPath.mClusterId != Clusters::RelativeHumidityMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::MeasuredValue::Id: { // type is int16u
            MN::MeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MinMeasuredValue::Id: { // type is int16u
            MN::MinMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::MaxMeasuredValue::Id: { // type is int16u
            MN::MaxMeasuredValue::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::Tolerance::Id: { // type is int16u
            MN::Tolerance::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR RelativeHumidityMeasurementAttributeAccess::Write(const ConcreteDataAttributePath& aPath,
    AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::RelativeHumidityMeasurement;

    if (aPath.mClusterId != Clusters::RelativeHumidityMeasurement::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        auto err_Result = m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
        if (err_Result != CHIP_ERROR_IN_PROGRESS) {
            return err_Result;
        }
    }

    switch (aPath.mAttributeId) {
        // MeasuredValue is not supported by UCL
        // MinMeasuredValue is not supported by UCL
        // MaxMeasuredValue is not supported by UCL
        // Tolerance is not supported by UCL
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/RelativityHumidity/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void RelativeHumidityMeasurementAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::RelativeHumidityMeasurement::Attributes;
    namespace UN = unify::matter_bridge::RelativeHumidityMeasurement::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::RelativeHumidityMeasurement::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is int16u
    case MN::MeasuredValue::Id: {
        using T = MN::MeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id,
                MN::MeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MinMeasuredValue::Id: {
        using T = MN::MinMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MinMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id,
                MN::MinMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::MaxMeasuredValue::Id: {
        using T = MN::MaxMeasuredValue::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "MaxMeasuredValue attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id,
                MN::MaxMeasuredValue::Id);
        }
        break;
    }
        // type is int16u
    case MN::Tolerance::Id: {
        using T = MN::Tolerance::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Tolerance attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::RelativeHumidityMeasurement::Id,
                MN::Tolerance::Id);
        }
        break;
    }
    }
}

CHIP_ERROR
OccupancySensingAttributeAccess::Read(const ConcreteReadAttributePath& aPath, AttributeValueEncoder& aEncoder)
{
    namespace MN = chip::app::Clusters::OccupancySensing::Attributes;
    namespace UN = unify::matter_bridge::OccupancySensing::Attributes;
    if (aPath.mClusterId != Clusters::OccupancySensing::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    // Do not handle Read for non-unify endpoints
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }

    ConcreteAttributePath atr_path = ConcreteAttributePath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        return m_node_state_monitor.emulator().read_attribute(aPath, aEncoder);
    }

    try {
        switch (aPath.mAttributeId) {
        case MN::Occupancy::Id: { // type is OccupancyBitmap
            MN::Occupancy::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OccupancySensorType::Id: { // type is OccupancySensorTypeEnum
            MN::OccupancySensorType::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::OccupancySensorTypeBitmap::Id: { // type is OccupancySensorTypeBitmap
            MN::OccupancySensorTypeBitmap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::HoldTime::Id: { // type is int16u
            MN::HoldTime::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PIROccupiedToUnoccupiedDelay::Id: { // type is int16u
            MN::PIROccupiedToUnoccupiedDelay::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PIRUnoccupiedToOccupiedDelay::Id: { // type is int16u
            MN::PIRUnoccupiedToOccupiedDelay::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PIRUnoccupiedToOccupiedThreshold::Id: { // type is int8u
            MN::PIRUnoccupiedToOccupiedThreshold::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UltrasonicOccupiedToUnoccupiedDelay::Id: { // type is int16u
            MN::UltrasonicOccupiedToUnoccupiedDelay::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UltrasonicUnoccupiedToOccupiedDelay::Id: { // type is int16u
            MN::UltrasonicUnoccupiedToOccupiedDelay::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::UltrasonicUnoccupiedToOccupiedThreshold::Id: { // type is int8u
            MN::UltrasonicUnoccupiedToOccupiedThreshold::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PhysicalContactOccupiedToUnoccupiedDelay::Id: { // type is int16u
            MN::PhysicalContactOccupiedToUnoccupiedDelay::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PhysicalContactUnoccupiedToOccupiedDelay::Id: { // type is int16u
            MN::PhysicalContactUnoccupiedToOccupiedDelay::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::PhysicalContactUnoccupiedToOccupiedThreshold::Id: { // type is int8u
            MN::PhysicalContactUnoccupiedToOccupiedThreshold::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::FeatureMap::Id: { // type is bitmap32
            MN::FeatureMap::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        case MN::ClusterRevision::Id: { // type is int16u
            MN::ClusterRevision::TypeInfo::Type value;
            if (attribute_state_cache::get_instance().get(atr_path, value)) {
                return aEncoder.Encode(value);
            }
            break;
        }
        }
    } catch (const std::out_of_range& e) {
        sl_log_info(LOG_TAG,
            "The request attribute Path for endpoint [%i] is not found in the attribute state "
            "container: %s\n",
            atr_path.mEndpointId, e.what());
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OccupancySensingAttributeAccess::Write(const ConcreteDataAttributePath& aPath, AttributeValueDecoder& aDecoder)
{
    using namespace chip::app::Clusters::OccupancySensing;

    if (aPath.mClusterId != Clusters::OccupancySensing::Id) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto unify_node = m_node_state_monitor.bridged_endpoint(aPath.mEndpointId);

    if (!unify_node) {
        return CHIP_NO_ERROR;
    }
    nlohmann::json jsn;

    if (m_node_state_monitor.emulator().is_attribute_emulated(aPath)) {
        auto err_Result = m_node_state_monitor.emulator().write_attribute(aPath, aDecoder);
        if (err_Result != CHIP_ERROR_IN_PROGRESS) {
            return err_Result;
        }
    }

    switch (aPath.mAttributeId) {
    // Occupancy is not supported by UCL
    // OccupancySensorType is not supported by UCL
    // OccupancySensorTypeBitmap is not supported by UCL
    // HoldTimeLimits is not supported by UCL
    case Attributes::PIROccupiedToUnoccupiedDelay::Id: {

        Attributes::PIROccupiedToUnoccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["PIROccupiedToUnoccupiedDelay"] = to_json(value);
        break;
    }
    case Attributes::PIRUnoccupiedToOccupiedDelay::Id: {

        Attributes::PIRUnoccupiedToOccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["PIRUnoccupiedToOccupiedDelay"] = to_json(value);
        break;
    }
    case Attributes::PIRUnoccupiedToOccupiedThreshold::Id: {

        Attributes::PIRUnoccupiedToOccupiedThreshold::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["PIRUnoccupiedToOccupiedThreshold"] = to_json(value);
        break;
    }
    case Attributes::UltrasonicOccupiedToUnoccupiedDelay::Id: {

        Attributes::UltrasonicOccupiedToUnoccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["UltrasonicOccupiedToUnoccupiedDelay"] = to_json(value);
        break;
    }
    case Attributes::UltrasonicUnoccupiedToOccupiedDelay::Id: {

        Attributes::UltrasonicUnoccupiedToOccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["UltrasonicUnoccupiedToOccupiedDelay"] = to_json(value);
        break;
    }
    case Attributes::UltrasonicUnoccupiedToOccupiedThreshold::Id: {

        Attributes::UltrasonicUnoccupiedToOccupiedThreshold::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["UltrasonicUnoccupiedToOccupiedThreshold"] = to_json(value);
        break;
    }
    case Attributes::PhysicalContactOccupiedToUnoccupiedDelay::Id: {

        Attributes::PhysicalContactOccupiedToUnoccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["PhysicalContactOccupiedToUnoccupiedDelay"] = to_json(value);
        break;
    }
    case Attributes::PhysicalContactUnoccupiedToOccupiedDelay::Id: {

        Attributes::PhysicalContactUnoccupiedToOccupiedDelay::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["PhysicalContactUnoccupiedToOccupiedDelay"] = to_json(value);
        break;
    }
    case Attributes::PhysicalContactUnoccupiedToOccupiedThreshold::Id: {

        Attributes::PhysicalContactUnoccupiedToOccupiedThreshold::TypeInfo::DecodableType value;
        aDecoder.Decode(value);
        jsn["PhysicalContactUnoccupiedToOccupiedThreshold"] = to_json(value);
        break;
    }
        // GeneratedCommandList is not supported by UCL
        // AcceptedCommandList is not supported by UCL
        // EventList is not supported by UCL
        // AttributeList is not supported by UCL
        // FeatureMap is not supported by UCL
        // ClusterRevision is not supported by UCL
    }

    if (!jsn.empty()) {
        std::string topic = "ucl/by-unid/" + unify_node->unify_unid + "/ep" + std::to_string(unify_node->unify_endpoint) + "/OccupancySensing/Commands/WriteAttributes";
        std::string payload_str = jsn.dump();
        m_unify_mqtt.Publish(topic, payload_str, true);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MESSAGE_HANDLER;
}

void OccupancySensingAttributeAccess::reported_updated(const bridged_endpoint* ep, const std::string& cluster,
    const std::string& attribute, const nlohmann::json& unify_value)
{
    namespace MN = chip::app::Clusters::OccupancySensing::Attributes;
    namespace UN = unify::matter_bridge::OccupancySensing::Attributes;

    auto cluster_id = m_dev_translator.get_cluster_id(cluster);

    if (!cluster_id.has_value() || (cluster_id.value() != Clusters::OccupancySensing::Id)) {
        return;
    }

    // get attribute id
    auto attribute_id = m_dev_translator.get_attribute_id(cluster, attribute);

    if (!attribute_id.has_value()) {
        return;
    }

    chip::EndpointId node_matter_endpoint = ep->matter_endpoint;
    ConcreteAttributePath attrpath = ConcreteAttributePath(node_matter_endpoint, Clusters::OccupancySensing::Id, attribute_id.value());
    switch (attribute_id.value()) {
    // type is OccupancyBitmap
    case MN::Occupancy::Id: {
        using T = MN::Occupancy::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "Occupancy attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id, MN::Occupancy::Id);
        }
        break;
    }
        // type is OccupancySensorTypeEnum
    case MN::OccupancySensorType::Id: {
        using T = MN::OccupancySensorType::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OccupancySensorType attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::OccupancySensorType::Id);
        }
        break;
    }
        // type is OccupancySensorTypeBitmap
    case MN::OccupancySensorTypeBitmap::Id: {
        using T = MN::OccupancySensorTypeBitmap::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "OccupancySensorTypeBitmap attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::OccupancySensorTypeBitmap::Id);
        }
        break;
    }
        // type is int16u
    case MN::PIROccupiedToUnoccupiedDelay::Id: {
        using T = MN::PIROccupiedToUnoccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PIROccupiedToUnoccupiedDelay attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::PIROccupiedToUnoccupiedDelay::Id);
        }
        break;
    }
        // type is int16u
    case MN::PIRUnoccupiedToOccupiedDelay::Id: {
        using T = MN::PIRUnoccupiedToOccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PIRUnoccupiedToOccupiedDelay attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::PIRUnoccupiedToOccupiedDelay::Id);
        }
        break;
    }
        // type is int8u
    case MN::PIRUnoccupiedToOccupiedThreshold::Id: {
        using T = MN::PIRUnoccupiedToOccupiedThreshold::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PIRUnoccupiedToOccupiedThreshold attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::PIRUnoccupiedToOccupiedThreshold::Id);
        }
        break;
    }
        // type is int16u
    case MN::UltrasonicOccupiedToUnoccupiedDelay::Id: {
        using T = MN::UltrasonicOccupiedToUnoccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UltrasonicOccupiedToUnoccupiedDelay attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::UltrasonicOccupiedToUnoccupiedDelay::Id);
        }
        break;
    }
        // type is int16u
    case MN::UltrasonicUnoccupiedToOccupiedDelay::Id: {
        using T = MN::UltrasonicUnoccupiedToOccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UltrasonicUnoccupiedToOccupiedDelay attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::UltrasonicUnoccupiedToOccupiedDelay::Id);
        }
        break;
    }
        // type is int8u
    case MN::UltrasonicUnoccupiedToOccupiedThreshold::Id: {
        using T = MN::UltrasonicUnoccupiedToOccupiedThreshold::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "UltrasonicUnoccupiedToOccupiedThreshold attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::UltrasonicUnoccupiedToOccupiedThreshold::Id);
        }
        break;
    }
        // type is int16u
    case MN::PhysicalContactOccupiedToUnoccupiedDelay::Id: {
        using T = MN::PhysicalContactOccupiedToUnoccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PhysicalContactOccupiedToUnoccupiedDelay attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::PhysicalContactOccupiedToUnoccupiedDelay::Id);
        }
        break;
    }
        // type is int16u
    case MN::PhysicalContactUnoccupiedToOccupiedDelay::Id: {
        using T = MN::PhysicalContactUnoccupiedToOccupiedDelay::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PhysicalContactUnoccupiedToOccupiedDelay attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::PhysicalContactUnoccupiedToOccupiedDelay::Id);
        }
        break;
    }
        // type is int8u
    case MN::PhysicalContactUnoccupiedToOccupiedThreshold::Id: {
        using T = MN::PhysicalContactUnoccupiedToOccupiedThreshold::TypeInfo::Type;
        std::optional<T> value = from_json<T>(unify_value);

        if (value.has_value()) {
            sl_log_debug(LOG_TAG, "PhysicalContactUnoccupiedToOccupiedThreshold attribute value is %s", unify_value.dump().c_str());
            attribute_state_cache::get_instance().set<T>(attrpath, value.value());
            MatterReportingAttributeChangeCallback(node_matter_endpoint, Clusters::OccupancySensing::Id,
                MN::PhysicalContactUnoccupiedToOccupiedThreshold::Id);
        }
        break;
    }
    }
}
