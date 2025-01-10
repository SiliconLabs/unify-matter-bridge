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

#ifndef EMULATE_WINDOWCOVERING_HPP
#define EMULATE_WINDOWCOVERING_HPP

// Emulator interface
#include "emulator.hpp"
#include "cluster_emulator.hpp"
#include "sl_log.h"
#include "attribute_callback_registry.hpp"

constexpr uint8_t WNCV_NOT_MOVING_OPERATIONAL_STATUS = 0x00;
// Window Covering(WNCV) Operational Status values if lift feature supported
constexpr uint8_t WNCV_LIFT_OPERATIONAL_STATUS_OPENING = 0x05;
constexpr uint8_t WNCV_LIFT_OPERATIONAL_STATUS_CLOSING = 0x0A;
constexpr uint8_t WNCV_MIN_LIFT_PERCENTAGE = 0x00;
constexpr uint8_t WNCV_MAX_LIFT_PERCENTAGE = 0x64;

namespace unify::matter_bridge {

using namespace chip::app::Clusters;

class EmulateWindowCovering : public EmulatorInterface
{

private:

void register_attribute_callbacks () const
    {

        AttributeCallbackRegistry::getInstance().register_callback("WindowCovering::CurrentPositionLiftPercentage",
            [](const nlohmann::json &reportedValue, const chip::EndpointId &matter_endpoint) {
                attribute_state_cache &cache = attribute_state_cache::get_instance();
                chip::app::ConcreteAttributePath aPath = { matter_endpoint, WindowCovering::Id, WindowCovering::Attributes::OperationalStatus::Id };

                int liftPercentage = reportedValue.get<int>();
                if (liftPercentage == WNCV_MIN_LIFT_PERCENTAGE || liftPercentage == WNCV_MAX_LIFT_PERCENTAGE) {
                    WindowCovering::Attributes::OperationalStatus::TypeInfo::Type OperationalStatus = WNCV_NOT_MOVING_OPERATIONAL_STATUS;
                    cache.set<WindowCovering::Attributes::OperationalStatus::TypeInfo::Type>(aPath, OperationalStatus);
                }
            });

        AttributeCallbackRegistry::getInstance().register_callback("WindowCovering::CurrentPositionLift",
            [](const nlohmann::json &reportedValue, const chip::EndpointId &matter_endpoint) {
                attribute_state_cache &cache = attribute_state_cache::get_instance();
                chip::app::ConcreteAttributePath aPath = { matter_endpoint, WindowCovering::Id, WindowCovering::Attributes::OperationalStatus::Id };

                auto installed_open_limit_path = ConcreteAttributePath(matter_endpoint, WindowCovering::Id,
                                WindowCovering::Attributes::InstalledOpenLimitLift::Id);

                WindowCovering::Attributes::InstalledOpenLimitLift::TypeInfo::Type InstalledOpenLimitLift;
                bool hasInstalledOpenLimitLift = cache.get<WindowCovering::Attributes::InstalledOpenLimitLift::TypeInfo::Type>(installed_open_limit_path, InstalledOpenLimitLift);

                auto installed_closed_limit_path = ConcreteAttributePath(matter_endpoint, WindowCovering::Id,
                                WindowCovering::Attributes::InstalledClosedLimitLift::Id);

                WindowCovering::Attributes::InstalledClosedLimitLift::TypeInfo::Type InstalledClosedLimitLift;
                bool hasInstalledClosedLimitLift = cache.get<WindowCovering::Attributes::InstalledClosedLimitLift::TypeInfo::Type>(installed_closed_limit_path, InstalledClosedLimitLift);

                uint16_t lift = reportedValue.get<uint16_t>();
                if (hasInstalledClosedLimitLift && hasInstalledOpenLimitLift &&
                    (lift == InstalledOpenLimitLift || lift == InstalledClosedLimitLift)) {
                        WindowCovering::Attributes::OperationalStatus::TypeInfo::Type OperationalStatus = WNCV_NOT_MOVING_OPERATIONAL_STATUS;
                        cache.set<WindowCovering::Attributes::OperationalStatus::TypeInfo::Type>(aPath, OperationalStatus);
                }
            });
    }

public:

    EmulateWindowCovering()
    {
        register_attribute_callbacks();
    }

    const char * emulated_cluster_name() const override { return "WindowCovering"; }

    chip::ClusterId emulated_cluster() const override { return WindowCovering::Id; };

    std::vector<chip::AttributeId> emulated_attributes() const override
    {
        return { WindowCovering::Attributes::OperationalStatus::Id, WindowCovering::Attributes::EndProductType::Id };
    }

    std::vector<chip::CommandId> emulated_commands() const override
    {
        return { WindowCovering::Commands::UpOrOpen::Id, WindowCovering::Commands::DownOrClose::Id, WindowCovering::Commands::StopMotion::Id };
    }

    /**     
     * @brief Emulate WindowCovering cluster to emulate commands and attributes
     *
     * @param unify_cluster
     * @param cluster_builder
     * @return CHIP_ERROR
     */
    CHIP_ERROR
    emulate(const node_state_monitor::cluster & unify_cluster, matter_cluster_builder & cluster_builder) override
    {
        // Add OperationalStatus and EndProductType attribute to the matter cluster
        cluster_builder.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                         WindowCovering::Attributes::OperationalStatus::Id,
                                                                         1,
                                                                         ZAP_TYPE(BITMAP8),
                                                                         ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)});
        cluster_builder.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                         WindowCovering::Attributes::EndProductType::Id,
                                                                         2,
                                                                         ZAP_TYPE(ENUM8),
                                                                         ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)});

        return CHIP_NO_ERROR;
    }

    /**
     * @brief Read an emulated attribute for WindowCovering cluster 
     *        
     * This read_attribute overides the default read for emulated attributes.
     *
     * @param aPath
     * @param aEncoder
     * @return CHIP_ERROR
     */
    CHIP_ERROR read_attribute(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override
    {
        attribute_state_cache & cache = attribute_state_cache::get_instance();

        switch (aPath.mAttributeId)
        {
        case WindowCovering::Attributes::EndProductType::Id: {
            WindowCovering::Attributes::EndProductType::TypeInfo::Type EndProductType;

            if (!cache.get(aPath, EndProductType))
            {
                EndProductType = WindowCovering::EndProductType::kRollerShade; 
            }
            return aEncoder.Encode(EndProductType);
        }
        case WindowCovering::Attributes::OperationalStatus::Id: {
            WindowCovering::Attributes::OperationalStatus::TypeInfo::Type OperationalStatus;
       
            if (!cache.get(aPath, OperationalStatus))
            {
                OperationalStatus = static_cast<WindowCovering::Attributes::OperationalStatus::TypeInfo::Type>(0X00);
            }
            return aEncoder.Encode(OperationalStatus);
        }
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    };

    /**
     * @brief Handle an emulated command for WindowCovering cluster
     *
     * @param handlerContext
     * @param cdata Holder for the emulated command data and payload
     * @return CHIP_ERROR
     */
    CHIP_ERROR command(CommandHandlerInterface::HandlerContext & handlerContext, emulated_cmd_payload & cdata) override
    {
        // WindowCovering cluster emulated command need further handling in command_translotor.
        cdata.cmd_emulation_completed = false;
        auto err = CHIP_ERROR_NOT_IMPLEMENTED;
        attribute_state_cache & cache = attribute_state_cache::get_instance();

        auto operational_status_path = ConcreteAttributePath(handlerContext.mRequestPath.mEndpointId, handlerContext.mRequestPath.mClusterId,
                                WindowCovering::Attributes::OperationalStatus::Id);

        auto current_pos_lift_per_path = ConcreteAttributePath(handlerContext.mRequestPath.mEndpointId, handlerContext.mRequestPath.mClusterId,
                                WindowCovering::Attributes::CurrentPositionLiftPercentage::Id);

        auto current_pos_lift_path = ConcreteAttributePath(handlerContext.mRequestPath.mEndpointId, handlerContext.mRequestPath.mClusterId,
                                WindowCovering::Attributes::CurrentPositionLift::Id);

        WindowCovering::Attributes::CurrentPositionLiftPercentage::TypeInfo::Type CurrentPositionLiftPercentage;
        WindowCovering::Attributes::CurrentPositionLift::TypeInfo::Type CurrentPositionLift;

        cache.get<WindowCovering::Attributes::CurrentPositionLiftPercentage::TypeInfo::Type>(current_pos_lift_per_path, CurrentPositionLiftPercentage);
        cache.get<WindowCovering::Attributes::CurrentPositionLift::TypeInfo::Type>(current_pos_lift_path, CurrentPositionLift);

        uint16_t CurrentPositionLiftVal = CurrentPositionLift.ValueOr(0);
        chip::Percent CurrentPositionLiftPerVal = CurrentPositionLiftPercentage.ValueOr(0);

        switch (handlerContext.mRequestPath.mCommandId)
        {
        case WindowCovering::Commands::UpOrOpen::Id: {
            cdata.cmd = "UpOrOpen"; // "UpOrOpen"

            auto installed_open_limit_path = ConcreteAttributePath(handlerContext.mRequestPath.mEndpointId, handlerContext.mRequestPath.mClusterId,
                                WindowCovering::Attributes::InstalledOpenLimitLift::Id);
            WindowCovering::Attributes::InstalledOpenLimitLift::TypeInfo::Type InstalledOpenLimitLift;
            bool hasInstalledOpenLimitLift = cache.get<WindowCovering::Attributes::InstalledOpenLimitLift::TypeInfo::Type>(installed_open_limit_path, InstalledOpenLimitLift);

            WindowCovering::Attributes::OperationalStatus::TypeInfo::Type OperationalStatus = WNCV_NOT_MOVING_OPERATIONAL_STATUS;
            if(!CurrentPositionLiftPercentage.IsNull() && static_cast<int>(CurrentPositionLiftPerVal) != WNCV_MIN_LIFT_PERCENTAGE)
            {
                OperationalStatus = chip::BitMask<chip::app::Clusters::WindowCovering::OperationalStatus>(WNCV_LIFT_OPERATIONAL_STATUS_OPENING);
            }
            else if( hasInstalledOpenLimitLift && (!CurrentPositionLift.IsNull() && CurrentPositionLiftVal != InstalledOpenLimitLift) ){
                OperationalStatus = chip::BitMask<chip::app::Clusters::WindowCovering::OperationalStatus>(WNCV_LIFT_OPERATIONAL_STATUS_OPENING);
            }
            cache.set<WindowCovering::Attributes::OperationalStatus::TypeInfo::Type>(operational_status_path, OperationalStatus);

            err = CHIP_NO_ERROR;
        }
        break;
        case WindowCovering::Commands::DownOrClose::Id: {
            cdata.cmd = "DownOrClose"; // "DownOrClose"

            auto installed_closed_limit_path = ConcreteAttributePath(handlerContext.mRequestPath.mEndpointId, handlerContext.mRequestPath.mClusterId,
                                WindowCovering::Attributes::InstalledClosedLimitLift::Id);

            WindowCovering::Attributes::InstalledClosedLimitLift::TypeInfo::Type InstalledClosedLimitLift;
            bool hasInstalledClosedLimitLift = cache.get<WindowCovering::Attributes::InstalledClosedLimitLift::TypeInfo::Type>(installed_closed_limit_path, InstalledClosedLimitLift);

            WindowCovering::Attributes::OperationalStatus::TypeInfo::Type OperationalStatus = WNCV_NOT_MOVING_OPERATIONAL_STATUS;

            if(!CurrentPositionLiftPercentage.IsNull() && static_cast<int>(CurrentPositionLiftPerVal) != WNCV_MAX_LIFT_PERCENTAGE)
            {
                OperationalStatus = chip::BitMask<chip::app::Clusters::WindowCovering::OperationalStatus>(WNCV_LIFT_OPERATIONAL_STATUS_CLOSING);
            }
            else if(hasInstalledClosedLimitLift && (!CurrentPositionLift.IsNull() && CurrentPositionLiftVal != InstalledClosedLimitLift)){
                OperationalStatus = chip::BitMask<chip::app::Clusters::WindowCovering::OperationalStatus>(WNCV_LIFT_OPERATIONAL_STATUS_CLOSING);
            }
            cache.set<WindowCovering::Attributes::OperationalStatus::TypeInfo::Type>(operational_status_path, OperationalStatus);

            err = CHIP_NO_ERROR;
        }
        break;
        case WindowCovering::Commands::StopMotion::Id: {
            cdata.cmd = "StopMotion"; // "StopMotion"
            WindowCovering::Attributes::OperationalStatus::TypeInfo::Type OperationalStatus = WNCV_NOT_MOVING_OPERATIONAL_STATUS;
            cache.set<WindowCovering::Attributes::OperationalStatus::TypeInfo::Type>(operational_status_path, OperationalStatus);
            err = CHIP_NO_ERROR;
        }
        default:
            break;
        }
        return err;
    };
};

} // namespace unify::matter_bridge

#endif // EMULATE_WindowCovering_HPP